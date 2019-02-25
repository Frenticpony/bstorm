#include <bstorm/serialized_script.hpp>

#include <bstorm/string_util.hpp>
#include <bstorm/file_util.hpp>
#include <bstorm/math_util.hpp>
#include <bstorm/lua_util.hpp>
#include <bstorm/logger.hpp>
#include <bstorm/api.hpp>
#include <bstorm/source_map.hpp>
#include <bstorm/parser.hpp>
#include <bstorm/semantics_checker.hpp>
#include <bstorm/code_analyzer.hpp>
#include <bstorm/code_generator.hpp>
#include <bstorm/script_entry_routine_names.hpp>

#include <luajit/lua.hpp>

namespace bstorm
{
SerializedScriptSignature::SerializedScriptSignature(const std::wstring & path, ScriptType type, std::wstring version, TimeStamp lastUpdateTime) :
    path(GetCanonicalPath(path)),
    type(type),
    version(version),
    lastUpdateTime(lastUpdateTime)
{
}

bool SerializedScriptSignature::operator==(const SerializedScriptSignature & params) const
{
    return path == params.path && type.value == params.type.value && version == params.version && lastUpdateTime == params.lastUpdateTime;
}

bool SerializedScriptSignature::operator!=(const SerializedScriptSignature & params) const
{
    return !(*this == params);
}

size_t SerializedScriptSignature::hashValue() const
{
    size_t h = 0;
    hash_combine(h, path);
    hash_combine(h, type.value);
    hash_combine(h, version);
    hash_combine(h, lastUpdateTime);
    return h;
}

SerializedScript::SerializedScript(const SerializedScriptSignature& signature, const std::shared_ptr<FileLoader>& fileLoader) :
    signature_(signature)
{
    std::unique_ptr<lua_State, decltype(&lua_close)> L(luaL_newstate(), lua_close);
    // ���쐬
    auto globalEnv = CreateInitRootEnv(signature.type, signature.version, nullptr);

    // �p�[�X
    ScriptInfo scriptInfo;
    std::shared_ptr<NodeBlock> program = ParseDnhScript(signature.path, globalEnv, true, &scriptInfo, fileLoader);

    // �ÓI�G���[����
    {
        SemanticsChecker checker;
        auto errors_ = checker.Check(*program);
        for (auto& err : errors_)
        {
            Logger::Write(err);
        }
        if (!errors_.empty())
        {
            throw Log(LogLevel::LV_ERROR)
                .Msg("Found " + std::to_string(errors_.size()) + " script error" + (errors_.size() > 1 ? "s." : "."))
                .Param(LogParam(LogParam::Tag::SCRIPT, signature.path));
        }
    }

    // �ÓI���
    CodeAnalyzer analyzer;
    analyzer.Analyze(*program);

    // �R�[�h����
    CodeGenerator::Option codeGenOption;
    codeGenOption.enableNilCheck = false;
    codeGenOption.deleteUnreachableDefinition = true;
    codeGenOption.deleteUnneededAssign = true;
    CodeGenerator codeGen(codeGenOption);
    codeGen.Generate(*program);

    // �R���p�C��
    {
        int hasCompileError = luaL_loadstring(L.get(), codeGen.GetCode().c_str());
        if (hasCompileError)
        {
            std::string msg = lua_tostring(L.get(), -1); lua_pop(L.get(), 1);
            if (msg.find("has more than 200 local variables") != std::string::npos)
            {
                auto ss = Split(ToUnicode(msg), L':');
                Log err = Log(LogLevel::LV_ERROR)
                    .Msg("Too many local variables declared in one function. (Limit 200.)");
                if (ss.size() >= 2)
                {
                    int line = _wtoi(ss[1].c_str());
                    err.AddSourcePos(codeGen.GetSourceMap().GetSourcePos(line));
                } else
                {
                    err.Param(LogParam(LogParam::Tag::SCRIPT, signature.path));
                }
                throw err;
            } else
            {
                throw Log(LogLevel::LV_ERROR)
                    .Msg("Unexpected compile error occured.  Please send a bug report. (" + msg + ")")
                    .Param(LogParam(LogParam::Tag::SCRIPT, signature.path));
            }
        }
    }
    scriptInfo.Serialize(scriptInfo_);
    codeGen.GetSourceMap().Serialize(srcMap_);
    SerializeChunk(L.get(), byteCode_);
#ifdef _DEBUG
    srcCode_ = codeGen.GetCode();
#endif
    for (auto&& name : SCRIPT_ENTRY_ROUTINE_NAMES)
    {
        if (auto def = globalEnv->FindDef(name))
        {
            builtInSubNameConversionMap_[name] = def->convertedName;
        } else
        {
            builtInSubNameConversionMap_[name] = name;
        }
    }
}
std::string SerializedScript::GetConvertedBuiltInSubName(const std::string & name) const
{
    auto it = builtInSubNameConversionMap_.find(name);
    if (it != builtInSubNameConversionMap_.end())
    {
        return it->second;
    }
    return "";
}

SerializedScriptStore::SerializedScriptStore(const std::shared_ptr<FileLoader>& fileLoader) :
    fileLoader_(fileLoader)
{
}

const std::shared_ptr<SerializedScript>& SerializedScriptStore::Load(const std::wstring & path, ScriptType type, const std::wstring & version)
{
    SerializedScriptSignature signature(path, type, version, GetFileLastUpdateTime(path));
    return cacheStore_.Load(signature, signature, fileLoader_);
}

SerializedScriptSignature SerializedScriptStore::LoadAsync(const std::wstring & path, ScriptType type, const std::wstring & version)
{
    SerializedScriptSignature signature(path, type, version, GetFileLastUpdateTime(path));
    cacheStore_.LoadAsync(signature, signature, fileLoader_);
    return signature;
}

NullableSharedPtr<SerializedScript> SerializedScriptStore::Get(const SerializedScriptSignature & signature) const
{
    try
    {
        return cacheStore_.Get(signature);
    } catch (...)
    {
        return nullptr;
    }
}

void SerializedScriptStore::RemoveCache(const SerializedScriptSignature& signature)
{
    cacheStore_.Remove(signature);
}
bool SerializedScriptStore::IsLoadCompleted(const SerializedScriptSignature & signature) const
{
    return cacheStore_.IsLoadCompleted(signature);
}
}