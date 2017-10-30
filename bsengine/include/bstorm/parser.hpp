#pragma once

#include <memory>
#include <string>

namespace bstorm {
  class FileLoader;
  class UserShotData;
  class UserItemData;
  class ScriptInfo;
  struct Env;
  struct NodeBlock;
  struct Mqo;
  std::shared_ptr<NodeBlock> parseDnhScript(const std::wstring& filePath, const std::shared_ptr<Env>& globalEnv, bool expandInclude, const std::shared_ptr<FileLoader>& loader);
  ScriptInfo scanDnhScriptInfo(const std::wstring& filePath, const std::shared_ptr<FileLoader>& loader);
  std::shared_ptr<UserShotData> parseUserShotData(const std::wstring& filePath, const std::shared_ptr<FileLoader>& loader);
  std::shared_ptr<UserItemData> parseUserItemData(const std::wstring& filePath, const std::shared_ptr<FileLoader>& loader);
  std::shared_ptr<Mqo> parseMqo(const std::wstring& filePath, const std::shared_ptr<FileLoader>& loader);
}