#include <bstorm/serialized_script.hpp>

#include <bstorm/lua_util.hpp>

namespace bstorm
{
SerializedScript::SerializedScript(const ScriptInfo & scriptInfo, const SourceMap & srcMap, lua_State * L)
{
    scriptInfo.Serialize(scriptInfo_);
    srcMap.Serialize(srcMap_);
    SerializeChunk(L, byteCode_);
}
}