#include <bstorm/code_generator.hpp>

#include <bstorm/const.hpp>
#include <bstorm/util.hpp>

namespace bstorm
{
static std::string varname(const std::string& name)
{
    return bstorm::DNH_VAR_PREFIX + name;
}

static std::string runtime(const std::string& name)
{
    return bstorm::DNH_RUNTIME_PREFIX + name;
}

static bool isDeclarationNeeded(const std::shared_ptr<NodeDef>& def)
{
    if (std::dynamic_pointer_cast<NodeVarDecl>(def)) return true;
    if (std::dynamic_pointer_cast<NodeFuncDef>(def)) return true;
    if (std::dynamic_pointer_cast<NodeSubDef>(def)) return true;
    if (std::dynamic_pointer_cast<NodeBuiltInSubDef>(def)) return true;
    if (std::dynamic_pointer_cast<NodeTaskDef>(def)) return true;
    return false;
}

CodeGenerator::CodeGenerator() :
    indentLevel(0),
    outputLine(1),
    isLineHead(true)
{
}
void CodeGenerator::traverse(NodeNum& exp)
{
    addCode(exp.number);
}
void CodeGenerator::traverse(NodeChar& exp)
{
    addCode("[=[" + toUTF8(std::wstring{ exp.c }) + "]=]");
    if (exp.c == L'\r' || exp.c == L'\n') outputLine++;
}
void CodeGenerator::traverse(NodeStr& exp)
{
    addCode("{");
    for (int i = 0; i < exp.str.size(); i++)
    {
        if (i != 0) addCode(",");
        NodeChar(exp.str[i]).traverse(*this);
    }
    addCode("}");
}
void CodeGenerator::traverse(NodeArray& exp)
{
    addCode("{");
    for (int i = 0; i < exp.elems.size(); i++)
    {
        if (i != 0) addCode(",");
        exp.elems[i]->traverse(*this);
    }
    addCode("}");
}
void CodeGenerator::traverse(NodeNeg& exp) { genMonoOp("negative", exp); }
void CodeGenerator::traverse(NodeNot& exp) { genMonoOp("not", exp); }
void CodeGenerator::traverse(NodeAbs& exp) { genMonoOp("absolute", exp); }

void CodeGenerator::traverse(NodeAdd& exp) { genBinOp("add", exp); }
void CodeGenerator::traverse(NodeSub& exp) { genBinOp("subtract", exp); }
void CodeGenerator::traverse(NodeMul& exp) { genBinOp("multiply", exp); }
void CodeGenerator::traverse(NodeDiv& exp) { genBinOp("divide", exp); }
void CodeGenerator::traverse(NodeRem& exp) { genBinOp("remainder", exp); }
void CodeGenerator::traverse(NodePow& exp) { genBinOp("power", exp); }

void CodeGenerator::traverse(NodeLt& exp) { genCmpBinOp("<", exp); }
void CodeGenerator::traverse(NodeGt& exp) { genCmpBinOp(">", exp); }
void CodeGenerator::traverse(NodeLe& exp) { genCmpBinOp("<=", exp); }
void CodeGenerator::traverse(NodeGe& exp) { genCmpBinOp(">=", exp); }
void CodeGenerator::traverse(NodeEq& exp) { genCmpBinOp("==", exp); }
void CodeGenerator::traverse(NodeNe& exp) { genCmpBinOp("~=", exp); }

void CodeGenerator::traverse(NodeAnd& exp) { genLogBinOp("and", exp); }
void CodeGenerator::traverse(NodeOr& exp) { genLogBinOp("or", exp); }
void CodeGenerator::traverse(NodeCat& exp)
{
    if (isCopyNeeded(exp.lhs))
    {
        genBinOp("concatenate", exp);
    } else
    {
        genBinOp("mconcatenate", exp);
    }
}
void CodeGenerator::traverse(NodeNoParenCallExp& call)
{
    auto def = env->findDef(call.name);
    if (std::dynamic_pointer_cast<NodeVarDecl>(def) ||
        std::dynamic_pointer_cast<NodeProcParam>(def) ||
        std::dynamic_pointer_cast<NodeLoopParam>(def))
    {
        genCheckNil(call.name);
    } else if (call.name == "GetCurrentScriptDirectory" && std::dynamic_pointer_cast<NodeBuiltInFunc>(def))
    {
        NodeStr(parentPath(*call.srcPos->filename) + L"/").traverse(*this);
    } else if (auto c = std::dynamic_pointer_cast<NodeConst>(def))
    {
        addCode(toUTF8(c->value) + " --[[ " + call.name + " ]]");
    } else
    {
        addCode(varname(call.name) + "()");
    }
}

void CodeGenerator::traverse(NodeCallExp& call)
{
    auto def = env->findDef(call.name);
    if (call.name == "GetCurrentScriptDirectory" && std::dynamic_pointer_cast<NodeBuiltInFunc>(def))
    {
        NodeStr(parentPath(*call.srcPos->filename) + L"/").traverse(*this);
    } else if (auto c = std::dynamic_pointer_cast<NodeConst>(def))
    {
        addCode(toUTF8(c->value) + " --[[ " + call.name + " ]]");
    } else
    {
        addCode(varname(call.name) + "(");
        bool isUserFunc = !std::dynamic_pointer_cast<NodeBuiltInFunc>(def);
        for (int i = 0; i < call.args.size(); i++)
        {
            if (i != 0) addCode(",");
            if (isUserFunc)
            {
                genCopy(call.args[i]);
            } else
            {
                call.args[i]->traverse(*this);
            }
        }
        addCode(")");
    }
}

void CodeGenerator::traverse(NodeHeader &) {}

void CodeGenerator::addCode(const std::wstring & s)
{
    addCode(toUTF8(s));
}
void CodeGenerator::addCode(const std::string& s)
{
#ifdef _DEBUG
    constexpr char* tab = "  ";
    if (isLineHead && !s.empty())
    {
        for (int i = 0; i < indentLevel; i++) { code += tab; }
        isLineHead = false;
    }
#endif
    code += s;
}
void CodeGenerator::newLine()
{
    outputLine++;
    addCode("\n");
    isLineHead = true;
}
void CodeGenerator::newLine(const std::shared_ptr<SourcePos>& srcPos)
{
    srcMap.logSourcePos(outputLine, srcPos->filename, srcPos->line);
    newLine();
}
void CodeGenerator::indent()
{
    indentLevel++;
}
void CodeGenerator::unindent()
{
    indentLevel--;
}
void CodeGenerator::genMonoOp(const std::string& fname, NodeMonoOp& exp)
{
    addCode(runtime(fname) + "(");
    exp.rhs->traverse(*this);
    addCode(")");
}
void CodeGenerator::genBinOp(const std::string& fname, NodeBinOp& exp)
{
    addCode(runtime(fname) + "(");
    exp.lhs->traverse(*this);
    addCode(",");
    exp.rhs->traverse(*this);
    addCode(")");
}
void CodeGenerator::genCmpBinOp(const std::string & op, NodeBinOp & exp)
{
    addCode("(" + runtime("compare") + "(");
    exp.lhs->traverse(*this);
    addCode(",");
    exp.rhs->traverse(*this);
    addCode(") ");
    addCode(op);
    addCode(" 0)");
}
void CodeGenerator::genLogBinOp(const std::string & fname, NodeBinOp & exp)
{
    addCode(runtime(fname) + "(");
    exp.lhs->traverse(*this);
    addCode(", function() return ");
    exp.rhs->traverse(*this);
    addCode(" end");
    addCode(")");
}
void CodeGenerator::genCheckNil(const std::string & name)
{
    addCode("r_checknil(" + varname(name) + ", \"" + name + "\")");
}
void CodeGenerator::genProc(std::shared_ptr<NodeDef> def, const std::vector<std::string>& params, NodeBlock & blk)
{
    addCode(varname(def->name) + " = function(");
    for (int i = 0; i < params.size(); i++)
    {
        if (i != 0) addCode(",");
        addCode(varname(params[i]));
    }
    addCode(")"); newLine();
    blk.traverse(*this);
    if (std::dynamic_pointer_cast<NodeFuncDef>(def))
    {
        auto result = blk.table.at("result");
        if (!std::dynamic_pointer_cast<NodeProcParam>(result))
        {
            indent();
            addCode("do return " + varname("result") + " end");
            newLine(result->srcPos);
            unindent();
        }
    }
    addCode("end"); newLine();
}
void CodeGenerator::genOpAssign(const std::string & fname, const std::shared_ptr<NodeLeftVal>& left, std::shared_ptr<NodeExp> right)
{
    switch (left->indices.size())
    {
        case 0:
            // a += e;
            // out : a = add(r_checknil(a), e);
            addCode(varname(left->name) + " = ");
            addCode(runtime(fname) + "(");
            genCheckNil(left->name);
            if (right)
            {
                addCode(",");
                right->traverse(*this);
            }
            addCode(");"); newLine(left->srcPos);
            break;
        case 1:
            // a[_i] += e;
            // out :  r_checknil(a);
            //        local i = _i;
            //        r_write1(a, i, r_add(r_read(a, i), e));
            addCode("do"); newLine();
            genCheckNil(left->name); newLine(left->srcPos);
            addCode("local i = "); left->indices[0]->traverse(*this); addCode(";"); newLine(left->srcPos);
            addCode("r_write1(" + varname(left->name) + ", i, ");
            addCode(runtime(fname) + "(");
            addCode(runtime("read") + "(" + varname(left->name) + ", i)");
            if (right)
            {
                addCode(",");
                right->traverse(*this);
            }
            addCode("));"); newLine(left->srcPos);
            addCode("end"); newLine();
            break;
        default:
            // a[i][j]..[z] += e;
            // out : check_nil(a);
            //       local is = {i, j, .. , z};
            //       r_write(check_nil(a), is, r_add(r_read(..r_read(a, is[1]), is[2]), .. is[n]), e);
            addCode("do"); newLine();
            genCheckNil(left->name); newLine(left->srcPos);
            addCode("local is = {");
            for (int i = 0; i < left->indices.size(); i++)
            {
                if (i != 0) addCode(",");
                left->indices[i]->traverse(*this);
            }
            addCode("};"); newLine(left->srcPos);
            addCode("r_write(" + varname(left->name) + ", is");
            addCode("," + runtime(fname) + "(");
            for (int i = 0; i < left->indices.size(); i++)
            {
                addCode(runtime("read") + "(");
            }
            addCode(varname(left->name) + ",");
            for (int i = 0; i < left->indices.size(); i++)
            {
                if (i != 0) addCode(",");
                addCode("is[" + std::to_string(i + 1) + "])");
            }
            if (right)
            {
                addCode(",");
                right->traverse(*this);
            }
            addCode("));"); newLine(left->srcPos);
            addCode("end"); newLine();
            break;
    }
}
void CodeGenerator::genCopy(std::shared_ptr<NodeExp> exp)
{
    if (isCopyNeeded(exp))
    {
        addCode("r_cp("); exp->traverse(*this); addCode(")");
    } else
    {
        exp->traverse(*this);
    }
}
bool CodeGenerator::isCopyNeeded(const std::shared_ptr<NodeExp>& exp)
{
    if (std::dynamic_pointer_cast<NodeNum>(exp) ||
        std::dynamic_pointer_cast<NodeChar>(exp) ||
        std::dynamic_pointer_cast<NodeStr>(exp) ||
        std::dynamic_pointer_cast<NodeMonoOp>(exp) ||
        std::dynamic_pointer_cast<NodeArrayRef>(exp) ||
        std::dynamic_pointer_cast<NodeArraySlice>(exp))
    {
        return false;
    } else if (auto arr = std::dynamic_pointer_cast<NodeArray>(exp))
    {
        for (auto& elem : arr->elems)
        {
            if (isCopyNeeded(elem)) return true;
        }
        return false;
    } else if (auto op = std::dynamic_pointer_cast<NodeAnd>(exp))
    {
        return isCopyNeeded(op->lhs) || isCopyNeeded(op->rhs);
    } else if (auto op = std::dynamic_pointer_cast<NodeOr>(exp))
    {
        return isCopyNeeded(op->lhs) || isCopyNeeded(op->rhs);
    } else if (std::dynamic_pointer_cast<NodeBinOp>(exp))
    {
        return false;
    } else if (auto call = std::dynamic_pointer_cast<NodeNoParenCallExp>(exp))
    {
        auto def = env->findDef(call->name);
        if (std::dynamic_pointer_cast<NodeBuiltInFunc>(def) ||
            std::dynamic_pointer_cast<NodeConst>(def))
        {
            return false;
        }
        return true;
    } else if (auto call = std::dynamic_pointer_cast<NodeCallExp>(exp))
    {
        auto def = env->findDef(call->name);
        if (std::dynamic_pointer_cast<NodeBuiltInFunc>(def) ||
            std::dynamic_pointer_cast<NodeConst>(def))
        {
            return false;
        }
        return true;
    }
    return true;
}
void CodeGenerator::traverse(NodeArrayRef& exp)
{
    addCode(runtime("read") + "(");
    exp.array->traverse(*this);
    addCode(",");
    exp.idx->traverse(*this);
    addCode(")");
}
void CodeGenerator::traverse(NodeRange &) {}
void CodeGenerator::traverse(NodeArraySlice& exp)
{
    addCode(runtime("slice") + "(");
    exp.array->traverse(*this);
    addCode(",");
    exp.range->start->traverse(*this);
    addCode(",");
    exp.range->end->traverse(*this);
    addCode(")");
}
void CodeGenerator::traverse(NodeNop &) {}
void CodeGenerator::traverse(NodeLeftVal &) {}
void CodeGenerator::traverse(NodeAssign& stmt)
{
    auto def = env->findDef(stmt.lhs->name);
    switch (stmt.lhs->indices.size())
    {
        case 0:
            // a = e;
            // out : a = r_cp(e);
            addCode(varname(def->name) + " = "); genCopy(stmt.rhs); addCode(";");
            break;
        case 1:
            // a[i] = e;
            // out : r_write1(r_checknil(a), i, e);
            addCode("r_write1(");
            genCheckNil(def->name);
            addCode(",");
            stmt.lhs->indices[0]->traverse(*this);
            addCode(",");
            stmt.rhs->traverse(*this);
            addCode(");");
            break;
        default:
            // a[i1][i2] .. [in] = e;
            // out : r_write(r_checknil(a), {i1, i2, .. in}, e);
            addCode("r_write(");
            genCheckNil(def->name);
            addCode(", {");
            for (int i = 0; i < stmt.lhs->indices.size(); i++)
            {
                if (i != 0) addCode(",");
                stmt.lhs->indices[i]->traverse(*this);
            }
            addCode("}, ");
            stmt.rhs->traverse(*this);
            addCode(");");
            break;
    }
    newLine(stmt.srcPos);
}
void CodeGenerator::traverse(NodeAddAssign& stmt) { genOpAssign("add", stmt.lhs, stmt.rhs); }
void CodeGenerator::traverse(NodeSubAssign& stmt) { genOpAssign("subtract", stmt.lhs, stmt.rhs); }
void CodeGenerator::traverse(NodeMulAssign& stmt) { genOpAssign("multiply", stmt.lhs, stmt.rhs); }
void CodeGenerator::traverse(NodeDivAssign& stmt) { genOpAssign("divide", stmt.lhs, stmt.rhs); }
void CodeGenerator::traverse(NodeRemAssign& stmt) { genOpAssign("remainder", stmt.lhs, stmt.rhs); }
void CodeGenerator::traverse(NodePowAssign& stmt) { genOpAssign("power", stmt.lhs, stmt.rhs); }
void CodeGenerator::traverse(NodeCatAssign& stmt) { genOpAssign("mconcatenate", stmt.lhs, stmt.rhs); }

void CodeGenerator::traverse(NodeCallStmt& call)
{
    auto def = env->findDef(call.name);

    if (std::dynamic_pointer_cast<NodeConst>(def)) return;

    bool isTask = (bool)std::dynamic_pointer_cast<NodeTaskDef>(def);
    if (isTask)
    {
        addCode("r_fork(" + varname(call.name));
        if (call.args.size() != 0)
        {
            addCode(", {");
        }
    } else
    {
        addCode(varname(call.name) + "(");
    }
    bool isUserFunc = !std::dynamic_pointer_cast<NodeBuiltInFunc>(def);
    for (int i = 0; i < call.args.size(); i++)
    {
        if (i != 0) addCode(",");
        if (isUserFunc)
        {
            genCopy(call.args[i]);
        } else
        {
            call.args[i]->traverse(*this);
        }
    }
    if (isTask && call.args.size() != 0)
    {
        addCode("}");
    }
    addCode(");"); newLine(call.srcPos);
}
void CodeGenerator::traverse(NodeReturn& stmt)
{
    addCode("do return "); stmt.ret->traverse(*this); addCode(" end");
    newLine(stmt.srcPos);
}
void CodeGenerator::traverse(NodeReturnVoid& stmt)
{
    if (procStack.empty())
    {
        // top level return
        addCode("do return end");
    } else
    {
        if (auto func = std::dynamic_pointer_cast<NodeFuncDef>(procStack.top()))
        {
            auto result = func->block->table.at("result");
            if (auto funcParam = std::dynamic_pointer_cast<NodeProcParam>(result))
            {
                // function has "result" param
                addCode("do return end");
            } else
            {
                // return implicit result
                addCode("do return " + varname("result") + " end");
            }
        } else
        {
            // return from sub or task
            addCode("do return end");
        }
    }
    newLine(stmt.srcPos);
}
void CodeGenerator::traverse(NodeYield& stmt)
{
    if (!procStack.empty())
    {
        addCode("coroutine.yield();");
        newLine(stmt.srcPos);
    }
}
void CodeGenerator::traverse(NodeBreak& stmt)
{
    if (procStack.empty())
    {
        addCode("do return end");
    } else
    {
        addCode("do break end");
    }
    newLine(stmt.srcPos);
}

void CodeGenerator::traverse(NodeSucc& stmt) { genOpAssign("successor", stmt.lhs, std::shared_ptr<NodeExp>()); }
void CodeGenerator::traverse(NodePred& stmt) { genOpAssign("predecessor", stmt.lhs, std::shared_ptr<NodeExp>()); }

void CodeGenerator::traverse(NodeVarDecl &) {}
void CodeGenerator::traverse(NodeVarInit& stmt)
{
    addCode(varname(stmt.name) + " = "); genCopy(stmt.rhs); addCode(";");
    newLine(stmt.rhs->srcPos);
}
void CodeGenerator::traverse(NodeProcParam &) {}
void CodeGenerator::traverse(NodeLoopParam& param)
{
    addCode("local " + varname(param.name) + " = r_cp(i);");
    newLine(param.srcPos);
}
void CodeGenerator::traverse(NodeBuiltInFunc &) {}
void CodeGenerator::traverse(NodeConst &) {}
void CodeGenerator::traverse(NodeLocal& stmt)
{
    addCode("do"); newLine();
    stmt.block->traverse(*this);
    addCode("end"); newLine();
}
void CodeGenerator::traverse(NodeLoop& stmt)
{
    addCode("while 1 do"); newLine();
    stmt.block->traverse(*this);
    addCode("end"); newLine();
}
void CodeGenerator::traverse(NodeTimes& stmt)
{
    addCode("do"); newLine();
    addCode("local i = 0;"); newLine(stmt.srcPos);
    addCode("local e = math.ceil(r_tonum(");  stmt.cnt->traverse(*this); addCode("));");
    newLine(stmt.cnt->srcPos);
    addCode("while i < e do"); newLine();
    stmt.block->traverse(*this);
    indent();
    addCode("i = i + 1;"); newLine();
    unindent();
    addCode("end"); newLine();
    addCode("end"); newLine();
}
void CodeGenerator::traverse(NodeWhile& stmt)
{
    addCode("while r_tobool("); stmt.cond->traverse(*this); addCode(") do"); newLine(stmt.cond->srcPos);
    stmt.block->traverse(*this);
    addCode("end"); newLine();
}
void CodeGenerator::traverse(NodeAscent& stmt)
{
    addCode("do"); newLine();
    addCode("local i = ");  stmt.range->start->traverse(*this); addCode(";"); newLine(stmt.range->start->srcPos);
    addCode("local e = ");  stmt.range->end->traverse(*this); addCode(";"); newLine(stmt.range->end->srcPos);
    addCode("while " + runtime("compare") + "(e, i) > 0 do"); newLine(stmt.srcPos);
    stmt.block->traverse(*this);
    indent(); addCode("i = " + runtime("successor") + "(i);"); newLine(stmt.srcPos); unindent();
    addCode("end"); newLine();
    addCode("end"); newLine();
}
void CodeGenerator::traverse(NodeDescent& stmt)
{
    addCode("do"); newLine();
    addCode("local s = ");  stmt.range->start->traverse(*this); addCode(";"); newLine(stmt.range->start->srcPos);
    addCode("local i = ");  stmt.range->end->traverse(*this); addCode(";"); newLine(stmt.range->end->srcPos);
    addCode("while " + runtime("compare") + "(s, i) < 0 do"); newLine(stmt.srcPos);
    indent(); addCode("i = " + runtime("predecessor") + "(i);"); newLine(stmt.srcPos); unindent();
    stmt.block->traverse(*this);
    addCode("end"); newLine();
    addCode("end"); newLine();
}
void CodeGenerator::traverse(NodeElseIf& elsif)
{
    addCode("elseif r_tobool("); elsif.cond->traverse(*this); addCode(") then"); newLine(elsif.cond->srcPos);
    elsif.block->traverse(*this);
}
void CodeGenerator::traverse(NodeIf& stmt)
{
    addCode("if r_tobool("); stmt.cond->traverse(*this); addCode(") then"); newLine(stmt.cond->srcPos);
    stmt.thenBlock->traverse(*this);
    for (auto elsif : stmt.elsifs) { elsif->traverse(*this); }
    if (stmt.elseBlock)
    {
        addCode("else"); newLine();
        stmt.elseBlock->traverse(*this);
    }
    addCode("end"); newLine();
}
void CodeGenerator::traverse(NodeCase &) {}
void CodeGenerator::traverse(NodeAlternative& stmt)
{
    addCode("do"); newLine();
    addCode("local c = "); genCopy(stmt.cond); addCode(";"); newLine(stmt.cond->srcPos);
    /* gen if-seq */
    for (int i = 0; i < stmt.cases.size(); i++)
    {
        // case
        if (i != 0) addCode("else"); // 1つ目以降のcaseはelse if
        addCode("if false");
        // orの右結合計算列を作る(右結合の方が早く短絡するので)
        for (auto& exp : stmt.cases[i]->exps)
        {
            addCode(" or (");
            addCode("(" + runtime("compare") + "(c,"); exp->traverse(*this); addCode(") == 0)");
        }
        for (auto& exp : stmt.cases[i]->exps)
        {
            addCode(")");
        }
        addCode(" then"); newLine(stmt.cases[i]->srcPos);
        stmt.cases[i]->block->traverse(*this);
    }
    /* gen else*/
    if (stmt.others)
    {
        addCode(stmt.cases.empty() ? "do" : "else"); newLine();
        stmt.others->traverse(*this);
        addCode("end"); newLine();
    } else
    {
        if (!stmt.cases.empty())
        {
            addCode("end"); newLine();
        }
    }
    addCode("end"); newLine();
}
void CodeGenerator::traverse(NodeBlock& blk)
{
    auto newEnv = std::make_shared<Env>();
    newEnv->parent = env;
    newEnv->table = blk.table;
    env = newEnv;

    if (!env->isRoot())
    {
        indent();
        // local declare
        for (const auto& bind : blk.table)
        {
            auto def = bind.second;
            if (isDeclarationNeeded(def))
            {
                addCode("local "); addCode(varname(def->name)); addCode(";"); newLine(def->srcPos);
            }
        }
    }

    for (const auto& bind : blk.table)
    {
        bind.second->traverse(*this);
    }

    for (auto& stmt : blk.stmts)
    {
        stmt->traverse(*this);
    }

    if (!env->isRoot())
    {
        unindent();
    }
    env = env->parent;
}
void CodeGenerator::traverse(NodeSubDef& sub)
{
    std::shared_ptr<NodeSubDef> def = std::make_shared<NodeSubDef>(sub);
    procStack.push(def);
    genProc(def, {}, *sub.block);
    procStack.pop();
}
void CodeGenerator::traverse(NodeBuiltInSubDef& sub)
{
    std::shared_ptr<NodeBuiltInSubDef> def = std::make_shared<NodeBuiltInSubDef>(sub);
    procStack.push(def);
    genProc(def, {}, *sub.block);
    procStack.pop();
}
void CodeGenerator::traverse(NodeFuncDef& func)
{
    std::shared_ptr<NodeFuncDef> def = std::make_shared<NodeFuncDef>(func);
    procStack.push(def);
    genProc(def, func.params, *func.block);
    procStack.pop();
}
void CodeGenerator::traverse(NodeTaskDef& task)
{
    std::shared_ptr<NodeTaskDef> def = std::make_shared<NodeTaskDef>(task);
    procStack.push(def);
    genProc(def, task.params, *task.block);
    procStack.pop();
}
}