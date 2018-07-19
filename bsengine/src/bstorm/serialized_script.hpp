#pragma once

#include <bstorm/cache_store.hpp>
#include <bstorm/script_info.hpp>
#include <bstorm/time_stamp.hpp>
#include <bstorm/nullable_shared_ptr.hpp>

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

namespace bstorm
{
class SerializedScriptSignature
{
public:
    SerializedScriptSignature(const std::wstring& path, ScriptType type, std::wstring version, TimeStamp lastUpdateTime);
    bool operator==(const SerializedScriptSignature& params) const;
    bool operator!=(const SerializedScriptSignature& params) const;
    size_t hashValue() const;
    std::wstring path;
    ScriptType type;
    std::wstring version;
    TimeStamp lastUpdateTime;
};
}

namespace std
{
template<>
struct hash<bstorm::SerializedScriptSignature>
{
    size_t operator()(const bstorm::SerializedScriptSignature& params) const
    {
        return params.hashValue();
    }
};
}

namespace bstorm
{
class FileLoader;
class SerializedScript
{
public:
    SerializedScript(const SerializedScriptSignature& signature, const std::shared_ptr<FileLoader>& fileLoader);
    const std::string& GetScriptInfo() const { return scriptInfo_; }
    const std::string& GetSourceMap() const { return srcMap_; }
    const char* GetByteCode() { return byteCode_.data(); }
    const size_t GetByteCodeSize() { return byteCode_.size(); }
    std::string GetConvertedBuiltInSubName(const std::string& name) const;
    const SerializedScriptSignature& GetSignature() const { return signature_; }
private:
    const SerializedScriptSignature signature_;
    std::string scriptInfo_;
    std::string srcMap_;
    std::string byteCode_;
    std::unordered_map<std::string, std::string> builtInSubNameConversionMap_;
};

class SerializedScriptStore
{
public:
    SerializedScriptStore(const std::shared_ptr<FileLoader>& fileLoader);
    const std::shared_ptr<SerializedScript>& Load(const std::wstring& path, ScriptType type, const std::wstring& version);
    SerializedScriptSignature LoadAsync(const std::wstring& path, ScriptType type, const std::wstring& version);
    NullableSharedPtr<SerializedScript> Get(const SerializedScriptSignature& signature) const;
    void RemoveCache(const SerializedScriptSignature& signature);
    bool IsLoadCompleted(const SerializedScriptSignature& signature) const;
    template <class Fn>
    void ForEach(Fn func) { cacheStore_.ForEach(func); }
private:
    std::shared_ptr<FileLoader> fileLoader_;
    CacheStore<SerializedScriptSignature, SerializedScript> cacheStore_;
};
}
