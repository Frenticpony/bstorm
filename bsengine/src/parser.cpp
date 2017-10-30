#include <cstdio>

#include "reflex/dnh_lexer.hpp"
#include "bison/dnh.tab.hpp"
#include "reflex/user_def_data_lexer.hpp"
#include "bison/user_def_data.tab.hpp"
#include "reflex/mqo_lexer.hpp"
#include "bison/mqo.tab.hpp"

#include <bstorm/util.hpp>
#include <bstorm/file_loader.hpp>
#include <bstorm/script_info.hpp>
#include <bstorm/env.hpp>
#include <bstorm/node.hpp>
#include <bstorm/shot_data.hpp>
#include <bstorm/item_data.hpp>
#include <bstorm/mqo.hpp>
#include <bstorm/parser.hpp>

namespace bstorm {
  std::shared_ptr<NodeBlock> parseDnhScript(const std::wstring& filePath, const std::shared_ptr<Env>& globalEnv, bool expandInclude, const std::shared_ptr<FileLoader>& loader) {
    DnhLexer lexer;
    lexer.setLoader(loader);
    lexer.pushInclude(filePath);
    DnhParseContext ctx(globalEnv, &lexer, expandInclude);
    DnhParser parser(&ctx);
    parser.parse();
    lexer.popInclude();
    return ctx.result;
  }
  ScriptInfo scanDnhScriptInfo(const std::wstring & filePath, const std::shared_ptr<FileLoader>& loader) {
    DnhLexer lexer;
    lexer.setLoader(loader);
    lexer.pushInclude(filePath);
    DnhParseContext ctx(&lexer, false);
    DnhParser parser(&ctx);
    parser.parse();
    lexer.popInclude();
    ScriptInfo info;
    info.path = canonicalPath(filePath);
    info.id = info.path;
    info.type = SCRIPT_TYPE_UNKNOWN;
    info.version = SCRIPT_VERSION_PH3;
    for (const auto& header : ctx.headers) {
      if (header.params.empty()) continue;
      if (header.name == L"TouhouDanmakufu") {
        info.type = header.params[0];
      } else if (header.name == L"std::wstring") {
        info.version = header.params[0];
      } else if (header.name == L"ID") {
        info.id = header.params[0];
      } else if (header.name == L"Title") {
        info.title = header.params[0];
      } else if (header.name == L"Text") {
        info.text = header.params[0];
      } else if (header.name == L"Image") {
        info.imagePath = header.params[0];
      } else if (header.name == L"System") {
        info.systemPath = header.params[0];
      } else if (header.name == L"Background") {
        info.backgroundPath = header.params[0];
      } else if (header.name == L"BGM") {
        info.bgmPath = header.params[0];
      } else if (header.name == L"Player") {
        info.playerScripts = header.params;
      } else if (header.name == L"ReplayName") {
        info.replayName = header.params[0];
      }
    }
    info.imagePath = expandIncludePath(info.path, info.imagePath);
    info.systemPath = expandIncludePath(info.path, info.systemPath);
    info.backgroundPath = expandIncludePath(info.path, info.backgroundPath);
    info.bgmPath = expandIncludePath(info.path, info.bgmPath);
    for (auto& playerPath : info.playerScripts) {
      playerPath = expandIncludePath(info.path, playerPath);
    }
    return info;
  }
  std::shared_ptr<UserShotData> parseUserShotData(const std::wstring& filePath, const std::shared_ptr<FileLoader>& loader) {
    auto userShotData = std::make_shared<UserShotData>();
    UserDefDataLexer lexer;
    lexer.setLoader(loader);
    lexer.setInputSource(filePath);
    userShotData->path = canonicalPath(filePath);
    userShotData->dataMap.clear();
    UserDefDataParseContext ctx(userShotData, &lexer);
    UserDefDataParser parser(&ctx);
    parser.parse();
    userShotData->imagePath = expandIncludePath(userShotData->path, userShotData->imagePath);
    return userShotData;
  }
  std::shared_ptr<UserItemData> parseUserItemData(const std::wstring & filePath, const std::shared_ptr<FileLoader>& loader) {
    auto userItemData = std::make_shared<UserItemData>();
    UserDefDataLexer lexer;
    lexer.setLoader(loader);
    lexer.setInputSource(filePath);
    userItemData->path = canonicalPath(filePath);
    UserDefDataParseContext ctx(userItemData, &lexer);
    UserDefDataParser parser(&ctx);
    parser.parse();
    userItemData->imagePath = expandIncludePath(userItemData->path, userItemData->imagePath);
    return userItemData;
  }
  std::shared_ptr<Mqo> parseMqo(const std::wstring & filePath, const std::shared_ptr<FileLoader>& loader) {
    MqoLexer lexer;
    lexer.setLoader(loader);
    lexer.setInputSource(filePath);
    MqoParseContext ctx(&lexer);
    MqoParser parser(&ctx);
    parser.parse();
    ctx.mqo->path = canonicalPath(filePath);
    return ctx.mqo;
  }
}
