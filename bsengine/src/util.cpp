#include <sstream>
#include <regex>
#include <vector>

#include <bstorm/util.hpp>

namespace bstorm {
  D3DCOLOR toD3DCOLOR(const ColorRGB& rgb, int alpha) {
    alpha = constrain(alpha, 0, 0xff);
    return D3DCOLOR_ARGB(alpha, rgb.getR(), rgb.getG(), rgb.getB());
  }

  void mkdir_p(const std::wstring& dirName) {
    auto attr = GetFileAttributes(dirName.c_str());
    if (attr != -1 && (attr & FILE_ATTRIBUTE_DIRECTORY)) return;
    std::wstring dir = L"";
    for (auto& name : split(canonicalPath(dirName), L'/')) {
      if (name.empty()) break;
      dir += name + L"/";
      _wmkdir(dir.c_str());
    }
  }

  std::wstring getExt(const std::wstring& path) {
    auto found = path.find_last_of(L".");
    return (found != std::string::npos) ? path.substr(found) : L"";
  }

  std::wstring getLowerExt(const std::wstring& path) {
    auto ext = getExt(path);
    std::transform(ext.begin(), ext.end(), ext.begin(), tolower);
    return ext;
  }

  std::wstring getStem(const std::wstring & path) {
    std::wstring fileName = getFileName(path);
    auto found = fileName.find_last_of(L".");
    if (found != std::string::npos) {
      return fileName.substr(0, found);
    } else {
      return fileName;
    }
  }

  std::wstring getFileName(const std::wstring & path) {
    auto found = path.find_last_of(L"/\\");
    if (found != std::string::npos) {
      return path.substr(found + 1);
    } else {
      return path;
    }
  }

  void getFilePathsRecursively(const std::wstring& dir, std::vector<std::wstring>& pathList, const std::unordered_set<std::wstring>& ignoreExts) {
    getFilePaths(dir, pathList, ignoreExts, true);
  }

  void getFilePaths(const std::wstring & dir, std::vector<std::wstring>& pathList, const std::unordered_set<std::wstring>& ignoreExts, bool doRecursive) {
    if (dir.empty()) return;
    WIN32_FIND_DATA data;
    HANDLE fh = FindFirstFile((dir + L"/*").c_str(), &data);
    if (fh == INVALID_HANDLE_VALUE) return;
    do {
      std::wstring fileName(data.cFileName);
      if (fileName == L".." || fileName == L".") {
        continue;
      }
      std::wstring path = concatPath(dir, fileName);
      if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        if (doRecursive) getFilePathsRecursively(path, pathList, ignoreExts);
      } else {
        const auto ext = getLowerExt(path);
        if (ignoreExts.count(ext) == 0) {
          pathList.push_back(path);
        }
      }
    } while (FindNextFile(fh, &data));
    FindClose(fh);
  }

  void getDirs(const std::wstring & dir, std::vector<std::wstring>& dirList, bool doRecursive) {
    if (dir.empty()) return;
    WIN32_FIND_DATA data;
    HANDLE fh = FindFirstFile((dir + L"/*").c_str(), &data);
    if (fh == INVALID_HANDLE_VALUE) return;
    do {
      std::wstring fileName(data.cFileName);
      if (fileName == L".." || fileName == L".") {
        continue;
      }
      std::wstring path = concatPath(dir, fileName);
      if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        dirList.push_back(path);
        if (doRecursive) getDirsRecursively(path, dirList);
      }
    } while (FindNextFile(fh, &data));
    FindClose(fh);
  }

  void getDirsRecursively(const std::wstring & dir, std::vector<std::wstring>& dirList) {
    getDirs(dir, dirList, true);
  }

  std::vector<std::wstring> split(const std::wstring& s, wchar_t delimiter) {
    std::vector<std::wstring> r;
    std::wistringstream ss(s);
    std::wstring word;
    while (std::getline(ss, word, delimiter)) {
      r.push_back(word);
    }
    return r;
  }

  std::vector<std::wstring> split(const std::wstring& s, const std::wstring& delimiter) {
    std::vector<std::wstring> r;
    std::wstring::size_type pos = 0;
    while (pos != std::wstring::npos) {
      std::string::size_type p = s.find(delimiter, pos);
      if (p == std::string::npos) {
        r.push_back(s.substr(pos));
        break;
      } else {
        r.push_back(s.substr(pos, p - pos));
      }
      pos = p + delimiter.size();
    }
    return r;
  }

  bool isSpace(wchar_t c) {
    return c == L' ' || c == L'\t' || c == L'\n' || c == L'\r' || c == L'\f' || c == L'\v' || c == L'\b';
  }

  void trimSpace(std::wstring & s) {
    std::wstring::iterator it_left = std::find_if_not(s.begin(), s.end(), isSpace);
    s.erase(s.begin(), it_left);

    std::wstring::iterator it_right = std::find_if_not(s.rbegin(), s.rend(), isSpace).base();
    s.erase(it_right, s.end());
  }

  D3DXMATRIX rotScaleTrans(float x, float y, float z, float rx, float ry, float rz, float sx, float sy, float sz) {
    D3DXMATRIX mat;
    //回転
    D3DXMatrixRotationYawPitchRoll(&mat, D3DXToRadian(ry), D3DXToRadian(rx), D3DXToRadian(rz));

    //拡大縮小
    mat._11 *= sx;
    mat._12 *= sx;
    mat._13 *= sx;

    mat._21 *= sy;
    mat._22 *= sy;
    mat._23 *= sy;

    mat._31 *= sz;
    mat._32 *= sz;
    mat._33 *= sz;

    //移動
    mat._41 = x;
    mat._42 = y;
    mat._43 = z;
    return mat;
  }

  std::array<Vertex, 4> rectToVertices(D3DCOLOR color, int textureWidth, int textureHeight, const Rect<int>& rect) {
    std::array<Vertex, 4> vs;
    float ul = 1.0 * rect.left / textureWidth;
    float vt = 1.0 * rect.top / textureHeight;
    float ur = 1.0 * rect.right / textureWidth;
    float vb = 1.0 * rect.bottom / textureHeight;
    // 0.5 (for half pixel offset)
    float hw = (rect.right - rect.left) / 2.0f - 0.5f;
    float hh = (rect.bottom - rect.top) / 2.0f - 0.5f;
    vs[0] = { -hw, -hh, 0, color, ul, vt };
    vs[1] = { hw, -hh, 0, color, ur, vt };
    vs[2] = { -hw, hh, 0, color, ul, vb };
    vs[3] = { hw, hh, 0, color, ur, vb };
    return vs;
  }

  std::wstring canonicalPath(const std::wstring& path) {
   // unify path separator
    auto unified = std::regex_replace(path, std::wregex(L"((\\\\)|/)+"), L"/");
    // remove relative path
    std::wistringstream ss(unified);
    std::vector<std::wstring> trail; trail.reserve(32);
    std::wstring tmp;
    while (std::getline(ss, tmp, L'/')) {
      if (tmp == L".") {
        // ignore
      } else if (tmp == L"..") {
        if (trail.empty()) {
          trail.push_back(L"..");
        } else {
          trail.pop_back();
        }
      } else {
        trail.push_back(tmp);
      }
    }
    std::wstring ret; ret.reserve(128);
    for (int i = 0; i < trail.size(); i++) {
      if (i != 0) ret += L"/";
      ret += trail[i];
    }
    return ret;
  }

  std::wstring concatPath(const std::wstring& a, const std::wstring& b) {
    if (!a.empty() && (a.back() == L'/' || a.back() == L'\\')) return a + b;
    return a + L"/" + b;
  }

  std::wstring parentPath(const std::wstring& path) {
    auto found = path.find_last_of(L"/\\");
    auto ret = path.substr(0, found);
    if (ret == path) {
      return L".";
    }
    return ret;
  }

  std::wstring expandIncludePath(const std::wstring & includerPath, const std::wstring & includeePath) {
    if (includeePath.substr(0, 2) == L"./" ||
        includeePath.substr(0, 2) == L".\\" ||
        includeePath.substr(0, 3) == L"../" ||
        includeePath.substr(0, 3) == L"..\\") {
      return canonicalPath(concatPath(parentPath(includerPath), includeePath));
    }
    return includeePath;
  }

  std::string readResourceText(int resourceId) {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    if (HRSRC hResource = FindResource(hInstance, MAKEINTRESOURCE(resourceId), RT_HTML)) {
      if (DWORD resourceSize = SizeofResource(hInstance, hResource)) {
        if (HGLOBAL hMem = LoadResource(hInstance, hResource)) {
          if (LPVOID pMem = LockResource(hMem)) {
            return std::string((char*)pMem, resourceSize);
          }
          FreeResource(hMem);
        }
      }
    }
    return "";
  }
}