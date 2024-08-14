
#include "clst.h"

#include "win_dialogue.h"
#include "win_filesystem.h"
#include "win_text.h"

#include "deps/nlohmann/json.hpp"

namespace clst
{
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

		return FindPathContainingId(wstrVoiceFolder, wstrScenerioId, L".json");
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
				const nlohmann::json& jRow = jData.at(i).at("strings");
				if (jRow.size() < 14)continue;

				const std::string strText = std::string(jRow.at(13));
				if (strText.empty())continue;

				std::string strVoice;
				if (jRow.size() > 15)
				{
					strVoice = std::string(jRow.at(15));
				}
				storyData.emplace_back(StoryDatum{ win_text::WidenUtf8(strText), win_text::WidenUtf8(strVoice) });
			}
		}
		catch (nlohmann::json::exception e)
		{
			strError = e.what();
		}

		if (!strError.empty())
		{
			win_dialogue::ShowMessageBox("Parse error", strError.c_str());
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

/*描画素材一覧取得*/
void clst::GetSpineList(const std::wstring& wstrFolderPath, std::vector<std::string>& atlasPaths, std::vector<std::string>& skelPaths)
{
	/*---------------------
	* atlas | *.atlas.txt
	* skel  | *.skel.txt
	* json  | *.txt
	*---------------------*/
	std::vector<std::wstring> filePaths;
	win_filesystem::CreateFilePathList(wstrFolderPath.c_str(), L".txt", filePaths);
	for (const std::wstring& filePath : filePaths)
	{
		if (filePath.rfind(L".atlas") != std::wstring::npos)
		{
			atlasPaths.push_back(win_text::NarrowANSI(filePath));
		}
		else
		{
			skelPaths.push_back(win_text::NarrowANSI(filePath));
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
			wstrVoiceFilePath = wstrVoiceFolderPath + L"\\" + storyDatum.wstrVoiceFileName + L".m4a";
		}

		textData.emplace_back(adv::TextDatum{ storyDatum.wstrText, wstrVoiceFilePath });
	}

	if (textData.empty())
	{
		/*脚本ファイルなし・読み取り失敗*/
		std::vector<std::wstring> audioFilePaths;
		win_filesystem::CreateFilePathList(wstrVoiceFolderPath.c_str(), L".m4a", audioFilePaths);

		for (size_t i = 0; i < audioFilePaths.size(); ++i)
		{
			if (audioFilePaths.at(i).at(0) != L'b')continue;
			textData.emplace_back(adv::TextDatum{ L"", audioFilePaths.at(i) });
		}
	}

	return !textData.empty();
}
