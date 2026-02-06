
#include "clst.h"

#include "win_dialogue.h"
#include "win_filesystem.h"
#include "win_text.h"

#include "deps/nlohmann/json.hpp"

/* 内部用 */
namespace clst
{
	struct SPlayerSetting
	{
		std::wstring wstrAtlasExtension = L".atlas";
		std::wstring wstrSkelExtension = L".skel";
		std::wstring wstrVoiceExtension = L".m4a";
		std::wstring wstrSceneTextExtension = L".json";

		std::string strFontFilePath = "C:\\Windows\\Fonts\\yumin.ttf";
	};

	static SPlayerSetting g_playerSetting;

	static bool ReadSettingFile(SPlayerSetting& playerSetting)
	{
		std::wstring wstrFilePath = win_filesystem::GetCurrentProcessPath() + L"\\setting.txt";
		std::string strFile = win_filesystem::LoadFileAsString(wstrFilePath.c_str());
		if (strFile.empty())return false;

		try
		{
			const nlohmann::json& nlJson = nlohmann::json::parse(strFile);
			const nlohmann::json& jExtension = nlJson.at("extensions");

			const std::string* pStr;

			pStr = jExtension.at("atlas").get<const std::string*>();
			if (pStr != nullptr)playerSetting.wstrAtlasExtension = win_text::WidenUtf8(*pStr);

			pStr = jExtension.at("skel").get<const std::string*>();
			if (pStr != nullptr)playerSetting.wstrSkelExtension = win_text::WidenUtf8(*pStr);

			pStr = jExtension.at("voice").get<const std::string*>();
			if (pStr != nullptr)playerSetting.wstrVoiceExtension = win_text::WidenUtf8(*pStr);

			pStr = jExtension.at("sceneText").get<const std::string*>();
			if (pStr != nullptr)playerSetting.wstrSceneTextExtension = win_text::WidenUtf8(*pStr);

			playerSetting.strFontFilePath = nlJson.at("fontPath");
		}
		catch (const nlohmann::json::exception& e)
		{
			win_dialogue::ShowMessageBox(e.what(), "Setting error");
			return false;
		}

		return true;
	}


	struct CommandIndex abstract final
	{
		enum
		{
			Command,
			Arg1,
			Arg2,
			Arg3,
			Arg4,
			Arg5,
			Arg6,
			Skin,
			SpineAnim,
			SpineSubAnim,
			AnimCompType,
			AnimSpd,
			WaitType,
			Text,
			PageCtrl,
			Voice,
			WindowType
		};
	};

	struct StoryDatum
	{
		std::wstring wstrText;
		std::wstring wstrVoiceFileName;

		/* There should be more structs to manage decently, but it requires much more code refactoring. */
		size_t nAnimationIndex = 0;
	};

	/*
	* Path regarding "XXXXXXX" as a character ID.
	* 脚本 ../Adventure/ImportChara/CharaScenarioXXXXXXX.book.json
	* 音声 ../AssetBundles/Sound/Voice/ImportChara/XXXXXXX/
	* 画像 ../AssetBundles/Stills/st_XXXXXXX/
	*/

	/* 人物ID抽出 */
	static std::wstring ExtractCharacterIdFromSpineFolderPath(const std::wstring& wstrSpineFolderPath)
	{
		size_t nPos = wstrSpineFolderPath.find_last_of(L"\\/");
		if (nPos == std::wstring::npos)return {};
		++nPos;

		nPos = wstrSpineFolderPath.find(L"st_", nPos);
		if (nPos == std::wstring::npos)return {};

		return wstrSpineFolderPath.substr(nPos + 3);
	}

	/* ID対応先探索 */
	static std::wstring FindPathContainingId(const std::wstring& targetFolder, const std::wstring& wstrId, const wchar_t* pwzFileExtension)
	{
		std::vector<std::wstring> paths;
		win_filesystem::CreateFilePathList(targetFolder.c_str(), pwzFileExtension, paths);
		const auto IsContained = [&wstrId](const std::wstring& wstr)
			-> bool
			{
				return wstr.find(wstrId) != std::wstring::npos;
			};

		const auto& iter = std::find_if(paths.begin(), paths.end(), IsContained);
		if (iter == paths.cend())return {};

		size_t nIndex = std::distance(paths.begin(), iter);
		return paths[nIndex];
	}

	/* 画像階層 => 音声階層 */
	static std::wstring DeriveVoiceFolderPathFromSpineFolderPath(const std::wstring& wstrAtlasFolderPath)
	{
		std::wstring wstrCharacterId = ExtractCharacterIdFromSpineFolderPath(wstrAtlasFolderPath);
		if (wstrCharacterId.empty())return {};

		size_t nPos = wstrAtlasFolderPath.find(L"Stills");
		if (nPos == std::wstring::npos)return {};

		std::wstring wstrVoiceFolder = wstrAtlasFolderPath.substr(0, nPos);
		wstrVoiceFolder += L"Sound\\Voice\\ImportChara";

		return FindPathContainingId(wstrVoiceFolder, wstrCharacterId, nullptr);
	}
	/* 画像階層 => 脚本経路 */
	static std::wstring DeriveScenarioFilePathFromSpineFolderPath(const std::wstring& wstrAtlasFolderPath)
	{
		std::wstring wstrCharacterId = ExtractCharacterIdFromSpineFolderPath(wstrAtlasFolderPath);
		if (wstrCharacterId.empty())return {};

		size_t nPos = wstrAtlasFolderPath.find(L"AssetBundles");
		if (nPos == std::wstring::npos)return {};

		std::wstring wstrVoiceFolder = wstrAtlasFolderPath.substr(0, nPos);
		wstrVoiceFolder += L"Adventure\\ImportChara";

		std::wstring wstrScenerioId = L"CharaScenario" + wstrCharacterId + L".book";

		return FindPathContainingId(wstrVoiceFolder, wstrScenerioId, g_playerSetting.wstrSceneTextExtension.c_str());
	}

	/* 脚本ファイル読み取り */
	static bool ReadScenarioFile(const std::wstring& wstrFilePath, std::vector<StoryDatum>& storyData, std::vector<std::string>& animationNames)
	{
		std::string strFile = win_filesystem::LoadFileAsString(wstrFilePath.c_str());
		if (strFile.empty())return false;

		size_t nCurrentAnimationIndex = 0;
		try
		{
			const nlohmann::json& nlJson = nlohmann::json::parse(strFile);
			/* [0 - 3] : 日常, [4]: 本番 */
			const nlohmann::json& jData = nlJson.at("importGridList").back().at("rows");

			/* 最初の要素は項目名なので飛ばす */
			for (size_t i = 1; i < jData.size(); ++i)
			{
				const nlohmann::json& jRow = jData[i].at("strings");
				if (jRow.empty())continue;

				std::string_view command = jRow[CommandIndex::Command];
				if (command.empty())
				{
					if (jRow.size() > CommandIndex::Text)
					{
						StoryDatum s;

						std::string_view text = jRow[CommandIndex::Text];
						if (text.empty())continue;

						std::string_view characterName = jRow[CommandIndex::Arg1];
						if (!characterName.empty())
						{
							s.wstrText = win_text::WidenUtf8(characterName.data(), static_cast<int>(characterName.size()));
							s.wstrText += L":\n";
						}
						s.wstrText += win_text::WidenUtf8(text.data(), static_cast<int>(text.size()));

						if (jRow.size() > CommandIndex::Voice)
						{
							s.wstrVoiceFileName = win_text::WidenUtf8(jRow[CommandIndex::Voice]);
						}

						s.nAnimationIndex = nCurrentAnimationIndex;

						storyData.push_back(std::move(s));
					}
				}
				else
				{
					/* 動作指定 */
					if (command == "BgEvent")
					{
						if (jRow.size() > CommandIndex::SpineAnim)
						{
							animationNames.push_back(jRow[CommandIndex::SpineAnim]);

							nCurrentAnimationIndex = animationNames.size() - 1;
						}
					}
				}
			}
		}
		catch (const nlohmann::json::exception& e)
		{
			win_dialogue::ShowMessageBox(e.what(), "Parse error");
			return false;
		}

		return true;
	}

	static void ReplaceEmoji(std::wstring& src)
	{
		constexpr wchar_t swzToBeReplaced[] = L"<emoji=heart04>";
		for (size_t nRead = 0;;)
		{
			size_t nPos = src.find(swzToBeReplaced, nRead);
			if (nPos == std::wstring::npos) break;
			src.replace(nPos, sizeof(swzToBeReplaced) / sizeof(wchar_t) - 1, L"♡");
			nRead = nPos + 1;
		}
	}

	static void EliminateTag(std::wstring& wstr)
	{
		std::wstring wstrResult;
		wstrResult.reserve(wstr.size());
		int iCount = 0;
		for (const auto& c : wstr)
		{
			if (c == L'<')
			{
				++iCount;
				continue;
			}
			else if (c == L'>')
			{
				--iCount;
				continue;
			}

			if (iCount == 0)
			{
				wstrResult.push_back(c);
			}
		}
		wstr = std::move(wstrResult);
	}

} /* namespace clst */


bool clst::InitialiseSetting()
{
	SPlayerSetting playerSetting;
	bool bRet = ReadSettingFile(playerSetting);
	if (bRet)
	{
		g_playerSetting = std::move(playerSetting);
	}

	return g_playerSetting.wstrAtlasExtension != g_playerSetting.wstrSkelExtension;
}

const std::string& clst::GetFontFilePath()
{
	return g_playerSetting.strFontFilePath;
}

bool clst::IsSkelBinary()
{
	constexpr const wchar_t* const binaryCandidates[] = { L".skel", L".bin" };

	for (const auto& binaryCandidate : binaryCandidates)
	{
		if (g_playerSetting.wstrSkelExtension.find(binaryCandidate) != std::wstring::npos)
		{
			return true;
		}
	}

	return false;
}

/* 描画素材一覧取得 */
void clst::GetSpineList(const std::wstring& wstrFolderPath, std::vector<std::string>& atlasPaths, std::vector<std::string>& skelPaths)
{
	bool isAtlasLonger = g_playerSetting.wstrAtlasExtension.size() > g_playerSetting.wstrSkelExtension.size();

	std::wstring& longerExtesion = isAtlasLonger ? g_playerSetting.wstrAtlasExtension : g_playerSetting.wstrSkelExtension;
	std::wstring& shorterExtension = isAtlasLonger ? g_playerSetting.wstrSkelExtension : g_playerSetting.wstrAtlasExtension;
	std::vector<std::string>& longerPaths = isAtlasLonger ? atlasPaths : skelPaths;
	std::vector<std::string>& shorterPaths = isAtlasLonger ? skelPaths : atlasPaths;

	std::vector<std::wstring> filePaths;
	win_filesystem::CreateFilePathList(wstrFolderPath.c_str(), L"*", filePaths);

	for (const auto& filePath : filePaths)
	{
		const auto EndsWith = [&filePath](const std::wstring& str)
			-> bool
			{
				if (filePath.size() < str.size()) return false;
				return std::equal(str.rbegin(), str.rend(), filePath.rbegin());
			};

		if (EndsWith(longerExtesion))
		{
			longerPaths.emplace_back(win_text::NarrowUtf8(filePath));
		}
		else if (EndsWith(shorterExtension))
		{
			shorterPaths.emplace_back(win_text::NarrowUtf8(filePath));
		}
	}
}
/* 脚本ファイルの探索と読み込み */
bool clst::SearchAndLoadScenarioFile(const std::wstring& wstrAtlasFolderPath, std::vector<adv::TextDatum>& textData, std::vector<std::string>& animationNames)
{
	std::wstring wstrVoiceFolderPath = DeriveVoiceFolderPathFromSpineFolderPath(wstrAtlasFolderPath);
	if (wstrVoiceFolderPath.empty())return false;

	std::vector<StoryDatum> storyData;
	std::wstring wstrScenarioFilePath = DeriveScenarioFilePathFromSpineFolderPath(wstrAtlasFolderPath);
	if (!wstrScenarioFilePath.empty())
	{
		ReadScenarioFile(wstrScenarioFilePath, storyData, animationNames);
	}

	for (StoryDatum& storyDatum : storyData)
	{
		if (storyDatum.wstrText.empty())continue;

		ReplaceEmoji(storyDatum.wstrText);
		EliminateTag(storyDatum.wstrText);
		std::wstring wstrVoiceFilePath;

		if (!storyDatum.wstrVoiceFileName.empty())
		{
			wstrVoiceFilePath = wstrVoiceFolderPath + L"\\" + storyDatum.wstrVoiceFileName + g_playerSetting.wstrVoiceExtension;
		}

		textData.emplace_back(adv::TextDatum{ storyDatum.wstrText, wstrVoiceFilePath, storyDatum.nAnimationIndex });
	}

	if (textData.empty())
	{
		/* 脚本ファイルなし・読み取り失敗 */
		std::vector<std::wstring> audioFilePaths;
		win_filesystem::CreateFilePathList(wstrVoiceFolderPath.c_str(), g_playerSetting.wstrVoiceExtension.c_str(), audioFilePaths);

		for (const auto& audioFilePath : audioFilePaths)
		{
			if (audioFilePath[0] != L'b')continue;
			textData.emplace_back(adv::TextDatum{ L"", audioFilePath });
		}
	}

	return !textData.empty();
}
