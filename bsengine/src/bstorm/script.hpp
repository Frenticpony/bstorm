#pragma once

#include <bstorm/non_copyable.hpp>
#include <bstorm/nullable_shared_ptr.hpp>
#include <bstorm/code_generator.hpp>

#include <string>
#include <memory>
#include <vector>
#include <utility>
#include <unordered_set>
#include <unordered_map>
#include <exception>
#include <mutex>
#include <luajit/lua.hpp>

namespace bstorm
{
class Log;
class Package;
class DnhValue;
class DnhArray;
class ScriptManager;
extern const std::unordered_set<std::wstring> ignoreScriptExts;

class Script : private NonCopyable
{
public:
    enum class State
    {
        SCRIPT_NOT_COMPILED, // コンパイル前 or コンパイル中
        SCRIPT_COMPILED, // コンパイル完了
        SCRIPT_COMPILE_FAILED, // コンパイル失敗
        SCRIPT_LOADING_COMPLETED, // トップレベル & @Loading完了
        SCRIPT_STARTED, // 実行開始 -- イベント受理開始
        SCRIPT_INITIALIZED, // @Initialize完了
        SCRIPT_CLOSED, // 正常終了
        SCRIPT_RUNTIME_FAILED, // 実行時エラー
        SCRIPT_FINALIZED // 正常終了→@Finalize完了
    };
    Script(const std::wstring& path, const std::wstring& type, const std::wstring& version, int id, Package* package, const std::shared_ptr<SourcePos>& compileSrcPos);
    ~Script();
    void Compile();
    void CompileInThread();
    void Close();
    bool IsClosed() const;
    int GetID() const;
    const std::wstring& GetPath() const;
    const std::wstring& GetType() const;
    const std::wstring& GetVersion() const;
    State GetState() const;
    std::shared_ptr<SourcePos> GetSourcePos(int line);
    void SaveError(const std::exception_ptr& e);
    void RethrowError() const;
    void Load();
    void Start();
    void RunInitialize();
    void RunMainLoop();
    void RunFinalize();
    void NotifyEvent(int eventType);
    void NotifyEvent(int eventType, const std::unique_ptr<DnhArray>& args);
    bool IsStgSceneScript() const;
    void SetAutoDeleteObjectEnable(bool enable);
    void AddAutoDeleteTargetObjectId(int id);
    const std::unique_ptr<DnhValue>& GetScriptResult() const;
    void SetScriptResult(std::unique_ptr<DnhValue>&& value);
    void SetScriptArgument(int idx, std::unique_ptr<DnhValue>&& value);
    int GetScriptArgumentCount() const;
    const std::unique_ptr<DnhValue>& GetScriptArgument(int idx);
private:
    void ExecCompile();
    void RunBuiltInSub(const std::string &name);
    void CallLuaChunk();
    lua_State* L_;
    const std::wstring path_;
    const std::wstring type_;
    const std::wstring version_;
    const int id_;
    const bool isStgSceneScript_;
    const std::shared_ptr<SourcePos> compileSrcPos_; // コンパイルを開始した場所
    State state_;
    SourceMap srcMap_;
    bool luaStateBusy_;
    std::exception_ptr err_;
    std::vector<int> autoDeleteTargetObjIds_;
    std::unordered_map<int, std::unique_ptr<DnhValue>> scriptArgs_;
    bool autoDeleteObjectEnable_;
    mutable std::recursive_mutex criticalSection_;
    std::thread compileThread_;
    Package* package_;
};

class ScriptManager
{
public:
    ScriptManager(Package* package);
    ~ScriptManager();
    std::shared_ptr<Script> Compile(const std::wstring& path, const std::wstring& type, const std::wstring& version, const std::shared_ptr<SourcePos>& srcPos);
    std::shared_ptr<Script> CompileInThread(const std::wstring& path, const std::wstring& type, const std::wstring& version, const std::shared_ptr<SourcePos>& srcPos);
    void RunAll(bool ignoreStgSceneScript);
    NullableSharedPtr<Script> Get(int id) const;
    void NotifyEventAll(int eventType);
    void NotifyEventAll(int eventType, const std::unique_ptr<DnhArray>& args);
    void CleanClosedScript();
    void FinalizeAll();
    void CloseStgSceneScript();
    const std::unique_ptr<DnhValue>& GetScriptResult(int scriptId) const;
    void SetScriptResult(int scriptId, std::unique_ptr<DnhValue>&& value);
    void ClearScriptResult();
private:
    Package * package_;
    int idGen_;
    std::list<std::shared_ptr<Script>> scriptList_;
    std::unordered_map <int, std::shared_ptr<Script>> scriptMap_;
    std::unordered_map<int, std::unique_ptr<DnhValue>> scriptResults_;
};
}