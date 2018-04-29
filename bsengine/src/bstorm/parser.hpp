#pragma once

#include <memory>
#include <string>

namespace bstorm
{
class FileLoader;
class UserShotData;
class UserItemData;
class ScriptInfo;
struct Env;
struct NodeBlock;
struct Mqo;
std::shared_ptr<NodeBlock> ParseDnhScript(const std::wstring& filePath, const std::shared_ptr<Env>& globalEnv, bool expandInclude, const std::shared_ptr<FileLoader>& loader);
ScriptInfo ScanDnhScriptInfo(const std::wstring& filePath, const std::shared_ptr<FileLoader>& loader);
std::shared_ptr<UserShotData> ParseUserShotData(const std::wstring& filePath, const std::shared_ptr<FileLoader>& loader);
std::shared_ptr<UserItemData> ParseUserItemData(const std::wstring& filePath, const std::shared_ptr<FileLoader>& loader);
std::shared_ptr<Mqo> ParseMqo(const std::wstring& filePath, const std::shared_ptr<FileLoader>& loader);
}