#pragma once

#include <string>
#include <array>
#include <windows.h>
#include <algorithm>
#include <unordered_set>
#include <d3dx9.h>

#include <bstorm/type.hpp>

namespace bstorm {
  template <class T>
  void safe_delete(T*& p) {
    delete p;
    p = NULL;
  };

  template <class T>
  void safe_delete_array(T*& p) {
    delete[] p;
    p = NULL;
  };

  template <class T>
  void safe_release(T*& p) {
    if (p) {
      p->Release();
      p = NULL;
    }
  };

  template <UINT cp>
  std::string toMultiByte(const std::wstring& ws) {
    if (ws.empty()) return std::string();
    int neededSize = WideCharToMultiByte(cp, 0, &ws[0], (int)ws.size(), NULL, 0, NULL, NULL);
    std::string s(neededSize, 0);
    WideCharToMultiByte(cp, 0, &ws[0], (int)ws.size(), &s[0], neededSize, NULL, NULL);
    return s;
  }

  template <UINT cp>
  std::wstring fromMultiByte(const std::string& s) {
    if (s.empty()) return std::wstring();
    int neededSize = MultiByteToWideChar(cp, 0, &s[0], (int)s.size(), NULL, 0);
    std::wstring ws(neededSize, 0);
    MultiByteToWideChar(cp, 0, &s[0], (int)s.size(), &ws[0], neededSize);
    return ws;
  }

  inline std::string toUTF8(const std::wstring& ws) {
    return toMultiByte<CP_UTF8>(ws);
  }

  inline std::wstring toUnicode(const std::string& s) {
    return fromMultiByte<CP_UTF8>(s);
  }

  std::vector<std::wstring> split(const std::wstring& s, wchar_t delimiter);
  std::vector<std::wstring> split(const std::wstring& s, const std::wstring& delimiter);

  bool isSpace(wchar_t c);

  void trimSpace(std::wstring& s);

  template <class T>
  void hash_combine(std::size_t& seed, const T& v) {
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  }

  template <typename T>
  T constrain(const T& v, const T& min, const T& max) {
    return std::min(std::max<T>(v, min), max);
  }

  D3DCOLOR toD3DCOLOR(const ColorRGB& rgb, int alpha);

  void mkdir_p(const std::wstring& dirName);

  std::wstring getExt(const std::wstring& path);
  std::wstring getLowerExt(const std::wstring& path);
  std::wstring getStem(const std::wstring& path);
  std::wstring getFileName(const std::wstring& path);

  // dir : 検索対象ディレクトリ、末尾の/はいらない、例：L"."
  // ignoreExts : 検索除外対象の拡張子の集合、小文字で記述、例：{L".png", L".jpg"}
  void getFilePaths(const std::wstring& dir, std::vector<std::wstring>& pathList, const std::unordered_set<std::wstring>& ignoreExts, bool doRecurive);
  void getFilePathsRecursively(const std::wstring& dir, std::vector<std::wstring>& pathList, const std::unordered_set<std::wstring>& ignoreExts);

  void getDirs(const std::wstring& dir, std::vector<std::wstring>& dirList, bool doRecursive);
  void getDirsRecursively(const std::wstring& dir, std::vector<std::wstring>& dirList);

  D3DXMATRIX rotScaleTrans(float x, float y, float z, float rx, float ry, float rz, float sx, float sy, float sz);
  std::array<Vertex, 4> rectToVertices(D3DCOLOR color, int textureWidth, int textureHeight, const Rect<int> &rect);

  inline int nextPow2(int x) {
    if (x < 0) return 0;
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x + 1;
  }

  std::wstring canonicalPath(const std::wstring& path);
  std::wstring concatPath(const std::wstring& a, const std::wstring& b);
  std::wstring parentPath(const std::wstring& path);
  std::wstring expandIncludePath(const std::wstring& includerPath, const std::wstring& includeePath);

  std::string readResourceText(int resourceId);
}