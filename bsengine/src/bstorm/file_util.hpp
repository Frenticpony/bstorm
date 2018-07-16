#pragma once

#include <string>
#include <vector>
#include <string>
#include <unordered_set>

namespace bstorm
{
// mkdir -p
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

std::wstring GetCanonicalPath(const std::wstring& path);
std::wstring ConcatPath(const std::wstring& a, const std::wstring& b);
std::wstring ConcatPath(std::wstring&& a, const std::wstring& b);
std::wstring GetParentPath(const std::wstring& path);
std::wstring ExpandIncludePath(const std::wstring& includerPath, const std::wstring& includeePath);
}