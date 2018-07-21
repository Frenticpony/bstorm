#include "reflex/dnh_lexer.hpp"
#include "bison/dnh.tab.hpp"
#include "reflex/user_def_data_lexer.hpp"
#include "bison/user_def_data.tab.hpp"
#include "reflex/mqo_lexer.hpp"
#include "bison/mqo.tab.hpp"

#include <bstorm/string_util.hpp>
#include <bstorm/file_loader.hpp>
#include <bstorm/env.hpp>
#include <bstorm/node.hpp>
#include <bstorm/shot_data.hpp>
#include <bstorm/item_data.hpp>
#include <bstorm/mqo.hpp>
#include <bstorm/parser.hpp>

#include <cstdio>

namespace bstorm
{
static ScriptInfo CreateScriptInfo(const std::wstring& filePath, const std::vector<NodeHeader>& headers)
{
    ScriptInfo info;
    info.path = GetCanonicalPath(filePath);
    info.id = info.path;
    info.version = SCRIPT_VERSION_PH3;
    for (const auto& header : headers)
    {
        if (header.params.empty()) continue;
        if (header.name == L"TouhouDanmakufu")
        {
            info.type = ScriptType::FromName(ToUTF8(header.params[0]));
        } else if (header.name == L"ScriptVersion")
        {
            info.version = header.params[0];
        } else if (header.name == L"ID")
        {
            info.id = header.params[0];
        } else if (header.name == L"Title")
        {
            info.title = header.params[0];
        } else if (header.name == L"Text")
        {
            info.text = header.params[0];
        } else if (header.name == L"Image")
        {
            info.imagePath = header.params[0];
        } else if (header.name == L"System")
        {
            info.systemPath = header.params[0];
        } else if (header.name == L"Background")
        {
            info.backgroundPath = header.params[0];
        } else if (header.name == L"BGM")
        {
            info.bgmPath = header.params[0];
        } else if (header.name == L"Player")
        {
            info.playerScriptPaths = header.params;
        } else if (header.name == L"ReplayName")
        {
            info.replayName = header.params[0];
        }
    }
    info.imagePath = ExpandIncludePath(info.path, info.imagePath);
    info.systemPath = ExpandIncludePath(info.path, info.systemPath);
    info.backgroundPath = ExpandIncludePath(info.path, info.backgroundPath);
    info.bgmPath = ExpandIncludePath(info.path, info.bgmPath);
    for (auto& playerPath : info.playerScriptPaths)
    {
        playerPath = ExpandIncludePath(info.path, playerPath);
    }
    return info;
}
std::shared_ptr<NodeBlock> ParseDnhScript(const std::wstring& filePath, const std::shared_ptr<Env>& globalEnv, bool expandInclude, ScriptInfo* scriptInfo, const std::shared_ptr<FileLoader>& loader)
{
    DnhLexer lexer;
    lexer.SetLoader(loader);
    lexer.PushInclude(filePath);
    DnhParseContext ctx(globalEnv, &lexer, expandInclude);
    DnhParser parser(&ctx);
    parser.parse();
    lexer.PopInclude();
    *scriptInfo = CreateScriptInfo(filePath, ctx.headers);
    return ctx.result;
}
ScriptInfo ScanDnhScriptInfo(const std::wstring & filePath, const std::shared_ptr<FileLoader>& loader)
{
    DnhLexer lexer;
    lexer.SetLoader(loader);
    lexer.PushInclude(filePath);
    DnhParseContext ctx(&lexer, false);
    DnhParser parser(&ctx);
    parser.parse();
    lexer.PopInclude();
    return CreateScriptInfo(filePath, ctx.headers);
}
std::shared_ptr<UserShotData> ParseUserShotData(const std::wstring& filePath, const std::shared_ptr<FileLoader>& loader)
{
    auto userShotData = std::make_shared<UserShotData>();
    UserDefDataLexer lexer;
    lexer.SetLoader(loader);
    lexer.SetInputSource(filePath);
    userShotData->path = GetCanonicalPath(filePath);
    userShotData->dataMap.clear();
    UserDefDataParseContext ctx(userShotData, &lexer);
    UserDefDataParser parser(&ctx);
    parser.parse();
    userShotData->imagePath = ExpandIncludePath(userShotData->path, userShotData->imagePath);
    return userShotData;
}
std::shared_ptr<UserItemData> ParseUserItemData(const std::wstring & filePath, const std::shared_ptr<FileLoader>& loader)
{
    auto userItemData = std::make_shared<UserItemData>();
    UserDefDataLexer lexer;
    lexer.SetLoader(loader);
    lexer.SetInputSource(filePath);
    userItemData->path = GetCanonicalPath(filePath);
    UserDefDataParseContext ctx(userItemData, &lexer);
    UserDefDataParser parser(&ctx);
    parser.parse();
    userItemData->imagePath = ExpandIncludePath(userItemData->path, userItemData->imagePath);
    return userItemData;
}
std::shared_ptr<Mqo> ParseMqo(const std::wstring & filePath, const std::shared_ptr<FileLoader>& loader)
{
    MqoLexer lexer;
    lexer.SetLoader(loader);
    lexer.SetInputSource(filePath);
    MqoParseContext ctx(&lexer);
    MqoParser parser(&ctx);
    parser.parse();
    ctx.mqo->path = GetCanonicalPath(filePath);
    return ctx.mqo;
}
}
