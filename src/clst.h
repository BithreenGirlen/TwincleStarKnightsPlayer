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

	void GetSpineList(const std::wstring& spineFolderPath, std::vector<std::string>& atlasFilePaths, std::vector<std::string>& skelFilePaths);

	bool SearchAndLoadScenarioFile(const std::wstring& stillSpineFolderPath, std::vector<adv::TextDatum>& textData, std::vector<std::string>& animationNames);
}
#endif // !CLST_H_
