#ifndef WIN_TEXT_H_
#define WIN_TEXT_H_

#include <string>

namespace win_text
{
    std::wstring WidenUtf8(const std::string& str);
    std::wstring WidenUtf8(const char* str, int length);

    std::string NarrowUtf8(const std::wstring& wstr);
    std::string NarrowUtf8(const wchar_t* wstr, int length);

    std::wstring WidenAnsi(const std::string& str);
    std::wstring WidenAnsi(const char* str, int length);

    std::string NarrowAnsi(const std::wstring& wstr);
    std::string NarrowAnsi(const wchar_t* wstr, int length);
}

#endif //WIN_TEXT_H_
