#include <bstorm/script.hpp>

#include <bstorm/dnh_const.hpp>
#include <bstorm/const.hpp>
#include <bstorm/util.hpp>
#include <bstorm/logger.hpp>
#include <bstorm/dnh_value.hpp>
#include <bstorm/obj.hpp>
#include <bstorm/api.hpp>
#include <bstorm/source_map.hpp>
#include <bstorm/serialized_script.hpp>
#include <bstorm/package.hpp>
#include <bstorm/script_runtime.h>

#include <exception>
#include <cassert>

namespace bstorm
{
const std::unordered_set<std::wstring> ignoreScriptExts{ L".png", L".jpg", L".jpeg", L".bmp", L".gif", L".dds", L".hdr", L".dib", L".pfm", L".tif", L".tiff", L".ttf", L".otf", L".mqo", L".mp3", L".mp4", L".avi", L".ogg", L".wav", L".wave", L".def", L".dat", L".fx", L".exe" };

Script::Script(const std::wstring& path, ScriptType type, const std::wstring& version, int id, const std::shared_ptr<FileLoader>& fileLoader, const std::shared_ptr<Package>& package, const std::shared_ptr<SourcePos>& srcPos) :
    L_(luaL_newstate(), lua_close),
    path_(GetCanonicalPath(path)),
    type_(type),
    version_(version),
    id_(id),
    compileSrcPos_(srcPos),
    luaStateBusy_(false),
    autoDeleteObjectEnable_(false),
    package_(package),
    serializedScript_(std::make_shared<SerializedScript>(path_, type_, version_, fileLoader))
{
}

Script::~Script()
{
    if (auto package = package_.lock())
    {
        if (autoDeleteObjectEnable_)
        {
            for (auto objId : autoDeleteTargetObjIds_)
            {
                package->DeleteObject(objId);
            }
        }
    }
}

int Script::GetID() const
{
    return id_;
}

const std::wstring & Script::GetPath() const
{
    return path_;
}

ScriptType Script::GetType() const
{
    return type_;
}

const std::wstring & Script::GetVersion() const
{
    return version_;
}

void Script::Close()
{
    state_.isClosed = true;
}

bool Script::IsClosed() const
{
    return state_.isClosed;
}

void Script::RunBuiltInSub(const std::string &name)
{
    if (state_.isFailed) { return; }
    if (luaStateBusy_)
    {
        lua_getglobal(L_.get(), (DNH_VAR_PREFIX + name).c_str());
        if (lua_isfunction(L_.get(), -1))
        {
            CallLuaChunk(0);
        } else
        {
            lua_pop(L_.get(), 1);
        }
    } else
    {
        lua_getglobal(L_.get(), (std::string(DNH_RUNTIME_PREFIX) + "run").c_str());
        lua_getglobal(L_.get(), (DNH_VAR_PREFIX + name).c_str());
        if (lua_isfunction(L_.get(), -1))
        {
            CallLuaChunk(1);
        } else
        {
            lua_pop(L_.get(), 2);
        }
    }
}

void Script::CallLuaChunk(int argCnt)
{
    // API用の静的変数をセット
    if (auto package = package_.lock())
    {
        Package::Current = package.get();
    } else
    {
        lua_pop(L_.get(), 1);
        return;
    }

    bool tmp = luaStateBusy_;
    luaStateBusy_ = true;
    if (lua_pcall(L_.get(), argCnt, 0, 0) != 0)
    {
        luaStateBusy_ = false;
        std::string msg = lua_tostring(L_.get(), -1);
        lua_pop(L_.get(), 1);
        if (!err_)
        {
            try
            {
                throw Log(Log::Level::LV_ERROR)
                    .SetMessage("unexpected script runtime error occured, please send a bug report.")
                    .SetParam(Log::Param(Log::Param::Tag::TEXT, msg));
            } catch (...)
            {
                SaveError(std::current_exception());
            }
        }
        state_.isClosed = true;
        state_.isFailed = true;
        RethrowError();
    }
    luaStateBusy_ = tmp;
}

void Script::Load()
{
    if (state_.isLoaded || IsClosed()) { return; }

    // Lua標準API登録
    luaL_openlibs(L_.get());

    // 弾幕風標準API登録
    RegisterStandardAPI(L_.get(), type_, version_, nullptr);

    // ランタイム読み込み
    luaL_loadbuffer(L_.get(), (const char *)luaJIT_BC_script_runtime, luaJIT_BC_script_runtime_SIZE, DNH_RUNTIME_NAME);
    CallLuaChunk(0);

    // スクリプトをバインド
    SetScript(L_.get(), this);

    // toplevel
    luaL_loadbuffer(L_.get(), serializedScript_->GetByteCode(), serializedScript_->GetByteCodeSize(), "main");
    CallLuaChunk(0);

    // call @Loading
    RunBuiltInSub("Loading");
    Logger::WriteLog(std::move(
        Log(Log::Level::LV_INFO)
        .SetMessage("load script.")
        .SetParam(Log::Param(Log::Param(Log::Param::Tag::SCRIPT, path_)))
        .AddSourcePos(compileSrcPos_)));
    state_.isLoaded = true;
}

void Script::Start()
{
    if (state_.isStarted || IsClosed()) { return; }
    Load();
    state_.isStarted = true;
}

void Script::RunInitialize()
{
    if (state_.isInitialized || IsClosed()) { return; }
    Start();
    RunBuiltInSub("Initialize");
    state_.isInitialized = true;
}

void Script::RunMainLoop()
{
    if (IsClosed()) { return; }

    if (state_.isInitialized)
    {
        RunBuiltInSub("MainLoop");
    }
}

void Script::RunFinalize()
{
    if (state_.isFinalized || state_.isFailed) { return; }

    RunBuiltInSub("Finalize");

    state_.isClosed = true;
    state_.isFinalized = true;
}

void Script::NotifyEvent(int eventType)
{
    NotifyEvent(eventType, std::make_unique<DnhArray>(L""));
}

void Script::NotifyEvent(int eventType, const std::unique_ptr<DnhArray>& args)
{
    if (state_.isFinalized || state_.isFailed)
    {
        return;
    }

    if (state_.isStarted)
    {
        DnhReal((double)eventType).Push(L_.get());
        lua_setglobal(L_.get(), "script_event_type");
        args->Push(L_.get());
        lua_setglobal(L_.get(), "script_event_args");
        SetScriptResult(std::make_unique<DnhNil>());
        RunBuiltInSub("Event");
    }
}

bool Script::IsStgSceneScript() const
{
    return type_.IsStgSceneScript();
}

void Script::SetAutoDeleteObjectEnable(bool enable)
{
    autoDeleteObjectEnable_ = enable;
}

void Script::AddAutoDeleteTargetObjectId(int id)
{
    if (id != ID_INVALID)
    {
        autoDeleteTargetObjIds_.push_back(id);
    }
}

const std::unique_ptr<DnhValue>& Script::GetScriptResult() const
{
    if (auto package = package_.lock())
    {
        return package->GetScriptResult(GetID());
    }
    return DnhValue::Nil();
}

void Script::SetScriptResult(std::unique_ptr<DnhValue>&& value)
{
    if (auto package = package_.lock())
    {
        package->SetScriptResult(GetID(), std::move(value));
    }
}

void Script::SetScriptArgument(int idx, std::unique_ptr<DnhValue>&& value)
{
    scriptArgs_[idx] = std::move(value);
}

int Script::GetScriptArgumentCount() const
{
    return scriptArgs_.size();
}

const std::unique_ptr<DnhValue>& Script::GetScriptArgument(int idx)
{
    if (scriptArgs_.count(idx) == 0)
    {
        return DnhValue::Nil();
    }
    return scriptArgs_[idx];
}

std::shared_ptr<SourcePos> Script::GetSourcePos(int line) const
{
    return SourceMap(serializedScript_->GetSourceMap()).GetSourcePos(line);
}

void Script::SaveError(const std::exception_ptr& e)
{
    err_ = e;
}

void Script::RethrowError() const
{
    std::rethrow_exception(err_);
}

ScriptManager::ScriptManager(const std::shared_ptr<FileLoader>& fileLoader) :
    idGen_(0),
    fileLoader_(fileLoader)
{
}

ScriptManager::~ScriptManager() {}

std::shared_ptr<Script> ScriptManager::Compile(const std::wstring& path, ScriptType type, const std::wstring& version, const std::shared_ptr<Package>& package, const std::shared_ptr<SourcePos>& srcPos)
{
    auto script = std::make_shared<Script>(path, type, version, idGen_++, fileLoader_, package, srcPos);
    scriptMap_.emplace_hint(scriptMap_.end(), script->GetID(), script);
    return script;
}

std::shared_ptr<Script> ScriptManager::CompileInThread(const std::wstring & path, ScriptType type, const std::wstring & version, const std::shared_ptr<Package>& package, const std::shared_ptr<SourcePos>& srcPos)
{
    return Compile(path, type, version, package, srcPos);
}

void ScriptManager::RunMainLoopAllNonStgScript()
{
    for (auto& entry : scriptMap_)
    {
        if (!entry.second->IsStgSceneScript())
        {
            entry.second->RunMainLoop();
        }
    }
}

void ScriptManager::RunMainLoopAllStgScript()
{
    for (auto& entry : scriptMap_)
    {
        if (entry.second->IsStgSceneScript())
        {
            entry.second->RunMainLoop();
        }
    }
}

NullableSharedPtr<Script> ScriptManager::Get(int id) const
{
    auto it = scriptMap_.find(id);
    if (it != scriptMap_.end())
    {
        const auto& script = it->second;
        if (!script->IsClosed())
        {
            return it->second;
        }
    }
    return nullptr;
}

void ScriptManager::NotifyEventAll(int eventType)
{
    for (auto& entry : scriptMap_)
    {
        auto& script = entry.second;
        // NOTE: NotifyEventAllで送るとcloseされたスクリプトには届かない仕様
        if (!script->IsClosed())
        {
            script->NotifyEvent(eventType);
        }
    }
}

void ScriptManager::NotifyEventAll(int eventType, const std::unique_ptr<DnhArray>& args)
{
    for (auto& entry : scriptMap_)
    {
        auto& script = entry.second;
        // NOTE: NotifyEventAllで送るとcloseされたスクリプトには届かない仕様
        if (!script->IsClosed())
        {
            script->NotifyEvent(eventType, args);
        }
    }
}

void ScriptManager::RunFinalizeOnClosedScript()
{
    auto it = scriptMap_.begin();
    while (it != scriptMap_.end())
    {
        auto& script = it->second;
        if (script->IsClosed())
        {
            script->RunFinalize();
            it = scriptMap_.erase(it);
        } else ++it;
    }
}

void ScriptManager::RunFinalizeAll()
{
    for (auto& entry : scriptMap_)
    {
        entry.second->RunFinalize();
    }
    scriptMap_.clear();
}

void ScriptManager::CloseStgSceneScript()
{
    for (auto& entry : scriptMap_)
    {
        auto& script = entry.second;
        if (script->IsStgSceneScript())
        {
            script->Close();
        }
    }
}

const std::unique_ptr<DnhValue>& ScriptManager::GetScriptResult(int scriptId) const
{
    auto it = scriptResults_.find(scriptId);
    if (it != scriptResults_.end())
    {
        return it->second;
    }
    return DnhValue::Nil();
}

void ScriptManager::SetScriptResult(int scriptId, std::unique_ptr<DnhValue>&& value)
{
    scriptResults_[scriptId] = std::move(value);
}

void ScriptManager::ClearScriptResult()
{
    scriptResults_.clear();
}
}