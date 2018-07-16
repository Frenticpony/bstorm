#pragma once

#include <bstorm/cache_store.hpp>
#include <bstorm/script_info.hpp>

#include <memory>
#include <vector>

namespace bstorm
{
class FileLoader;
class SerializedScript
{
public:
    // compile
    SerializedScript(const std::wstring& path, ScriptType type, const std::wstring& version, const std::shared_ptr<FileLoader>& fileLoader);
    const std::string& GetScriptInfo() const { return scriptInfo_; }
    const std::string& GetSourceMap() const { return srcMap_; }
    const char* GetByteCode() { return byteCode_.data(); }
    const size_t GetByteCodeSize() { return byteCode_.size(); }
private:
    ScriptType type_;
    std::wstring version_;
    std::string scriptInfo_;
    std::string srcMap_;
    std::string byteCode_;
};
}