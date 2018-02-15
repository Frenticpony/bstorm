#include <exception>

#include <bstorm/dnh_const.hpp>
#include <bstorm/const.hpp>
#include <bstorm/util.hpp>
#include <bstorm/script_info.hpp>
#include <bstorm/parser.hpp>
#include <bstorm/obj.hpp>
#include <bstorm/dnh_value.hpp>
#include <bstorm/engine.hpp>
#include <bstorm/sem_checker.hpp>
#include <bstorm/code_generator.hpp>
#include <bstorm/api.hpp>
#include <bstorm/file_loader.hpp>
#include <bstorm/logger.hpp>
#include <bstorm/engine.hpp>
#include <bstorm/fps_counter.hpp>
#include <bstorm/script.hpp>

#include "runtime.h"

namespace bstorm {
  const std::unordered_set<std::wstring> ignoreScriptExts{ L".png", L".jpg", L".jpeg", L".bmp", L".gif", L".dds", L".hdr", L".dib", L".pfm", L".tif", L".tiff", L".ttf", L".otf", L".mqo", L".mp3", L".mp4", L".avi", L".ogg", L".wav", L".wave", L".def", L".dat", L".fx", L".exe" };

  Script::Script(const std::wstring& p, const std::wstring& type, const std::wstring& version, int id, Engine* engine, const std::shared_ptr<SourcePos>& srcPos) :
    L(NULL),
    path(canonicalPath(p)),
    type(type),
    id(id),
    state(State::SCRIPT_NOT_COMPILED),
    luaStateBusy(false),
    stgSceneScript(type != SCRIPT_TYPE_PACKAGE),
    autoDeleteObjectEnable(false),
    engine(engine)
  {
    try {
      L = lua_open();
      // Lua標準API登録
      luaL_openlibs(L);

      // 弾幕風標準API登録
      auto globalEnv = std::make_shared<Env>();
      addStandardAPI(type, version, globalEnv->table);

      for (const auto& entry : globalEnv->table) {
        if (auto builtInFunc = std::dynamic_pointer_cast<NodeBuiltInFunc>(entry.second)) {
          if (builtInFunc->funcPointer) {
            lua_register(L, (DNH_VAR_PREFIX + entry.first).c_str(), (lua_CFunction)builtInFunc->funcPointer);
          }
        }
      }

      // ランタイム用関数登録
      lua_register(L, "c_chartonum", c_chartonum);
      lua_register(L, "c_succchar", c_succchar);
      lua_register(L, "c_predchar", c_predchar);
      lua_register(L, "c_raiseerror", c_raiseerror);

      // ポインタ設定
      setEngine(L, engine);
      setScript(L, this);

      // パース
      std::shared_ptr<NodeBlock> program;
      {
        Logger::WriteLog(std::move(
          Log(Log::Level::LV_DETAIL)
          .setMessage("parse start...")
          .setParam(Log::Param(Log::Param::Tag::SCRIPT, path))
          .addSourcePos(srcPos)));
        TimePoint tp;
        program = parseDnhScript(path, globalEnv, true, std::make_shared<FileLoaderFromTextFile>());
        Logger::WriteLog(std::move(
          Log(Log::Level::LV_DETAIL)
          .setMessage("...parse complete " + std::to_string(tp.getElapsedMilliSec()) + " [ms].")
          .setParam(Log::Param(Log::Param::Tag::SCRIPT, path))
          .addSourcePos(srcPos)));
      }

      // 静的エラー検査
      {
        SemChecker checker;
        auto errors = checker.check(*program);
        for (auto& err : errors) {
          Logger::WriteLog(err);
        }
        if (!errors.empty()) {
          throw Log(Log::Level::LV_ERROR)
            .setMessage("found " + std::to_string(errors.size()) + " script error" + (errors.size() > 1 ? "s." : "."))
            .setParam(Log::Param(Log::Param::Tag::SCRIPT, path));
        }
      }

      // 変換
      CodeGenerator codeGen;
      {
        Logger::WriteLog(std::move(
          Log(Log::Level::LV_DETAIL)
          .setMessage("codegen start...")
          .setParam(Log::Param(Log::Param::Tag::SCRIPT, path))
          .addSourcePos(srcPos)));
        TimePoint tp;
        codeGen.generate(*program);
        Logger::WriteLog(std::move(
          Log(Log::Level::LV_DETAIL)
          .setMessage("...codegen complete " + std::to_string(tp.getElapsedMilliSec()) + " [ms].")
          .setParam(Log::Param(Log::Param::Tag::SCRIPT, path))
          .addSourcePos(srcPos)));
        srcMap = codeGen.getSourceMap();
      }

      // ランタイム読み込み
      luaL_loadbuffer(L, (const char *)luaJIT_BC_runtime, luaJIT_BC_runtime_SIZE, DNH_RUNTIME_NAME);
      callLuaChunk();

      // コンパイル
      {
        Logger::WriteLog(std::move(
          Log(Log::Level::LV_DETAIL)
          .setMessage("compile start...")
          .setParam(Log::Param(Log::Param::Tag::SCRIPT, path))
          .addSourcePos(srcPos)));
        TimePoint tp;
        int hasCompileError = luaL_loadstring(L, codeGen.getCode());
        if (hasCompileError) {
          std::string msg = lua_tostring(L, -1); lua_pop(L, 1);
          if (msg.find("has more than 200 local variables") != std::string::npos) {
            auto ss = split(toUnicode(msg), L':');
            Log err = Log(Log::Level::LV_ERROR)
              .setMessage("too many variable used in one function.");
            if (ss.size() >= 2) {
              int line = _wtoi(ss[1].c_str());
              err.addSourcePos(srcMap.getSourcePos(line));
            } else {
              err.setParam(Log::Param(Log::Param::Tag::SCRIPT, path));
            }
            throw err;
          } else {
            throw Log(Log::Level::LV_ERROR)
              .setMessage("unexpected compile error occured, please send a bug report.")
              .setParam(Log::Param(Log::Param::Tag::TEXT, msg));
          }
        }
        Logger::WriteLog(std::move(
          Log(Log::Level::LV_DETAIL)
          .setMessage("...compile complete " + std::to_string(tp.getElapsedMilliSec()) + " [ms].")
          .setParam(Log::Param(Log::Param::Tag::SCRIPT, path))
          .addSourcePos(srcPos)));
      }
      // グローバル領域のコード実行
      callLuaChunk();
      state = State::SCRIPT_COMPILED;
    } catch (...) {
      if (L) lua_close(L);
      throw;
    }
  }

  Script::~Script() {
    runBuiltInSub("Finalize");

    if (autoDeleteObjectEnable) {
      for (auto objId : autoDeleteTargetObjIds) {
        engine->deleteObject(objId);
      }
    }

    if (L) lua_close(L);

    Logger::WriteLog(std::move(
      Log(Log::Level::LV_INFO)
      .setMessage("release script.")
      .setParam(Log::Param(Log::Param(Log::Param::Tag::SCRIPT, path)))));
  }

  Script::State Script::getState() const { return state; }

  int Script::getID() const {
    return id;
  }

  std::wstring Script::getType() const {
    return type;
  }

  void Script::close() { state = State::SCRIPT_CLOSED; }

  bool Script::isClosed() const { return state == State::SCRIPT_CLOSED; }

  void Script::runBuiltInSub(const std::string &name) {
    if (!errLog) {
      if (luaStateBusy) {
        lua_getglobal(L, (DNH_VAR_PREFIX + name).c_str());
      } else {
        lua_getglobal(L, (std::string(DNH_RUNTIME_PREFIX) + "run_" + name).c_str());
      }
      callLuaChunk();
    }
  }

  void Script::callLuaChunk() {
    bool tmp = luaStateBusy;
    luaStateBusy = true;
    if (lua_pcall(L, 0, 0, 0) != 0) {
      luaStateBusy = false;
      std::string msg = lua_tostring(L, -1);
      lua_pop(L, 1);
      state = State::SCRIPT_CLOSED;
      if (!errLog) {
        errLog = std::make_shared<Log>(
          Log(Log::Level::LV_ERROR)
          .setMessage("unexpected script runtime error occured, please send a bug report.")
          .setParam(Log::Param(Log::Param::Tag::TEXT, msg)));
      }
      throw *errLog;
    }
    luaStateBusy = tmp;
  }

  void Script::runLoading() {
    if (state == State::SCRIPT_COMPILED) {
      runBuiltInSub("Loading");
      state = State::SCRIPT_LOADING_COMPLETE;
    }
  }

  void Script::runInitialize() {
    if (state == State::SCRIPT_LOADING_COMPLETE) {
      runBuiltInSub("Initialize");
      if (!isClosed()) {
        state = State::SCRIPT_RUNNING;
      }
    }
  }

  void Script::runMainLoop() {
    if (state == State::SCRIPT_RUNNING) {
      runBuiltInSub("MainLoop");
    }
  }

  void Script::notifyEvent(int eventType) {
    notifyEvent(eventType, std::make_unique<DnhArray>(L""));
  }

  void Script::notifyEvent(int eventType, const std::unique_ptr<DnhArray>& args) {
    DnhReal((double)eventType).push(L);
    lua_setglobal(L, "script_event_type");
    args->push(L);
    lua_setglobal(L, "script_event_args");
    setScriptResult(std::make_unique<DnhNil>());
    runBuiltInSub("Event");
  }

  bool Script::isStgSceneScript() const {
    return stgSceneScript;
  }

  void Script::setAutoDeleteObjectEnable(bool enable) {
    autoDeleteObjectEnable = enable;
  }

  void Script::addAutoDeleteTargetObjectId(int id) {
    if (id != ID_INVALID) {
      autoDeleteTargetObjIds.push_back(id);
    }
  }

  std::unique_ptr<DnhValue> Script::getScriptResult() const {
    return engine->getScriptResult(getID());
  }

  void Script::setScriptResult(std::unique_ptr<DnhValue>&& value) {
    engine->setScriptResult(getID(), std::move(value));
  }

  void Script::setScriptArgument(int idx, std::unique_ptr<DnhValue>&& value) {
    scriptArgs[idx] = std::move(value);
  }

  int Script::getScriptArgumentount() const {
    return scriptArgs.size();
  }

  std::unique_ptr<DnhValue> Script::getScriptArgument(int idx) {
    if (scriptArgs.count(idx) == 0) return std::make_unique<DnhNil>();
    return scriptArgs[idx]->clone();
  }

  std::shared_ptr<SourcePos> Script::getSourcePos(int line) {
    return srcMap.getSourcePos(line);
  }

  void Script::saveErrLog(const std::shared_ptr<Log>& log) {
    errLog = log;
  }

  ScriptManager::ScriptManager(Engine * engine) :
    idGen(0),
    engine(engine)
  {
  }

  ScriptManager::~ScriptManager() {}

  std::shared_ptr<Script> ScriptManager::newScript(const std::wstring& path, const std::wstring& type, const std::wstring& version, const std::shared_ptr<SourcePos>& srcPos) {
    auto script = std::make_shared<Script>(path, type, version, idGen++, engine, srcPos);
    scriptList.push_back(script);
    scriptMap[script->getID()] = script;
    Logger::WriteLog(std::move(
      Log(Log::Level::LV_INFO)
      .setMessage("load script.")
      .setParam(Log::Param(Log::Param(Log::Param::Tag::SCRIPT, canonicalPath(path))))
      .addSourcePos(srcPos)));
    return script;
  }

  void ScriptManager::runAll(bool ignoreStgSceneScript) {
    for (auto& script : scriptList) {
      if (!ignoreStgSceneScript || !script->isStgSceneScript()) {
        script->runMainLoop();
      }
    }
  }

  std::shared_ptr<Script> ScriptManager::get(int id) const {
    auto it = scriptMap.find(id);
    if (it != scriptMap.end()) {
      if (!it->second->isClosed()) {
        return it->second;
      }
    }
    return nullptr;
  }

  void ScriptManager::notifyEventAll(int eventType) {
    for (auto& script : scriptList) {
      // NOTE: NotifyEventAllで送るとcloseされたスクリプトには届かない
      if (!script->isClosed()) {
        script->notifyEvent(eventType);
      }
    }
  }

  void ScriptManager::notifyEventAll(int eventType, const std::unique_ptr<DnhArray>& args) {
    for (auto& script : scriptList) {
      script->notifyEvent(eventType, args);
    }
  }

  void ScriptManager::cleanClosedScript() {
    auto it = scriptList.begin();
    while (it != scriptList.end()) {
      auto& script = *it;
      if (script->isClosed()) {
        scriptMap.erase(script->getID());
        it = scriptList.erase(it);
      } else ++it;
    }
  }

  void ScriptManager::closeStgSceneScript() {
    for (auto& script : scriptList) {
      if (script->isStgSceneScript()) {
        script->close();
      }
    }
  }

  std::unique_ptr<DnhValue> ScriptManager::getScriptResult(int scriptId) const {
    auto it = scriptResults.find(scriptId);
    if (it != scriptResults.end()) {
      return it->second->clone();
    }
    return std::make_unique<DnhNil>();
  }

  void ScriptManager::setScriptResult(int scriptId, std::unique_ptr<DnhValue>&& value) {
    scriptResults[scriptId] = std::move(value);
  }

  void ScriptManager::clearScriptResult() {
    scriptResults.clear();
  }
}