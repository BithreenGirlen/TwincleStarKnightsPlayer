
#include <Windows.h>
#include <shlwapi.h>

#include "win_filesystem.h"

#pragma comment(lib, "Shlwapi.lib")

namespace win_filesystem
{
	/*ファイルのメモリ展開*/
	static char* LoadExistingFile(const wchar_t* pwzFilePath, unsigned long* ulSize)
	{
		HANDLE hFile = ::CreateFileW(pwzFilePath, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			DWORD dwSize = ::GetFileSize(hFile, nullptr);
			if (dwSize != INVALID_FILE_SIZE)
			{
				char* pBuffer = static_cast<char*>(malloc(static_cast<size_t>(dwSize + 1ULL)));
				if (pBuffer != nullptr)
				{
					DWORD dwRead = 0;
					BOOL iRet = ::ReadFile(hFile, pBuffer, dwSize, &dwRead, nullptr);
					if (iRet)
					{
						::CloseHandle(hFile);
						*(pBuffer + dwRead) = '\0';
						*ulSize = dwRead;

						return pBuffer;
					}
					else
					{
						free(pBuffer);
					}
				}
			}
			::CloseHandle(hFile);
		}

		return nullptr;
	}
	/*指定階層のファイル・フォルダ名一覧取得*/
	static bool CreateFilaNameList(const std::wstring& wstrFolderPath, const wchar_t* pwzFileNamePattern, std::vector<std::wstring>& wstrNames)
	{
		std::wstring wstrPath = wstrFolderPath;
		if (pwzFileNamePattern != nullptr)
		{
			if (wcschr(pwzFileNamePattern, L'*') == nullptr)
			{
				wstrPath += L'*';
			}
			wstrPath += pwzFileNamePattern;
		}
		else
		{
			wstrPath += L'*';
		}

		WIN32_FIND_DATAW sFindData;

		HANDLE hFind = ::FindFirstFileW(wstrPath.c_str(), &sFindData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			if (pwzFileNamePattern != nullptr)
			{
				do
				{
					/*ファイル一覧*/
					if (!(sFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
					{
						wstrNames.push_back(sFindData.cFileName);
					}
				} while (::FindNextFileW(hFind, &sFindData));
			}
			else
			{
				do
				{
					/*フォルダ一覧*/
					if ((sFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
					{
						if (wcscmp(sFindData.cFileName, L".") != 0 && wcscmp(sFindData.cFileName, L"..") != 0)
						{
							wstrNames.push_back(sFindData.cFileName);
						}
					}
				} while (::FindNextFileW(hFind, &sFindData));
			}

			::FindClose(hFind);
		}
		return wstrNames.size() > 0;
	}

	static bool MakeDirectory(const std::wstring& wstrPath)
	{
		BOOL iRet = ::CreateDirectoryW(wstrPath.c_str(), nullptr);
		if (iRet == 0)
		{
			if (::GetLastError() == ERROR_ALREADY_EXISTS)
			{
				return true;
			}
		}
		else
		{
			return true;
		}

		return false;
	}
}

/*指定階層のファイル・フォルダ一覧作成*/
bool win_filesystem::CreateFilePathList(const wchar_t* pwzFolderPath, const wchar_t* pwzFileSpec, std::vector<std::wstring>& paths, bool toAddParent)
{
	if (pwzFolderPath == nullptr || pwzFolderPath[0] == L'\0')return false;

	std::wstring wstrParent = pwzFolderPath;
	if (wstrParent.back() != L'\\')
	{
		wstrParent += L"\\";
	}
	std::vector<std::wstring> wstrNames;

	if (pwzFileSpec != nullptr)
	{
		const auto SplitSpecs = [](const wchar_t* pwzFileSpec, std::vector<std::wstring>& specs)
			-> void
			{
				std::wstring wstrTemp;
				for (const wchar_t* p = pwzFileSpec; *p != L'\0' && p != nullptr; ++p)
				{
					if (*p == L';')
					{
						if (!wstrTemp.empty())
						{
							specs.push_back(wstrTemp);
							wstrTemp.clear();
						}
						continue;
					}

					wstrTemp.push_back(*p);
				}

				if (!wstrTemp.empty())
				{
					specs.push_back(wstrTemp);
				}
			};
		std::vector<std::wstring> specs;
		SplitSpecs(pwzFileSpec, specs);

		for (const auto& spec : specs)
		{
			CreateFilaNameList(wstrParent, spec.c_str(), wstrNames);
		}
	}
	else
	{
		CreateFilaNameList(wstrParent, pwzFileSpec, wstrNames);
	}

	/*名前順に整頓*/
	for (size_t i = 0; i < wstrNames.size(); ++i)
	{
		size_t nIndex = i;
		for (size_t j = i; j < wstrNames.size(); ++j)
		{
			if (::StrCmpLogicalW(wstrNames[nIndex].c_str(), wstrNames[j].c_str()) > 0)
			{
				nIndex = j;
			}
		}
		std::swap(wstrNames[i], wstrNames[nIndex]);
	}

	if (paths.empty())
	{
		paths = std::move(wstrNames);
		if (toAddParent)
		{
			for (std::wstring& path : paths)
			{
				path = wstrParent + path;
			}
		}
	}
	else
	{
		for (const std::wstring& wstr : wstrNames)
		{
			if (toAddParent)paths.emplace_back(wstrParent + wstr);
			else paths.push_back(wstr);
		}
	}

	return paths.size() > 0;
}
/*指定経路と同階層のファイル・フォルダ一覧作成・相対位置取得*/
bool win_filesystem::GetFilePathListAndIndex(const std::wstring& wstrPath, const wchar_t* pwzFileSpec, std::vector<std::wstring>& paths, size_t* nIndex)
{
	std::wstring wstrParent;

	size_t nPos = wstrPath.find_last_of(L"\\/");
	if (nPos != std::wstring::npos)
	{
		wstrParent = wstrPath.substr(0, nPos);
	}

	win_filesystem::CreateFilePathList(wstrParent.c_str(), pwzFileSpec, paths);

	const auto& iter = std::find(paths.begin(), paths.end(), wstrPath);
	if (iter != paths.cend())
	{
		*nIndex = std::distance(paths.begin(), iter);
	}

	return iter != paths.cend();
}
/*文字列としてファイル読み込み*/
std::string win_filesystem::LoadFileAsString(const wchar_t* pwzFilePath)
{
	DWORD ulSize = 0;
	char* pBuffer = LoadExistingFile(pwzFilePath, &ulSize);
	if (pBuffer != nullptr)
	{
		std::string str;
		str.resize(ulSize);
		memcpy(&str[0], pBuffer, ulSize);

		free(pBuffer);
		return str;
	}

	return std::string();
}

std::wstring win_filesystem::GetCurrentProcessPath()
{
	wchar_t sBuffer[MAX_PATH]{};
	DWORD ulLength = ::GetModuleFileNameW(nullptr, sBuffer, MAX_PATH);
	if (ulLength == 0)return {};

	const wchar_t* p = sBuffer + ulLength;
	for (; p != sBuffer; --p)
	{
		if (*p == L'\\' || *p == '/')break;
	}
	return std::wstring(sBuffer, p - sBuffer);
}

std::wstring win_filesystem::CreateWorkFolder(const std::wstring& wstrRelativePath)
{
	if (wstrRelativePath.empty())return std::wstring();

	std::wstring wstrPath = GetCurrentProcessPath();
	if (wstrPath.empty())return std::wstring{};

	wstrPath.push_back(L'\\');
	size_t nRead = 0;
	if (wstrRelativePath[0] == L'\\' || wstrRelativePath[0] == L'/')++nRead;

	for (const wchar_t* pStart = wstrRelativePath.data();;)
	{
		size_t nPos = wstrRelativePath.find_first_of(L"\\/", nRead);
		if (nPos == std::wstring::npos)
		{
			wstrPath.append(pStart + nRead, wstrRelativePath.size() - nRead);
			wstrPath.push_back(L'\\');
			::CreateDirectoryW(wstrPath.c_str(), nullptr);

			break;
		}
		wstrPath.append(pStart + nRead, nPos - nRead);
		wstrPath.push_back(L'\\');
		::CreateDirectoryW(wstrPath.c_str(), nullptr);

		nRead = nPos + 1;
	}

	return wstrPath;
}

bool win_filesystem::CreateFolderByAbsolutePath(const std::wstring& wstrAbsolutePath)
{
	std::wstring wstrPath = wstrAbsolutePath;
	for (unsigned int i = 0;;)
	{
		bool bRet = MakeDirectory(wstrPath);
		if (bRet)
		{
			if (i == 0)return true;
			else
			{
				size_t nPos = wstrPath.find(L'\0');
				if (nPos == std::wstring::npos)return false;
				wstrPath[nPos] = L'\\';
				--i;
				continue;
			}
		}

		size_t nPos = wstrPath.find_last_of(L"\\/");
		if (nPos == std::wstring::npos)return false;
		wstrPath[nPos] = L'\0';
		++i;
	}

	return false;
}

bool win_filesystem::SaveStringToFile(const wchar_t* pwzFilePath, const char* szData, unsigned long ulDataLength, bool bOverWrite)
{
	if (pwzFilePath != nullptr)
	{
		HANDLE hFile = ::CreateFileW(pwzFilePath, GENERIC_WRITE, 0, nullptr, bOverWrite ? CREATE_ALWAYS : OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			::SetFilePointer(hFile, NULL, nullptr, FILE_END);
			DWORD dwWritten = 0;
			BOOL iRet = ::WriteFile(hFile, szData, ulDataLength, &dwWritten, nullptr);
			::CloseHandle(hFile);
			return iRet == TRUE;
		}
	}
	return false;
}

bool win_filesystem::DoesFileExist(const wchar_t* pwzFilePath)
{
	return ::PathFileExistsW(pwzFilePath) == TRUE;
}

bool win_filesystem::RenameFile(const wchar_t* swzExistingFilePath, const wchar_t* swzNewFilePath)
{
	return ::MoveFileW(swzExistingFilePath, swzNewFilePath) != 0;
}
