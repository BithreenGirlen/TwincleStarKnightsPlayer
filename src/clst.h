#ifndef CLST_H_
#define CLST_H_

#include <string>
#include <vector>

#include "adv.h"

namespace clst
{
	bool InitialiseSetting();
	const std::string& GetFontFilePath();
	bool IsSkelBinary();

	void GetSpineList(const std::wstring& wstrFolderPath, std::vector<std::string>& atlasPaths, std::vector<std::string>& skelPaths);

	bool SearchAndLoadScenarioFile(const std::wstring& wstrAtlasFolderPath, std::vector<adv::TextDatum>& textData, std::vector<std::string>& animationNames);
}
#endif // !CLST_H_
