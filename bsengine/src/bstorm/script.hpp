#pragma once

#include <bstorm/non_copyable.hpp>
#include <bstorm/nullable_shared_ptr.hpp>
#include <bstorm/source_map.hpp>

#include <string>
#include <memory>
#include <deque>
#include <utility>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <exception>
#include <luajit/lua.hpp>

namespace bstorm
{
class Package;
class FileLoader;
class DnhValue;
class DnhArray;
extern const std::unordered_set<std::wstring> ignoreScriptExts;

class Script : private NonCopyable
{
public:
    Script(const std::wstring& path, const std::wstring& type, const std::wstring& version, int id, const std::shared_ptr<FileLoader>& fileLoader, const std::shared_ptr<Package>& package, const std::shared_ptr<SourcePos>& compileSrcPos);
    ~Script();
    void Close();
    bool IsClosed() const;
    int GetID() const;
    const std::wstring& GetPath() const;
    const std::wstring& GetType() const;
    const std::wstring& GetVersion() const;
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
    void RunBuiltInSub(const std::string &name);
    void CallLuaChunk(int argCnt);
    lua_State* L_;
    const std::wstring path_;
    const std::wstring type_;
    const std::wstring version_;
    const int id_;
    const bool isStgSceneScript_;
    const std::shared_ptr<SourcePos> compileSrcPos_; // コンパイルを開始した場所
    SourceMap srcMap_;
    bool luaStateBusy_;
    std::exception_ptr err_;
    std::deque<int> autoDeleteTargetObjIds_;
    std::unordered_map<int, std::unique_ptr<DnhValue>> scriptArgs_;
    bool autoDeleteObjectEnable_;
    struct State
    {
        bool isLoaded = false; // after toplevel and @Loading
        bool isStarted = false; // start event handling
        bool isInitialized = false; // after @Initialize
        bool isClosed = false;
        bool isFinalized = false; // after @Finalize
        bool isFailed = false; // error
    } state_;
    std::weak_ptr<Package> package_;
};

class ScriptManager
{
public:
    ScriptManager(const std::shared_ptr<FileLoader>& fileLoader);
    ~ScriptManager();
    std::shared_ptr<Script> Compile(const std::wstring& path, const std::wstring& type, const std::wstring& version, const std::shared_ptr<Package>& package, const std::shared_ptr<SourcePos>& srcPos);
    std::shared_ptr<Script> CompileInThread(const std::wstring& path, const std::wstring& type, const std::wstring& version, const std::shared_ptr<Package>& package, const std::shared_ptr<SourcePos>& srcPos);
    void RunMainLoopAllNonStgScript();
    void RunMainLoopAllStgScript();
    NullableSharedPtr<Script> Get(int id) const;
    void NotifyEventAll(int eventType);
    void NotifyEventAll(int eventType, const std::unique_ptr<DnhArray>& args);
    void RunFinalizeOnClosedScript();
    void RunFinalizeAll();
    void CloseStgSceneScript();
    const std::unique_ptr<DnhValue>& GetScriptResult(int scriptId) const;
    void SetScriptResult(int scriptId, std::unique_ptr<DnhValue>&& value);
    void ClearScriptResult();
private:
    int idGen_;
    std::map<int, std::shared_ptr<Script>> scriptMap_; // IDが若い順に走査される
    std::unordered_map<int, std::unique_ptr<DnhValue>> scriptResults_;
    const std::shared_ptr<FileLoader> fileLoader_;
};
}