#include <bstorm/util.hpp>

#include <sstream>
#include <regex>

namespace bstorm
{
void MakeDirectoryP(const std::wstring& dirName)
{
    auto attr = GetFileAttributes(dirName.c_str());
    if (attr != -1 && (attr & FILE_ATTRIBUTE_DIRECTORY)) return;
    std::wstring dir = L"";
    for (auto& name : Split(GetCanonicalPath(dirName), L'/'))
    {
        if (name.empty()) break;
        dir += name + L"/";
        _wmkdir(dir.c_str());
    }
}

std::wstring GetExt(const std::wstring& path)
{
    auto found = path.find_last_of(L".");
    return (found != std::string::npos) ? path.substr(found) : L"";
}

std::wstring GetLowerExt(const std::wstring& path)
{
    auto ext = GetExt(path);
    std::transform(ext.begin(), ext.end(), ext.begin(), tolower);
    return ext;
}

std::wstring GetStem(const std::wstring & path)
{
    std::wstring fileName = GetFileName(path);
    auto found = fileName.find_last_of(L".");
    if (found != std::string::npos)
    {
        return fileName.substr(0, found);
    } else
    {
        return fileName;
    }
}

std::wstring GetFileName(const std::wstring & path)
{
    auto found = path.find_last_of(L"/\\");
    if (found != std::string::npos)
    {
        return path.substr(found + 1);
    } else
    {
        return path;
    }
}

std::wstring GetOmittedFileName(const std::wstring & path, int size)
{
    auto stem = GetStem(path);
    auto ext = GetExt(path);
    std::wstring ret;
    int omittedStemSize = std::max(0, size - (int)ext.size());
    ret += stem.substr(0, omittedStemSize);
    if (omittedStemSize < stem.size())
    {
        ret += L"…";
    }
    ret += ext;
    return ret;
}

void GetFilePathsRecursively(const std::wstring& dir, std::vector<std::wstring>& pathList, const std::unordered_set<std::wstring>& ignoreExts)
{
    GetFilePaths(dir, pathList, ignoreExts, true);
}

void GetFilePaths(const std::wstring & dir, std::vector<std::wstring>& pathList, const std::unordered_set<std::wstring>& ignoreExts, bool doRecursive)
{
    if (dir.empty()) return;
    WIN32_FIND_DATA data;
    HANDLE fh = FindFirstFile((dir + L"/*").c_str(), &data);
    if (fh == INVALID_HANDLE_VALUE) return;
    do
    {
        std::wstring fileName(data.cFileName);
        if (fileName == L".." || fileName == L".")
        {
            continue;
        }
        std::wstring path = ConcatPath(dir, fileName);
        if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if (doRecursive) GetFilePathsRecursively(path, pathList, ignoreExts);
        } else
        {
            const auto ext = GetLowerExt(path);
            if (ignoreExts.count(ext) == 0)
            {
                pathList.push_back(path);
            }
        }
    } while (FindNextFile(fh, &data));
    FindClose(fh);
}

void GetDirs(const std::wstring & dir, std::vector<std::wstring>& dirList, bool doRecursive)
{
    if (dir.empty()) return;
    WIN32_FIND_DATA data;
    HANDLE fh = FindFirstFile((dir + L"/*").c_str(), &data);
    if (fh == INVALID_HANDLE_VALUE) return;
    do
    {
        std::wstring fileName(data.cFileName);
        if (fileName == L".." || fileName == L".")
        {
            continue;
        }
        std::wstring path = ConcatPath(dir, fileName);
        if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            dirList.push_back(path);
            if (doRecursive) GetDirsRecursively(path, dirList);
        }
    } while (FindNextFile(fh, &data));
    FindClose(fh);
}

void GetDirsRecursively(const std::wstring & dir, std::vector<std::wstring>& dirList)
{
    GetDirs(dir, dirList, true);
}

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
    return c == L' ' || c == L'\t' || c == L'\n' || c == L'\r' || c == L'\f' || c == L'\v' || c == L'\b';
}

void TrimSpace(std::wstring * s)
{
    std::wstring::iterator it_left = std::find_if_not(s->begin(), s->end(), IsSpace);
    s->erase(s->begin(), it_left);

    std::wstring::iterator it_right = std::find_if_not(s->rbegin(), s->rend(), IsSpace).base();
    s->erase(it_right, s->end());
}

std::wstring GetCanonicalPath(const std::wstring& path)
{
    // expand relative path
    const size_t pathSize = path.size();
    std::vector<std::wstring> trail; trail.reserve(16);
    {
        std::wstring tmp;
        size_t i = 0;
        for (; i < pathSize; ++i)
        {
            const char& c = path[i];
            switch (c)
            {
                case '/':
                case '\\':
separator:
                    if (tmp.empty())
                    {
                    } else if (tmp == L".")
                    {
                        // ignore
                        tmp.pop_back();
                    } else if (tmp == L"..")
                    {
                        if (trail.empty())
                        {
                            trail.emplace_back(L"..");
                        } else
                        {
                            trail.pop_back();
                        }
                        tmp.clear();
                    } else
                    {
                        trail.push_back(std::move(tmp));
                    }
                    break;
                default:
                    tmp.push_back(c);
                    break;

            }
        }

        if (i == pathSize) goto separator; // end of string
    }

    // concat
    std::wstring ret; ret.reserve(pathSize);
    for (const auto& s : trail)
    {
        ret += s;
        ret.push_back(L'/');
    }
    if (!ret.empty())
    {
        ret.pop_back();
    }
    return ret;
}

std::wstring ConcatPath(const std::wstring& a, const std::wstring& b)
{
    if (!a.empty() && (a.back() == L'/' || a.back() == L'\\')) return a + b;
    return a + L"/" + b;
}

std::wstring GetParentPath(const std::wstring& path)
{
    auto found = path.find_last_of(L"/\\");
    auto ret = path.substr(0, found);
    if (ret == path)
    {
        return L".";
    }
    return ret;
}

std::wstring ExpandIncludePath(const std::wstring & includerPath, const std::wstring & includeePath)
{
    if (includeePath.substr(0, 2) == L"./" ||
        includeePath.substr(0, 2) == L".\\" ||
        includeePath.substr(0, 3) == L"../" ||
        includeePath.substr(0, 3) == L"..\\")
    {
        return GetCanonicalPath(ConcatPath(GetParentPath(includerPath), includeePath));
    }
    return includeePath;
}
}