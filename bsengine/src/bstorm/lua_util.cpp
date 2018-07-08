#include <bstorm/lua_util.hpp>

static int AccumChunkSize(lua_State* L, const void* data, size_t dataSize, size_t* totalSize)
{
    *totalSize += dataSize;
    return 0;
}

static int ChunkWriter(lua_State* L, const void* data, size_t dataSize, std::string* dst)
{
    dst->append((const char*)data, dataSize);
    return 0;
}

namespace bstorm
{
void* GetPointerFromLuaRegistry(lua_State* L, const char* key)
{
    lua_pushstring(L, key);
    lua_rawget(L, LUA_REGISTRYINDEX);
    void* p = lua_touserdata(L, -1);
    return p;
}

void SetPointerToLuaRegistry(lua_State* L, const char* key, void* p)
{
    lua_pushstring(L, key);
    lua_pushlightuserdata(L, p);
    lua_rawset(L, LUA_REGISTRYINDEX);
}

void SerializeChunk(lua_State * L, std::string & byteCode)
{
    // 最初に全チャンク片の大きさの合計を求めてメモリを確保
    size_t totalSize = 0;
    lua_dump(L, (lua_Writer)AccumChunkSize, &totalSize);
    byteCode.clear();
    byteCode.reserve(totalSize);
    lua_dump(L, (lua_Writer)ChunkWriter, &byteCode);
}
}