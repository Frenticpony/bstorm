#pragma once

#include <string>
#include <memory>
#include <vector>
#include <utility>
#include <unordered_set>
#include <unordered_map>
#include <exception>
#include <atomic>
#include <mutex>
#include <luajit/lua.hpp>

#include <bstorm/non_copyable.hpp>
#include <bstorm/code_generator.hpp>

namespace bstorm {
  class Log;
  class Engine;
  class DnhValue;
  class DnhArray;
  class ScriptManager;
  extern const std::unordered_set<std::wstring> ignoreScriptExts;

  class Script : private NonCopyable {
  public:
    enum class State {
      SCRIPT_NOT_COMPILED, // コンパイル前 or コンパイル中
      SCRIPT_COMPILED, // コンパイル完了
      SCRIPT_COMPILE_FAILED, // コンパイル失敗
      SCRIPT_LOADING_COMPLETED, // トップレベル & @Loading完了
      SCRIPT_STARTED, // 実行開始 -- イベント受理開始
      SCRIPT_INITIALIZED, // @Initialize完了
      SCRIPT_CLOSED // 終了(実行時エラー含む)
    };
    Script(const std::wstring& path, const std::wstring& type, const std::wstring& version, int id, Engine* engine, const std::shared_ptr<SourcePos>& compileSrcPos);
    ~Script();
    void compile();
    void compileInThread();
    void close();
    bool isClosed() const;
    int getID() const;
    const std::wstring& getPath() const;
    const std::wstring& getType() const;
    const std::wstring& getVersion() const;
    State getState() const;
    std::shared_ptr<SourcePos> getSourcePos(int line);
    void saveError(const std::exception_ptr& e);
    void rethrowError() const;
    void load();
    void start();
    void runInitialize();
    void runMainLoop();
    void notifyEvent(int eventType);
    void notifyEvent(int eventType, const std::unique_ptr<DnhArray>& args);
    bool isStgSceneScript() const;
    void setAutoDeleteObjectEnable(bool enable);
    void addAutoDeleteTargetObjectId(int id);
    std::unique_ptr<DnhValue> getScriptResult() const;
    void setScriptResult(std::unique_ptr<DnhValue>&& value);
    void setScriptArgument(int idx, std::unique_ptr<DnhValue>&& value);
    int getScriptArgumentount() const;
    std::unique_ptr<DnhValue> getScriptArgument(int idx);
  private:
    void execCompile();
    void runBuiltInSub(const std::string &name);
    void callLuaChunk();
    lua_State* L;
    const std::wstring path;
    const std::wstring type;
    const std::wstring version;
    const int id;
    const bool stgSceneScript;
    const std::shared_ptr<SourcePos> compileSrcPos; // コンパイルを開始した場所
    std::atomic<State> state;
    SourceMap srcMap;
    bool luaStateBusy;
    std::exception_ptr err;
    std::vector<int> autoDeleteTargetObjIds;
    std::unordered_map<int, std::unique_ptr<DnhValue>> scriptArgs;
    bool autoDeleteObjectEnable;
    mutable std::recursive_mutex compileSection;
    Engine* engine;
  };

  class ScriptManager {
  public:
    ScriptManager(Engine* engine);
    ~ScriptManager();
    std::shared_ptr<Script> compile(const std::wstring& path, const std::wstring& type, const std::wstring& version, const std::shared_ptr<SourcePos>& srcPos);
    std::shared_ptr<Script> compileInThread(const std::wstring& path, const std::wstring& type, const std::wstring& version, const std::shared_ptr<SourcePos>& srcPos);
    void runAll(bool ignoreStgSceneScript);
    std::shared_ptr<Script> get(int id) const;
    void notifyEventAll(int eventType);
    void notifyEventAll(int eventType, const std::unique_ptr<DnhArray>& args);
    void cleanClosedScript();
    void closeStgSceneScript();
    std::unique_ptr<DnhValue> getScriptResult(int scriptId) const;
    void setScriptResult(int scriptId, std::unique_ptr<DnhValue>&& value);
    void clearScriptResult();
  private:
    Engine* engine;
    int idGen;
    std::list<std::shared_ptr<Script>> scriptList;
    std::unordered_map <int, std::shared_ptr<Script>> scriptMap;
    std::unordered_map<int, std::unique_ptr<DnhValue>> scriptResults;
  };
}