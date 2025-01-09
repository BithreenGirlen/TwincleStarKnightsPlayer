
#include <Windows.h>

#include "clst.h"

#include "win_dialogue.h"
#include "win_filesystem.h"
#include "win_text.h"

#include "deps/nlohmann/json.hpp"

namespace clst
{
	struct SPlayerSetting
	{
		std::wstring wstrAtlasExtension = L"atlas.txt";
		std::wstring wstrSkelExtension = L"skel.txt";
		std::wstring wstrVoiceExtension = L".m4a";;
		std::wstring wstrSceneTextExtension = L".json";
		std::string strFontFilePath = "C:\\Windows\\Fonts\\yumindb.ttf";

		bool bSkelBinary = true;
	};

	static SPlayerSetting g_playerSetting;

	static bool ReadSettingFile(SPlayerSetting& playerSetting)
	{
		std::wstring wstrFilePath = win_filesystem::GetCurrentProcessPath() + L"\\setting.txt";
		std::string strFile = win_filesystem::LoadFileAsString(wstrFilePath.c_str());
		if (strFile.empty())return false;

		std::string strError;

		try
		{
			nlohmann::json nlJson = nlohmann::json::parse(strFile);
			std::string str;

			nlohmann::json& jExtension = nlJson.at("extensions");

			str = std::string(jExtension.at("atlas"));
			playerSetting.wstrAtlasExtension = win_text::WidenUtf8(str);

			str = std::string(jExtension.at("skel"));
			playerSetting.wstrSkelExtension = win_text::WidenUtf8(str);

			str = std::string(jExtension.at("voice"));
			playerSetting.wstrVoiceExtension = win_text::WidenUtf8(str);

			str = std::string(jExtension.at("sceneText"));
			playerSetting.wstrSceneTextExtension = win_text::WidenUtf8(str);

			playerSetting.bSkelBinary = nlJson.at("binarySkel");
			playerSetting.strFontFilePath = nlJson.at("fontPath");

		}
		catch (nlohmann::json::exception e)
		{
			strError = e.what();
			::MessageBoxA(nullptr, strError.c_str(), "Setting error", MB_ICONERROR);
		}

		return strError.empty();
	}


	struct StoryDatum
	{
		std::wstring wstrText;
		std::wstring wstrVoiceFileName;
	};

	/*
	* Path regarding "XXXXXXX" as a character ID.
	* 脚本 ../Adventure/ImportChara/CharaScenarioXXXXXXX.book.json
	* 音声 ../AssetBundles/Sound/Voice/ImportChara/XXXXXXX/
	* 画像 ../AssetBundles/Stills/st_XXXXXXX/
	*/

	/*人物ID抽出*/
	std::wstring ExtractCharacterIdFromSpineFolderPath(const std::wstring& wstrSpineFolderPath)
	{
		size_t nPos = wstrSpineFolderPath.find_last_of(L"\\/");
		if (nPos == std::wstring::npos)return std::wstring();

		std::wstring wstrCurrent = wstrSpineFolderPath.substr(nPos + 1);
		nPos = wstrCurrent.find(L"st_");
		if (nPos == std::wstring::npos)return std::wstring();

		return wstrCurrent.substr(nPos + 3);
	}

	/*ID対応先探索*/
	std::wstring FindPathContainingId(const std::wstring& targetFolder, const std::wstring& wstrId, const wchar_t* pwzFileExtension)
	{
		std::vector<std::wstring> paths;
		win_filesystem::CreateFilePathList(targetFolder.c_str(), pwzFileExtension, paths);
		const auto IsContained = [&wstrId](const std::wstring& wstr)
			-> bool
			{
				return wcsstr(wstr.c_str(), wstrId.c_str()) != nullptr;
			};

		const auto iter = std::find_if(paths.begin(), paths.end(), IsContained);
		if (iter == paths.cend())return std::wstring();

		size_t nIndex = std::distance(paths.begin(), iter);
		return paths.at(nIndex);
	}

	/*画像階層 => 音声階層*/
	std::wstring DeriveVoiceFolderPathFromSpineFolderPath(const std::wstring& wstrAtlasFolderPath)
	{
		std::wstring wstrCharacterId = ExtractCharacterIdFromSpineFolderPath(wstrAtlasFolderPath);
		if (wstrCharacterId.empty())return std::wstring();

		size_t nPos = wstrAtlasFolderPath.find(L"Stills");
		if (nPos == std::wstring::npos)return std::wstring();

		std::wstring wstrVoiceFolder = wstrAtlasFolderPath.substr(0, nPos);
		wstrVoiceFolder += L"Sound\\Voice\\ImportChara";

		return FindPathContainingId(wstrVoiceFolder, wstrCharacterId, nullptr);
	}
	/*画像階層 => 脚本経路*/
	std::wstring DeriveScenarioFilePathFromSpineFolderPath(const std::wstring& wstrAtlasFolderPath)
	{
		std::wstring wstrCharacterId = ExtractCharacterIdFromSpineFolderPath(wstrAtlasFolderPath);
		if (wstrCharacterId.empty())return std::wstring();

		size_t nPos = wstrAtlasFolderPath.find(L"AssetBundles");
		if (nPos == std::wstring::npos)return std::wstring();

		std::wstring wstrVoiceFolder = wstrAtlasFolderPath.substr(0, nPos);
		wstrVoiceFolder += L"Adventure\\ImportChara";

		std::wstring wstrScenerioId = L"CharaScenario" + wstrCharacterId + L".book";

		return FindPathContainingId(wstrVoiceFolder, wstrScenerioId, g_playerSetting.wstrSceneTextExtension.c_str());
	}

	/*脚本ファイル読み取り*/
	bool ReadScenarioFile(const std::wstring& wstrFilePath, std::vector<StoryDatum>& storyData)
	{
		std::string strFile = win_filesystem::LoadFileAsString(wstrFilePath.c_str());
		if (strFile.empty())return false;

		std::string strError;

		try
		{
			nlohmann::json nlJson = nlohmann::json::parse(strFile);
			/*[0 - 3] : 日常, [4]: 本番*/
			const nlohmann::json& jData = nlJson.at("importGridList").back().at("rows");

			/*最初の要素は項目名なので飛ばす*/
			for (size_t i = 1; i < jData.size(); ++i)
			{
				StoryDatum s;

				const nlohmann::json& jRow = jData.at(i).at("strings");
				if (jRow.size() < 14)continue;

				const std::string strText = std::string(jRow[13]);
				if (strText.empty())continue;

				const std::string strName = std::string(jRow[1]);
				if (!strName.empty())
				{
					s.wstrText = win_text::WidenUtf8(strName);
					s.wstrText += L":\n";
				}
				s.wstrText += win_text::WidenUtf8(strText);

				std::string strVoice;
				if (jRow.size() > 15)
				{
					s.wstrVoiceFileName = win_text::WidenUtf8(jRow[15]);
				}
				storyData.push_back(std::move(s));
			}
		}
		catch (nlohmann::json::exception e)
		{
			strError = e.what();
		}

		if (!strError.empty())
		{
			::MessageBoxA(nullptr, strError.c_str(), "Parse error", MB_ICONERROR);
			return false;
		}

		return true;
	}

	void ReplaceEmoji(std::wstring& src)
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

	void EliminateTag(std::wstring& wstr)
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
		wstr = wstrResult;
	}

} /*namespace clst*/


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

const bool clst::IsSkelBinary()
{
	return g_playerSetting.bSkelBinary;
}

/*描画素材一覧取得*/
void clst::GetSpineList(const std::wstring& wstrFolderPath, std::vector<std::string>& atlasPaths, std::vector<std::string>& skelPaths)
{
	bool bAtlasLonger = g_playerSetting.wstrAtlasExtension.size() > g_playerSetting.wstrSkelExtension.size();

	std::wstring& wstrLongerExtesion = bAtlasLonger ? g_playerSetting.wstrAtlasExtension : g_playerSetting.wstrSkelExtension;
	std::wstring& wstrShorterExtension = bAtlasLonger ? g_playerSetting.wstrSkelExtension : g_playerSetting.wstrAtlasExtension;
	std::vector<std::string>& strLongerPaths = bAtlasLonger ? atlasPaths : skelPaths;
	std::vector<std::string>& strShorterPaths = bAtlasLonger ? skelPaths : atlasPaths;

	std::vector<std::wstring> wstrFilePaths;
	win_filesystem::CreateFilePathList(wstrFolderPath.c_str(), L"*", wstrFilePaths);

	for (const auto& filePath : wstrFilePaths)
	{
		const auto EndsWith = [&filePath](const std::wstring& str)
			-> bool
			{
				if (filePath.size() < str.size()) return false;
				return std::equal(str.rbegin(), str.rend(), filePath.rbegin());
			};

		if (EndsWith(wstrLongerExtesion))
		{
			strLongerPaths.push_back(win_text::NarrowUtf8(filePath));
		}
		else if (EndsWith(wstrShorterExtension))
		{
			strShorterPaths.push_back(win_text::NarrowUtf8(filePath));
		}
	}
}
/*脚本ファイルの探索と読み込み*/
bool clst::SearchAndLoadScenarioFile(const std::wstring& wstrAtlasFolderPath, std::vector<adv::TextDatum>& textData)
{
	std::wstring wstrVoiceFolderPath = DeriveVoiceFolderPathFromSpineFolderPath(wstrAtlasFolderPath);
	if (wstrVoiceFolderPath.empty())return false;

	std::vector<StoryDatum> storyData;
	std::wstring wstrScenarioFilePath = DeriveScenarioFilePathFromSpineFolderPath(wstrAtlasFolderPath);
	if (!wstrScenarioFilePath.empty())
	{
		ReadScenarioFile(wstrScenarioFilePath, storyData);
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

		textData.emplace_back(adv::TextDatum{ storyDatum.wstrText, wstrVoiceFilePath });
	}

	if (textData.empty())
	{
		/*脚本ファイルなし・読み取り失敗*/
		std::vector<std::wstring> audioFilePaths;
		win_filesystem::CreateFilePathList(wstrVoiceFolderPath.c_str(), g_playerSetting.wstrVoiceExtension.c_str(), audioFilePaths);

		for (size_t i = 0; i < audioFilePaths.size(); ++i)
		{
			if (audioFilePaths.at(i).at(0) != L'b')continue;
			textData.emplace_back(adv::TextDatum{ L"", audioFilePaths.at(i) });
		}
	}

	return !textData.empty();
}
