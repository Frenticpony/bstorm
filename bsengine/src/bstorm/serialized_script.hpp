#pragma once

#include <bstorm/cache_store.hpp>
#include <bstorm/script_info.hpp>
#include <bstorm/source_map.hpp>

#include <memory>
#include <vector>
#include <luajit/lua.hpp>

namespace bstorm
{
class SerializedScript
{
public:
    SerializedScript(const ScriptInfo& scriptInfo, const SourceMap& srcMap, lua_State* L);
    const std::string& GetScriptInfo() const { return scriptInfo_; }
    const std::string& GetSourceMap() const { return srcMap_; }
    const char* GetByteCode() { return byteCode_.data(); }
    const size_t GetByteCodeSize() { return byteCode_.size(); }
private:
    std::string scriptInfo_;
    std::string srcMap_;
    std::string byteCode_;
};
}