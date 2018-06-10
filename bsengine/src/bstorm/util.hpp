#pragma once

#include <string>
#include <array>
#include <vector>
#include <windows.h>
#include <algorithm>
#include <unordered_set>
#include <future>
#include <chrono>
#include <d3dx9.h>

namespace bstorm
{
template <class T>
void safe_delete(T*& p)
{
    delete p;
    p = NULL;
};

template <class T>
void safe_delete_array(T*& p)
{
    delete[] p;
    p = NULL;
};

template <class T>
void safe_release(T*& p)
{
    if (p)
    {
        p->Release();
        p = NULL;
    }
};

template <class T>
struct com_deleter
{
    void operator()(T* p)
    {
        safe_release(p);
    }
};

template <UINT cp>
std::string ToMultiByte(const std::wstring& ws)
{
    if (ws.empty()) return std::string();
    int neededSize = WideCharToMultiByte(cp, 0, &ws[0], (int)ws.size(), NULL, 0, NULL, NULL);
    std::string s(neededSize, 0);
    WideCharToMultiByte(cp, 0, &ws[0], (int)ws.size(), &s[0], neededSize, NULL, NULL);
    return s;
}

template <UINT cp>
std::wstring FromMultiByte(const std::string& s)
{
    if (s.empty()) return std::wstring();
    int neededSize = MultiByteToWideChar(cp, 0, &s[0], (int)s.size(), NULL, 0);
    std::wstring ws(neededSize, 0);
    MultiByteToWideChar(cp, 0, &s[0], (int)s.size(), &ws[0], neededSize);
    return ws;
}

inline std::string ToUTF8(const std::wstring& ws)
{
    return ToMultiByte<CP_UTF8>(ws);
}

inline std::wstring ToUnicode(const std::string& s)
{
    return FromMultiByte<CP_UTF8>(s);
}

std::vector<std::wstring> Split(const std::wstring& s, wchar_t delimiter);
std::vector<std::wstring> Split(const std::wstring& s, const std::wstring& delimiter);

bool IsMatchString(const std::string& searchText, const std::string& searchTarget);

bool IsSpace(wchar_t c);

void TrimSpace(std::wstring* s);

template <class T>
void hash_combine(std::size_t& seed, const T& v)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template <typename T>
T constrain(const T& v, const T& min, const T& max)
{
    return std::min(std::max<T>(v, min), max);
}

void MakeDirectoryP(const std::wstring& dirName);

std::wstring GetExt(const std::wstring& path);
std::wstring GetLowerExt(const std::wstring& path);
std::wstring GetStem(const std::wstring& path);
std::wstring GetFileName(const std::wstring& path);
std::wstring GetOmittedFileName(const std::wstring& path, int size);

// dir : 検索対象ディレクトリ、末尾の/はいらない、例：L"."
// ignoreExts : 検索除外対象の拡張子の集合、小文字で記述、例：{L".png", L".jpg"}
void GetFilePaths(const std::wstring& dir, std::vector<std::wstring>& pathList, const std::unordered_set<std::wstring>& ignoreExts, bool doRecurive);
void GetFilePathsRecursively(const std::wstring& dir, std::vector<std::wstring>& pathList, const std::unordered_set<std::wstring>& ignoreExts);

void GetDirs(const std::wstring& dir, std::vector<std::wstring>& dirList, bool doRecursive);
void GetDirsRecursively(const std::wstring& dir, std::vector<std::wstring>& dirList);

// 拡大、回転、移動の順番で掛けた行列を作る
D3DXMATRIX CreateScaleRotTransMatrix(float x, float y, float z, float rx, float ry, float rz, float sx, float sy, float sz);

inline int NextPow2(int x)
{
    if (x < 0) return 0;
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x + 1;
}

std::wstring GetCanonicalPath(const std::wstring& path);
std::wstring ConcatPath(const std::wstring& a, const std::wstring& b);
std::wstring GetParentPath(const std::wstring& path);
std::wstring ExpandIncludePath(const std::wstring& includerPath, const std::wstring& includeePath);

template <typename T>
bool is_shared_future_ready(const std::shared_future<T>& f)
{
    return f.wait_for(std::chrono::seconds(0)) == future_status::ready;
}
}