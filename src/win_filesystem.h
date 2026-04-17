#ifndef WIN_FILESYSTEM_H_
#define WIN_FILESYSTEM_H_

#include <string>
#include <vector>

namespace win_filesystem
{
	/// @brief 指定階層のファイル・フォルダ一覧作成
	/// @param folderPath 探索先のフォルダ経路
	/// @param fileSpec フィルタ。空文字列の場合フォルダ探索。例"*.htm;*.html"
	bool CreateFilePathList(std::wstring_view folderPath, std::wstring_view fileSpec, std::vector<std::wstring>& paths, bool toAddParent = true);

	/// @brief 大域変数を用いて実行プロセスの階層を取得
	/// @return 大域変数を指すstd::wstring_view
	std::wstring_view GetCurrentProcessPath();

	/// @brief 静的バッファを用いてフォルダ作成
	/// @param directoryName 作成する階層名
	/// @param dst 書き込み先
	/// @param nWritten 書き込まれた文字列長
	/// @param dstSize 書き込み先の容量
	/// @param basePath 基底階層。空文字列の場合実行プロセスの階層。
	/// @return 成功時true, 失敗時false
	bool CreateDirectoryToBuffer(std::wstring_view directoryName, wchar_t* dst, size_t dstSize, size_t& nWritten, std::wstring_view basePath = {});

	std::string LoadFileAsString(const wchar_t* filePath);
	bool SaveDataToFile(const wchar_t* filePath, const void* pData, unsigned long dataLength, bool toOverWrite = true);
	bool DoesFileExist(const wchar_t* filePath);

	bool RenameFile(const wchar_t* filePathOld, const wchar_t* filePathNew);
}
#endif // WIN_FILESYSTEM_H_
