#include <bstorm/serialized_script.hpp>

#include <bstorm/util.hpp>
#include <bstorm/lua_util.hpp>
#include <bstorm/logger.hpp>
#include <bstorm/api.hpp>
#include <bstorm/source_map.hpp>
#include <bstorm/node.hpp>
#include <bstorm/parser.hpp>
#include <bstorm/semantics_checker.hpp>
#include <bstorm/code_generator.hpp>

#include <luajit/lua.hpp>

namespace bstorm
{
SerializedScript::SerializedScript(const std::wstring & path, ScriptType type, const std::wstring & version, const std::shared_ptr<FileLoader>& fileLoader) :
    type_(type),
    version_(version)
{
    std::unique_ptr<lua_State, decltype(&lua_close)> L(luaL_newstate(), lua_close);
    // 環境作成
    auto globalEnv = std::make_shared<Env>();
    RegisterStandardAPI(L.get(), type_, version_, globalEnv);

    // パース
    ScriptInfo scriptInfo;
    std::shared_ptr<NodeBlock> program = ParseDnhScript(path, globalEnv, true, &scriptInfo, fileLoader);

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
                .SetParam(Log::Param(Log::Param::Tag::SCRIPT, path));
        }
    }

    // 変換
    CodeGenerator codeGen;
    codeGen.Generate(true, *program);

    // コンパイル
    {
        int hasCompileError = luaL_loadstring(L.get(), codeGen.GetCode().c_str());
        if (hasCompileError)
        {
            std::string msg = lua_tostring(L.get(), -1); lua_pop(L.get(), 1);
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
                    err.SetParam(Log::Param(Log::Param::Tag::SCRIPT, path));
                }
                throw err;
            } else
            {
                throw Log(Log::Level::LV_ERROR)
                    .SetMessage("unexpected compile error occured, please send a bug report. (" + msg + ")")
                    .SetParam(Log::Param(Log::Param::Tag::SCRIPT, path));
            }
        }
    }
    scriptInfo.Serialize(scriptInfo_);
    codeGen.GetSourceMap().Serialize(srcMap_);
    SerializeChunk(L.get(), byteCode_);
}
}