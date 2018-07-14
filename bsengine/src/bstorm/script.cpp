#include <bstorm/script.hpp>

#include <bstorm/dnh_const.hpp>
#include <bstorm/const.hpp>
#include <bstorm/util.hpp>
#include <bstorm/script_info.hpp>
#include <bstorm/parser.hpp>
#include <bstorm/obj.hpp>
#include <bstorm/dnh_value.hpp>
#include <bstorm/package.hpp>
#include <bstorm/semantics_checker.hpp>
#include <bstorm/code_generator.hpp>
#include <bstorm/api.hpp>
#include <bstorm/logger.hpp>
#include <bstorm/package.hpp>
#include <bstorm/time_point.hpp>
#include <bstorm/script_runtime.h>

#include <exception>
#include <cassert>

namespace bstorm
{
const std::unordered_set<std::wstring> ignoreScriptExts{ L".png", L".jpg", L".jpeg", L".bmp", L".gif", L".dds", L".hdr", L".dib", L".pfm", L".tif", L".tiff", L".ttf", L".otf", L".mqo", L".mp3", L".mp4", L".avi", L".ogg", L".wav", L".wave", L".def", L".dat", L".fx", L".exe" };

Script::Script(const std::wstring& p, const std::wstring& type, const std::wstring& version, int id, const std::shared_ptr<FileLoader>& fileLoader, const std::shared_ptr<Package>& package, const std::shared_ptr<SourcePos>& srcPos) :
    L_(NULL),
    path_(GetCanonicalPath(p)),
    type_(type),
    version_(version),
    id_(id),
    compileSrcPos_(srcPos),
    luaStateBusy_(false),
    isStgSceneScript_(bstorm::IsStgSceneScript(type)),
    autoDeleteObjectEnable_(false),
    package_(package)
{
    try
    {
        L_ = lua_open();
        // Lua標準API登録
        luaL_openlibs(L_);

        // 弾幕風標準API登録
        auto globalEnv = std::make_shared<Env>();
        RegisterStandardAPI(L_, type_, version_, globalEnv->table);

        // スクリプトをバインド
        SetScript(L_, this);

        // パース
        std::shared_ptr<NodeBlock> program;
        {
            Logger::WriteLog(std::move(
                Log(Log::Level::LV_DETAIL)
                .SetMessage("parse start...")
                .SetParam(Log::Param(Log::Param::Tag::SCRIPT, path_))
                .AddSourcePos(compileSrcPos_)));
            TimePoint tp;
            program = ParseDnhScript(path_, globalEnv, true, fileLoader);
            Logger::WriteLog(std::move(
                Log(Log::Level::LV_DETAIL)
                .SetMessage("...parse complete " + std::to_string(tp.GetElapsedMilliSec()) + " [ms].")
                .SetParam(Log::Param(Log::Param::Tag::SCRIPT, path_))
                .AddSourcePos(compileSrcPos_)));
        }

        // 静的エラー検査
        {
            SemanticsChecker checker;
            auto errors_ = checker.Check(*program);
            for (auto& err : errors_)
            {
                Logger::WriteLog(err);
            }
            if (!errors_.empty())
            {
                throw Log(Log::Level::LV_ERROR)
                    .SetMessage("found " + std::to_string(errors_.size()) + " script error" + (errors_.size() > 1 ? "s." : "."))
                    .SetParam(Log::Param(Log::Param::Tag::SCRIPT, path_));
            }
        }

        // 変換
        CodeGenerator codeGen;
        {
            Logger::WriteLog(std::move(
                Log(Log::Level::LV_DETAIL)
                .SetMessage("codegen start...")
                .SetParam(Log::Param(Log::Param::Tag::SCRIPT, path_))
                .AddSourcePos(compileSrcPos_)));
            TimePoint tp;
            codeGen.Generate(true, *program);
            codeGen.GetSourceMap().Serialize(srcMap_);
            Logger::WriteLog(std::move(
                Log(Log::Level::LV_DETAIL)
                .SetMessage("...codegen complete " + std::to_string(tp.GetElapsedMilliSec()) + " [ms].")
                .SetParam(Log::Param(Log::Param::Tag::SCRIPT, path_))
                .AddSourcePos(compileSrcPos_)));
        }

        // ランタイム読み込み
        luaL_loadbuffer(L_, (const char *)luaJIT_BC_script_runtime, luaJIT_BC_script_runtime_SIZE, DNH_RUNTIME_NAME);
        CallLuaChunk(0);

        // コンパイル
        {
            Logger::WriteLog(std::move(
                Log(Log::Level::LV_DETAIL)
                .SetMessage("compile start...")
                .SetParam(Log::Param(Log::Param::Tag::SCRIPT, path_))
                .AddSourcePos(compileSrcPos_)));
            TimePoint tp;
            int hasCompileError = luaL_loadstring(L_, codeGen.GetCode().c_str());
            if (hasCompileError)
            {
                std::string msg = lua_tostring(L_, -1); lua_pop(L_, 1);
                if (msg.find("has more than 200 local variables") != std::string::npos)
                {
                    auto ss = Split(ToUnicode(msg), L':');
                    Log err = Log(Log::Level::LV_ERROR)
                        .SetMessage("too many variable used in one function.");
                    if (ss.size() >= 2)
                    {
                        int line = _wtoi(ss[1].c_str());
                        err.AddSourcePos(codeGen.GetSourceMap().GetSourcePos(line));
                    } else
                    {
                        err.SetParam(Log::Param(Log::Param::Tag::SCRIPT, path_));
                    }
                    throw err;
                } else
                {
                    throw Log(Log::Level::LV_ERROR)
                        .SetMessage("unexpected compile error occured, please send a bug report. (" + msg + ")")
                        .SetParam(Log::Param(Log::Param::Tag::SCRIPT, path_));
                }
            }
            Logger::WriteLog(std::move(
                Log(Log::Level::LV_DETAIL)
                .SetMessage("...compile complete " + std::to_string(tp.GetElapsedMilliSec()) + " [ms].")
                .SetParam(Log::Param(Log::Param::Tag::SCRIPT, path_))
                .AddSourcePos(compileSrcPos_)));
        }
    } catch (...)
    {
        if (L_)
        {
            lua_close(L_);
            L_ = NULL;
        }
        throw;
    }
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
    lua_close(L_);
}

int Script::GetID() const
{
    return id_;
}

const std::wstring & Script::GetPath() const
{
    return path_;
}

const std::wstring& Script::GetType() const
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
        lua_getglobal(L_, (DNH_VAR_PREFIX + name).c_str());
        if (lua_isfunction(L_, -1))
        {
            CallLuaChunk(0);
        } else
        {
            lua_pop(L_, 1);
        }
    } else
    {
        lua_getglobal(L_, (std::string(DNH_RUNTIME_PREFIX) + "run").c_str());
        lua_getglobal(L_, (DNH_VAR_PREFIX + name).c_str());
        if (lua_isfunction(L_, -1))
        {
            CallLuaChunk(1);
        } else
        {
            lua_pop(L_, 2);
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
        lua_pop(L_, 1);
        return;
    }

    bool tmp = luaStateBusy_;
    luaStateBusy_ = true;
    if (lua_pcall(L_, argCnt, 0, 0) != 0)
    {
        luaStateBusy_ = false;
        std::string msg = lua_tostring(L_, -1);
        lua_pop(L_, 1);
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

    CallLuaChunk(0); // exec toplevel chunk
    RunBuiltInSub("Loading");
    Logger::WriteLog(std::move(
        Log(Log::Level::LV_INFO)
        .SetMessage("load script.")
        .SetParam(Log::Param(Log::Param(Log::Param::Tag::SCRIPT, GetCanonicalPath(path_))))
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
        DnhReal((double)eventType).Push(L_);
        lua_setglobal(L_, "script_event_type");
        args->Push(L_);
        lua_setglobal(L_, "script_event_args");
        SetScriptResult(std::make_unique<DnhNil>());
        RunBuiltInSub("Event");
    }
}

bool Script::IsStgSceneScript() const
{
    return isStgSceneScript_;
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

std::shared_ptr<SourcePos> Script::GetSourcePos(int line)
{
    return SourceMap(srcMap_).GetSourcePos(line);
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

std::shared_ptr<Script> ScriptManager::Compile(const std::wstring& path, const std::wstring& type, const std::wstring& version, const std::shared_ptr<Package>& package, const std::shared_ptr<SourcePos>& srcPos)
{
    auto script = std::make_shared<Script>(path, type, version, idGen_++, fileLoader_, package, srcPos);
    scriptMap_.emplace_hint(scriptMap_.end(), script->GetID(), script);
    return script;
}

std::shared_ptr<Script> ScriptManager::CompileInThread(const std::wstring & path, const std::wstring & type, const std::wstring & version, const std::shared_ptr<Package>& package, const std::shared_ptr<SourcePos>& srcPos)
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