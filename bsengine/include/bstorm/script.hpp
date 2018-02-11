#pragma once

#include <string>
#include <memory>
#include <vector>
#include <utility>
#include <unordered_set>
#include <unordered_map>
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
      SCRIPT_NOT_COMPILED,
      SCRIPT_COMPILED,
      SCRIPT_LOADING_COMPLETE,
      SCRIPT_RUNNING,
      SCRIPT_TERMINATED,
      SCRIPT_CLOSED
    };
    Script(const std::wstring& path, const std::wstring& type, const std::wstring& version, int id, Engine* engine, const std::shared_ptr<SourcePos>& srcPos);
    ~Script();
    int getID() const;
    std::wstring getType() const;
    State getState() const;
    void close();
    bool isClosed() const;
    std::shared_ptr<SourcePos> getSourcePos(int line);
    void saveErrLog(const std::shared_ptr<Log>& log);
    void runLoading();
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
    void runBuiltInSub(const std::string &name);
    void callLuaChunk();
    lua_State* L;
    std::wstring path;
    std::wstring type;
    int id;
    State state;
    SourceMap srcMap;
    bool stgSceneScript;
    bool luaStateBusy;
    std::shared_ptr<Log> errLog;
    std::vector<int> autoDeleteTargetObjIds;
    std::unordered_map<int, std::unique_ptr<DnhValue>> scriptArgs;
    bool autoDeleteObjectEnable;
    Engine* engine;
    friend ScriptManager;
  };

  class ScriptManager {
  public:
    ScriptManager(Engine* engine);
    ~ScriptManager();
    std::shared_ptr<Script> ScriptManager::newScript(const std::wstring& path, const std::wstring& type, const std::wstring& version, const std::shared_ptr<SourcePos>& srcPos);
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