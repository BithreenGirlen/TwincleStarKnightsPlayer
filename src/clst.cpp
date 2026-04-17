
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
		std::string settingFile = win_filesystem::LoadFileAsString(L"setting.txt");
		if (settingFile.empty())return false;

		try
		{
			const nlohmann::json& nlJson = nlohmann::json::parse(settingFile);
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

		/* There should be another struct to manage animation decently, but it requires much more code refactoring. */
		size_t nAnimationIndex = 0;
	};

	/*
	* Path regarding "XXXXXXX" as a character ID.
	* 脚本 ../Adventure/ImportChara/CharaScenarioXXXXXXX.book.json
	* 音声 ../AssetBundles/Sound/Voice/ImportChara/XXXXXXX/
	* 画像 ../AssetBundles/Stills/st_XXXXXXX/
	*/

	/* 人物ID抽出 */
	static std::wstring_view ExtractCharacterIdFromSpineFolderPath(const std::wstring& stillSpineFolderPath)
	{
		size_t nPos = stillSpineFolderPath.find_last_of(L"\\/");
		if (nPos == std::wstring::npos)return {};
		++nPos;

		nPos = stillSpineFolderPath.find(L"st_", nPos);
		if (nPos == std::wstring::npos)return {};
		nPos += 3;

		return { &stillSpineFolderPath[nPos], stillSpineFolderPath.length() - nPos };
	}

	/* ID対応先探索 */
	static std::wstring FindPathContainingId(const std::wstring& targetFolder, const std::wstring_view Id, const std::wstring_view fileSpec)
	{
		std::vector<std::wstring> paths;
		win_filesystem::CreateFilePathList(targetFolder, fileSpec, paths);
		const auto IsContained = [&Id](const std::wstring& wstr)
			-> bool
			{
				return wstr.find(Id) != std::wstring::npos;
			};

		const auto& iter = std::find_if(paths.begin(), paths.end(), IsContained);
		if (iter == paths.cend())return {};

		return *iter;
	}

	/* 画像階層 => 音声階層 */
	static std::wstring DeriveVoiceFolderPathFromSpineFolderPath(const std::wstring& stillSpineFolderPath)
	{
		std::wstring_view characterId = ExtractCharacterIdFromSpineFolderPath(stillSpineFolderPath);
		if (characterId.empty())return {};

		size_t nPos = stillSpineFolderPath.find(L"Stills");
		if (nPos == std::wstring::npos)return {};

		std::wstring voiceFolderPath = stillSpineFolderPath.substr(0, nPos).append(L"Sound\\Voice\\ImportChara");

		return FindPathContainingId(voiceFolderPath, characterId, {});
	}
	/* 画像階層 => 脚本経路 */
	static std::wstring DeriveScenarioFilePathFromSpineFolderPath(const std::wstring& stillSpineFolderPath)
	{
		std::wstring_view characterId = ExtractCharacterIdFromSpineFolderPath(stillSpineFolderPath);
		if (characterId.empty())return {};

		size_t nPos = stillSpineFolderPath.find(L"AssetBundles");
		if (nPos == std::wstring::npos)return {};

		std::wstring scenarioFolderPath = stillSpineFolderPath.substr(0, nPos).append(L"Adventure\\ImportChara");

		std::wstring scenerioId = std::wstring(L"CharaScenario").append(characterId).append(L".book");

		return FindPathContainingId(scenarioFolderPath, scenerioId, g_playerSetting.wstrSceneTextExtension.c_str());
	}

	/* 脚本ファイル読み取り */
	static bool ReadScenarioFile(const std::wstring& wstrFilePath, std::vector<StoryDatum>& storyData, std::vector<std::string>& animationNames)
	{
		std::string scenarioFile = win_filesystem::LoadFileAsString(wstrFilePath.c_str());
		if (scenarioFile.empty())return false;

		size_t nCurrentAnimationIndex = 0;
		try
		{
			const nlohmann::json& nlJson = nlohmann::json::parse(scenarioFile);
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
		static constexpr std::wstring_view toBeReplaced = L"<emoji=heart04>";
		for (size_t nRead = 0;;)
		{
			size_t nPos = src.find(toBeReplaced, nRead);
			if (nPos == std::wstring::npos) break;
			src.replace(nPos, toBeReplaced.length(), L"♡");
			nRead = nPos + 1;
		}
	}

	static void EliminateTagInPlace(std::wstring& wstr)
	{
		size_t nWritten = 0;
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
				wstr[nWritten++] = c;
			}
		}

		wstr.resize(nWritten);
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
	constexpr const std::wstring_view binaryCandidates[] = { L".skel", L".bin", L".bytes" };

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
void clst::GetSpineList(const std::wstring& spineFolderPath, std::vector<std::string>& atlasPaths, std::vector<std::string>& skelPaths)
{
	bool isAtlasLonger = g_playerSetting.wstrAtlasExtension.size() > g_playerSetting.wstrSkelExtension.size();

	std::wstring& longerExtesion = isAtlasLonger ? g_playerSetting.wstrAtlasExtension : g_playerSetting.wstrSkelExtension;
	std::wstring& shorterExtension = isAtlasLonger ? g_playerSetting.wstrSkelExtension : g_playerSetting.wstrAtlasExtension;
	std::vector<std::string>& longerPaths = isAtlasLonger ? atlasPaths : skelPaths;
	std::vector<std::string>& shorterPaths = isAtlasLonger ? skelPaths : atlasPaths;

	std::vector<std::wstring> filePaths;
	win_filesystem::CreateFilePathList(spineFolderPath, L"*", filePaths);

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
bool clst::SearchAndLoadScenarioFile(const std::wstring& stillSpineFolderPath, std::vector<adv::TextDatum>& textData, std::vector<std::string>& animationNames)
{
	std::wstring voiceFolderPath = DeriveVoiceFolderPathFromSpineFolderPath(stillSpineFolderPath);
	if (voiceFolderPath.empty())return false;

	std::vector<StoryDatum> storyData;
	std::wstring scenarioFilePath = DeriveScenarioFilePathFromSpineFolderPath(stillSpineFolderPath);
	if (!scenarioFilePath.empty())
	{
		ReadScenarioFile(scenarioFilePath, storyData, animationNames);
	}

	for (StoryDatum& storyDatum : storyData)
	{
		if (storyDatum.wstrText.empty())continue;

		ReplaceEmoji(storyDatum.wstrText);
		EliminateTagInPlace(storyDatum.wstrText);
		std::wstring voiceFilePath;

		if (!storyDatum.wstrVoiceFileName.empty())
		{
			voiceFilePath.assign(voiceFolderPath).append(L"\\").append(storyDatum.wstrVoiceFileName).append(g_playerSetting.wstrVoiceExtension);
		}

		textData.emplace_back(adv::TextDatum{ storyDatum.wstrText, voiceFilePath, storyDatum.nAnimationIndex });
	}

	if (textData.empty())
	{
		/* 脚本ファイルなし・読み取り失敗 */
		std::vector<std::wstring> audioFilePaths;
		win_filesystem::CreateFilePathList(voiceFolderPath, g_playerSetting.wstrVoiceExtension, audioFilePaths);

		for (const auto& audioFilePath : audioFilePaths)
		{
			if (audioFilePath[0] != L'b')continue;
			textData.emplace_back(adv::TextDatum{ L"", audioFilePath });
		}
	}

	return !textData.empty();
}
