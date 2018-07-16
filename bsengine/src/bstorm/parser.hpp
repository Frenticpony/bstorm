#pragma once

#include <bstorm/script_info.hpp>

#include <memory>
#include <string>

namespace bstorm
{
class ScriptInfo;
class FileLoader;
class UserShotData;
class UserItemData;
class Env;
struct NodeBlock;
struct Mqo;
std::shared_ptr<NodeBlock> ParseDnhScript(const std::wstring& filePath, const std::shared_ptr<Env>& globalEnv, bool expandInclude, ScriptInfo* scriptInfo, const std::shared_ptr<FileLoader>& loader);
ScriptInfo ScanDnhScriptInfo(const std::wstring& filePath, const std::shared_ptr<FileLoader>& loader);
std::shared_ptr<UserShotData> ParseUserShotData(const std::wstring& filePath, const std::shared_ptr<FileLoader>& loader);
std::shared_ptr<UserItemData> ParseUserItemData(const std::wstring& filePath, const std::shared_ptr<FileLoader>& loader);
std::shared_ptr<Mqo> ParseMqo(const std::wstring& filePath, const std::shared_ptr<FileLoader>& loader);
}