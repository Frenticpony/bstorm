#include <fstream>
#include <cstdio>
#include <windows.h>
#include <bstorm/config.hpp>

#include "../../version.hpp"

namespace bstorm {
  conf::BstormConfig loadBstormConfig(const std::string & path, bool isBinaryFormat, int defaultConfigResourceId) noexcept(false) {
    conf::BstormConfig config;
    try {
      if (FILE* fp = fopen(path.c_str(), "r")) {
        fclose(fp);
        if (isBinaryFormat) {
          std::ifstream configFileIn(path, std::ios::binary);
          config = conf::json::from_msgpack(configFileIn);
          configFileIn.close();
        } else {
          std::ifstream configFileIn(path);
          conf::json j;
          configFileIn >> j;
          config = j;
          configFileIn.close();
        }
      } else {
        std::string defaultConfigText = readResourceText(defaultConfigResourceId);
        config = conf::json::parse(defaultConfigText);
      }
    } catch (...) {
      throw std::runtime_error("can't load uncompatible config file: " + path + " (hint: please delete it, and try again.)");
    }
    return config;
  }

  bool saveBstormConfig(const std::string & path, bool isBinaryFormat, conf::BstormConfig config) {
    config.generatedBy = BSTORM_VERSION;
    if (isBinaryFormat) {
      std::ofstream configFileOut(path, std::ios::binary);
      if (!configFileOut.good()) return false;
      auto data = conf::json::to_msgpack(config);
      configFileOut.write((char*)data.data(), data.size());
    } else {
      std::ofstream configFileOut(path);
      if (!configFileOut.good()) return false;
      configFileOut << std::setw(4) << conf::json(config) << std::endl;
    }
    return true;
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
