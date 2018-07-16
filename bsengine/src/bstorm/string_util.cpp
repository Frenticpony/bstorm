#include <bstorm/string_util.hpp>

#include <sstream>
#include <regex>

namespace bstorm
{
std::vector<std::wstring> Split(const std::wstring& s, wchar_t delimiter)
{
    std::vector<std::wstring> r;
    std::wistringstream ss(s);
    std::wstring word;
    while (std::getline(ss, word, delimiter))
    {
        r.push_back(word);
    }
    return r;
}

std::vector<std::wstring> Split(const std::wstring& s, const std::wstring& delimiter)
{
    std::vector<std::wstring> r;
    std::wstring::size_type pos = 0;
    while (pos != std::wstring::npos)
    {
        std::string::size_type p = s.find(delimiter, pos);
        if (p == std::string::npos)
        {
            r.push_back(s.substr(pos));
            break;
        } else
        {
            r.push_back(s.substr(pos, p - pos));
        }
        pos = p + delimiter.size();
    }
    return r;
}

bool IsMatchString(const std::string& searchText, const std::string& searchTarget)
{
    return searchTarget.find(searchText) != std::string::npos;
}

bool IsSpace(wchar_t c)
{
    switch (c)
    {
        case L' ':
        case L'\t':
        case L'\n':
        case L'\r':
        case L'\f':
        case L'\v':
        case L'\b':
            return true;
        default:
            return false;
    }
}

void TrimSpace(std::wstring * s)
{
    std::wstring::iterator it_left = std::find_if_not(s->begin(), s->end(), IsSpace);
    s->erase(s->begin(), it_left);

    std::wstring::iterator it_right = std::find_if_not(s->rbegin(), s->rend(), IsSpace).base();
    s->erase(it_right, s->end());
}

}