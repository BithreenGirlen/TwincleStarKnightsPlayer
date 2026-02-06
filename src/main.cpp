
#include <winsdkver.h>
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN 
#include <Windows.h>

#include <locale.h>

/*SFML*/
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
#endif // _DEBUG

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

	std::wstring wstrPickedFolder = win_dialogue::SelectWorkFolder(L"Select Stills/st_XXXXXXXX folder", nullptr);
	if (!wstrPickedFolder.empty())
	{
		CSfmlMainWindow sfmlMainWindow(L"TwinkleStar Player");
		sfmlMainWindow.setFont(clst::GetFontFilePath(), true, true);

		std::vector<std::wstring> folders;
		size_t nFolderIndex = 0;
		win_filesystem::GetFilePathListAndIndex(wstrPickedFolder, nullptr, folders, &nFolderIndex);
		for (;;)
		{
			std::wstring wstrFolderPath = folders.at(nFolderIndex);

			std::vector<std::string> atlasPaths;
			std::vector<std::string> skelPaths;
			clst::GetSpineList(wstrFolderPath, atlasPaths, skelPaths);
			if (skelPaths.empty())break;

			bool bRet = sfmlMainWindow.setSpineFromFile(atlasPaths, skelPaths, clst::IsSkelBinary());
			if (!bRet)break;

			std::vector<adv::TextDatum> textData;
			std::vector<std::string> animationNames;
			clst::SearchAndLoadScenarioFile(wstrFolderPath, textData, animationNames);
			sfmlMainWindow.setScenarioData(textData, animationNames);

			int iRet = sfmlMainWindow.display();
			if (iRet == 1)
			{
				++nFolderIndex;
				if (nFolderIndex > folders.size() - 1)nFolderIndex = 0;
			}
			else if (iRet == 2)
			{
				--nFolderIndex;
				if (nFolderIndex > folders.size() - 1)nFolderIndex = folders.size() - 1;
			}
			else
			{
				break;
			}
		}
	}

	return 0;
}