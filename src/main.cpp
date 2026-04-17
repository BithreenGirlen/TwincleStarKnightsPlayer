
#include <winsdkver.h>
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN 
#include <Windows.h>

#include <locale.h>

/* SFML */
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "winmm.lib")

#ifdef  _DEBUG
#pragma comment(lib, "sfml-system-d.lib")
#pragma comment(lib, "sfml-graphics-d.lib")
#pragma comment(lib, "sfml-window-d.lib")
#else
#pragma comment(lib, "sfml-system.lib")
#pragma comment(lib, "sfml-graphics.lib")
#pragma comment(lib, "sfml-window.lib")
#endif

#include "win_dialogue.h"
#include "win_filesystem.h"
#include "clst.h"
#include "sfml_main_window.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	::setlocale(LC_ALL, ".utf8");

	bool bRet = clst::InitialiseSetting();
	if (!bRet)return 0;

	std::wstring selectedFolderPath = win_dialogue::SelectFolder(L"Select Stills/st_XXXXXXXX folder", nullptr);
	if (selectedFolderPath.empty())return 0;

	std::vector<std::wstring> folderPaths;
	size_t nFolderPathIndex = 0;
	if (size_t nPos = selectedFolderPath.find_last_of(L"\\/"); nPos != std::wstring::npos)
	{
		std::wstring_view parentFolderPath(&selectedFolderPath[0], nPos);
		win_filesystem::CreateFilePathList(parentFolderPath, {}, folderPaths);
		if (const auto& iter = std::find(folderPaths.begin(), folderPaths.end(), selectedFolderPath); iter != folderPaths.cend())
		{
			nFolderPathIndex = std::distance(folderPaths.begin(), iter);
		}
	}
	if (folderPaths.empty())return 0;

	CSfmlMainWindow sfmlMainWindow(U"TwinkleStar Player");
	sfmlMainWindow.setFont(clst::GetFontFilePath(), true, true);

	for (;;)
	{
		const std::wstring& folderPath = folderPaths[nFolderPathIndex];

		std::vector<std::string> atlasFilePaths;
		std::vector<std::string> skelFilePaths;
		clst::GetSpineList(folderPath, atlasFilePaths, skelFilePaths);
		if (skelFilePaths.empty())break;

		bool bRet = sfmlMainWindow.setSpineFromFile(atlasFilePaths, skelFilePaths, clst::IsSkelBinary());
		if (!bRet)break;

		std::vector<adv::TextDatum> textData;
		std::vector<std::string> animationNames;
		clst::SearchAndLoadScenarioFile(folderPath, textData, animationNames);
		sfmlMainWindow.setScenarioData(textData, animationNames);

		int iRet = sfmlMainWindow.display();
		if (iRet == 1)
		{
			++nFolderPathIndex;
			if (nFolderPathIndex > folderPaths.size() - 1)nFolderPathIndex = 0;
		}
		else if (iRet == 2)
		{
			--nFolderPathIndex;
			if (nFolderPathIndex > folderPaths.size() - 1)nFolderPathIndex = folderPaths.size() - 1;
		}
		else
		{
			break;
		}
	}

	return 0;
}