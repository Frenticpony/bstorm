#include <bstorm/semantics_checker.hpp>

#include <bstorm/env.hpp>

namespace bstorm
{
static Log invalid_return(const std::shared_ptr<SourcePos>& srcPos)
{
    return Log(LogLevel::LV_ERROR)
        .Msg("'return' with a value is available only to function.")
        .AddSourcePos(srcPos);
}

static Log variable_call(const std::shared_ptr<SourcePos>& srcPos, const std::string& name)
{
    return Log(LogLevel::LV_ERROR)
        .Msg("can't call variable '" + name + "' as if it were a function.")
        .AddSourcePos(srcPos);
}

static Log undefined_name(const std::shared_ptr<SourcePos>& srcPos, const std::string& name)
{
    return Log(LogLevel::LV_ERROR)
        .Msg("'" + name + "' is not defined.")
        .AddSourcePos(srcPos);
}

static Log wrong_number_args(const std::shared_ptr<SourcePos>& srcPos, const std::string& name, int passed, int expected)
{
    auto msg = "wrong number of arguments was passed to '" + name + "' (passed : " + std::to_string(passed) + ", expected : " + std::to_string(expected) + ").";
    return Log(LogLevel::LV_ERROR)
        .Msg(msg)
        .AddSourcePos(srcPos);
}

static Log invalid_left_value(const std::shared_ptr<SourcePos>& srcPos, const std::string& name)
{
    return Log(LogLevel::LV_ERROR)
        .Msg("invalid assignment : '" + name + "' is not a variable.")
        .AddSourcePos(srcPos);
}

static Log invalid_sub_call(const std::shared_ptr<SourcePos>& srcPos, const std::string& name)
{
    return Log(LogLevel::LV_ERROR)
        .Msg("can't call subroutine '" + name + "' as an expression.")
        .AddSourcePos(srcPos);
}

static Log invalid_task_call(const std::shared_ptr<SourcePos>& srcPos, const std::string& name)
{
    return Log(LogLevel::LV_ERROR)
        .Msg("can't call micro thread '" + name + "' as an expression.")
        .AddSourcePos(srcPos);
}

static Log invalid_break(const std::shared_ptr<SourcePos>& srcPos)
{
    return Log(LogLevel::LV_ERROR)
        .Msg("'break' outside the loop.")
        .AddSourcePos(srcPos);
}

static int GetParamCnt(const std::shared_ptr<NodeDef>& def)
{
    if (auto b = std::dynamic_pointer_cast<NodeBuiltInFunc>(def)) return (int)b->paramCnt;
    if (auto f = std::dynamic_pointer_cast<NodeFuncDef>(def)) return (int)f->params.size();
    if (auto t = std::dynamic_pointer_cast<NodeTaskDef>(def)) return (int)t->params.size();
    return 0;
}

std::vector<Log> SemanticsChecker::Check(Node & n)
{
    env_ = nullptr;
    errors_.clear();
    n.Traverse(*this);
    return errors_;
}

void SemanticsChecker::Traverse(NodeNum&) {}
void SemanticsChecker::Traverse(NodeChar&) {}
void SemanticsChecker::Traverse(NodeStr&) {}
void SemanticsChecker::Traverse(NodeArray& arr)
{
    for (auto elem : arr.elems) { elem->Traverse(*this); }
}
void SemanticsChecker::Traverse(NodeNeg& exp) { CheckMonoOp(exp); }
void SemanticsChecker::Traverse(NodeNot& exp) { CheckMonoOp(exp); }
void SemanticsChecker::Traverse(NodeAbs& exp) { CheckMonoOp(exp); }
void SemanticsChecker::Traverse(NodeAdd& exp) { CheckBinOp(exp); }
void SemanticsChecker::Traverse(NodeSub& exp) { CheckBinOp(exp); }
void SemanticsChecker::Traverse(NodeMul& exp) { CheckBinOp(exp); }
void SemanticsChecker::Traverse(NodeDiv& exp) { CheckBinOp(exp); }
void SemanticsChecker::Traverse(NodeRem& exp) { CheckBinOp(exp); }
void SemanticsChecker::Traverse(NodePow& exp) { CheckBinOp(exp); }
void SemanticsChecker::Traverse(NodeLt& exp) { CheckBinOp(exp); }
void SemanticsChecker::Traverse(NodeGt& exp) { CheckBinOp(exp); }
void SemanticsChecker::Traverse(NodeLe& exp) { CheckBinOp(exp); }
void SemanticsChecker::Traverse(NodeGe& exp) { CheckBinOp(exp); }
void SemanticsChecker::Traverse(NodeEq& exp) { CheckBinOp(exp); }
void SemanticsChecker::Traverse(NodeNe& exp) { CheckBinOp(exp); }
void SemanticsChecker::Traverse(NodeAnd& exp) { CheckBinOp(exp); }
void SemanticsChecker::Traverse(NodeOr& exp) { CheckBinOp(exp); }
void SemanticsChecker::Traverse(NodeCat& exp) { CheckBinOp(exp); }
void SemanticsChecker::Traverse(NodeNoParenCallExp& call)
{
    auto def = env_->FindDef(call.name);
    if (!def)
    {
        // 未定義名の使用
        errors_.push_back(undefined_name(call.srcPos, call.name));
    } else if (std::dynamic_pointer_cast<NodeSubDef>(def))
    {
        // サブルーチンの使用
        errors_.push_back(invalid_sub_call(call.srcPos, call.name));
    } else if (std::dynamic_pointer_cast<NodeTaskDef>(def))
    {
        // マイクロスレッドの使用
        errors_.push_back(invalid_task_call(call.srcPos, call.name));
    } else if (GetParamCnt(def) != 0)
    {
        // 引数の数が定義と一致しない
        errors_.push_back(wrong_number_args(call.srcPos, call.name, 0, GetParamCnt(def)));
    }
}
void SemanticsChecker::Traverse(NodeCallExp& call)
{
    auto def = env_->FindDef(call.name);
    if (!def)
    {
        // 未定義名の使用
        errors_.push_back(undefined_name(call.srcPos, call.name));
    } else if (std::dynamic_pointer_cast<NodeSubDef>(def))
    {
        // サブルーチンの使用
        errors_.push_back(invalid_sub_call(call.srcPos, call.name));
    } else if (std::dynamic_pointer_cast<NodeTaskDef>(def))
    {
        // マイクロスレッドの使用
        errors_.push_back(invalid_task_call(call.srcPos, call.name));
    } else if (def->IsVariable())
    {
        // 変数を呼び出している
        errors_.push_back(variable_call(call.srcPos, call.name));
    } else if (GetParamCnt(def) != call.args.size())
    {
        // 引数の数が定義と一致しない
        errors_.push_back(wrong_number_args(call.srcPos, call.name, (int)call.args.size(), GetParamCnt(def)));
    }
    for (auto arg : call.args) { arg->Traverse(*this); }
}
void SemanticsChecker::Traverse(NodeArrayRef& exp)
{
    exp.array->Traverse(*this);
    exp.idx->Traverse(*this);
}
void SemanticsChecker::Traverse(NodeRange& range)
{
    range.start->Traverse(*this);
    range.end->Traverse(*this);
}
void SemanticsChecker::Traverse(NodeArraySlice& exp)
{
    exp.array->Traverse(*this);
    exp.range->Traverse(*this);
}
void SemanticsChecker::Traverse(NodeNop &) {}
void SemanticsChecker::Traverse(NodeLeftVal& left)
{
    auto def = env_->FindDef(left.name);
    if (!def)
    {
        // 未定義名の使用
        errors_.push_back(undefined_name(left.srcPos, left.name));
    } else if (!def->IsVariable())
    {
        // 変数以外へのの代入
        errors_.push_back(invalid_left_value(left.srcPos, left.name));
    }
    for (auto& idx : left.indices) idx->Traverse(*this);
}
void SemanticsChecker::Traverse(NodeAssign& stmt) { CheckAssign(stmt); }
void SemanticsChecker::Traverse(NodeAddAssign& stmt) { CheckAssign(stmt); }
void SemanticsChecker::Traverse(NodeSubAssign& stmt) { CheckAssign(stmt); }
void SemanticsChecker::Traverse(NodeMulAssign& stmt) { CheckAssign(stmt); }
void SemanticsChecker::Traverse(NodeDivAssign& stmt) { CheckAssign(stmt); }
void SemanticsChecker::Traverse(NodeRemAssign& stmt) { CheckAssign(stmt); }
void SemanticsChecker::Traverse(NodePowAssign& stmt) { CheckAssign(stmt); }
void SemanticsChecker::Traverse(NodeCatAssign& stmt) { CheckAssign(stmt); }
void SemanticsChecker::Traverse(NodeCallStmt& call)
{
    auto def = env_->FindDef(call.name);
    if (!def)
    {
        // 未定義名の使用
        errors_.push_back(undefined_name(call.srcPos, call.name));
    } else if (def->IsVariable())
    {
        // 変数を呼び出している
        errors_.push_back(variable_call(call.srcPos, call.name));
    } else if (GetParamCnt(def) != call.args.size())
    {
        // 引数の数が定義と一致しない
        errors_.push_back(wrong_number_args(call.srcPos, call.name, (int)call.args.size(), GetParamCnt(def)));
    }
    for (auto arg : call.args) { arg->Traverse(*this); }
}
void SemanticsChecker::Traverse(NodeReturn& stmt)
{
    if (!IsInFunc())
    {
        errors_.push_back(invalid_return(stmt.srcPos));
    }
    stmt.ret->Traverse(*this);
}
void SemanticsChecker::Traverse(NodeReturnVoid &) {}
void SemanticsChecker::Traverse(NodeYield &) {}
void SemanticsChecker::Traverse(NodeBreak& stmt)
{
    if (!IsInLoop())
    {
        errors_.push_back(invalid_break(stmt.srcPos));
    }
}
void SemanticsChecker::Traverse(NodeSucc& stmt)
{
    stmt.lhs->Traverse(*this);
}
void SemanticsChecker::Traverse(NodePred& stmt)
{
    stmt.lhs->Traverse(*this);
}
void SemanticsChecker::Traverse(NodeVarDecl&) {}
void SemanticsChecker::Traverse(NodeVarInit& stmt)
{
    stmt.rhs->Traverse(*this);
}
void SemanticsChecker::Traverse(NodeProcParam &) {}
void SemanticsChecker::Traverse(NodeLoopParam &) {}
void SemanticsChecker::Traverse(NodeResult &) {}
void SemanticsChecker::Traverse(NodeBlock& blk)
{
    env_ = std::make_shared<Env>(blk.nameTable, env_);
    for (auto& bind : *(blk.nameTable))
    {
        bind.second->Traverse(*this);
    }
    for (auto& stmt : blk.stmts) stmt->Traverse(*this);
    env_ = env_->GetParent();
}
void SemanticsChecker::Traverse(NodeSubDef& def)
{
    funcCtxStack_.push(false);
    loopCtxStack_.push(false);
    def.block->Traverse(*this);
    loopCtxStack_.pop();
    funcCtxStack_.pop();
}
void SemanticsChecker::Traverse(NodeBuiltInSubDef& def)
{
    funcCtxStack_.push(false);
    loopCtxStack_.push(false);
    def.block->Traverse(*this);
    loopCtxStack_.pop();
    funcCtxStack_.pop();
}
void SemanticsChecker::CheckMonoOp(NodeMonoOp& exp)
{
    exp.rhs->Traverse(*this);
}
void SemanticsChecker::CheckBinOp(NodeBinOp& exp)
{
    exp.lhs->Traverse(*this);
    exp.rhs->Traverse(*this);
}
void SemanticsChecker::CheckAssign(NodeAssign& stmt)
{
    stmt.lhs->Traverse(*this);
    stmt.rhs->Traverse(*this);
}
void SemanticsChecker::Traverse(NodeFuncDef& def)
{
    funcCtxStack_.push(true);
    loopCtxStack_.push(false);
    def.block->Traverse(*this);
    loopCtxStack_.pop();
    funcCtxStack_.pop();
}
void SemanticsChecker::Traverse(NodeTaskDef& def)
{
    funcCtxStack_.push(false);
    loopCtxStack_.push(false);
    def.block->Traverse(*this);
    loopCtxStack_.pop();
    funcCtxStack_.pop();
}
void SemanticsChecker::Traverse(NodeBuiltInFunc &) {}
void SemanticsChecker::Traverse(NodeConst &) {}
void SemanticsChecker::Traverse(NodeLocal& stmt)
{
    stmt.block->Traverse(*this);
}
void SemanticsChecker::Traverse(NodeLoop& stmt)
{
    loopCtxStack_.push(true);
    stmt.block->Traverse(*this);
    loopCtxStack_.pop();
}
void SemanticsChecker::Traverse(NodeTimes& stmt)
{
    stmt.cnt->Traverse(*this);
    loopCtxStack_.push(true);
    stmt.block->Traverse(*this);
    loopCtxStack_.pop();
}
void SemanticsChecker::Traverse(NodeWhile& stmt)
{
    stmt.cond->Traverse(*this);
    loopCtxStack_.push(true);
    stmt.block->Traverse(*this);
    loopCtxStack_.pop();
}
void SemanticsChecker::Traverse(NodeAscent& stmt)
{
    stmt.range->Traverse(*this);
    loopCtxStack_.push(true);
    stmt.block->Traverse(*this);
    loopCtxStack_.pop();
}
void SemanticsChecker::Traverse(NodeDescent& stmt)
{
    stmt.range->Traverse(*this);
    loopCtxStack_.push(true);
    stmt.block->Traverse(*this);
    loopCtxStack_.pop();
}
void SemanticsChecker::Traverse(NodeElseIf& elsif)
{
    elsif.cond->Traverse(*this);
    elsif.block->Traverse(*this);
}
void SemanticsChecker::Traverse(NodeIf& stmt)
{
    stmt.cond->Traverse(*this);
    stmt.thenBlock->Traverse(*this);
    for (auto& elsif : stmt.elsifs) elsif->Traverse(*this);
    if (stmt.elseBlock) stmt.elseBlock->Traverse(*this);
}
void SemanticsChecker::Traverse(NodeCase& c)
{
    for (auto& exp : c.exps)
    {
        exp->Traverse(*this);
    }
    c.block->Traverse(*this);
}
void SemanticsChecker::Traverse(NodeAlternative& stmt)
{
    stmt.cond->Traverse(*this);
    for (auto& c : stmt.cases) c->Traverse(*this);
    if (stmt.others) stmt.others->Traverse(*this);
}
void SemanticsChecker::Traverse(NodeHeader &) {}
bool SemanticsChecker::IsInFunc() const
{
    return funcCtxStack_.empty() || funcCtxStack_.top() == true;
}
bool SemanticsChecker::IsInLoop() const
{
    return loopCtxStack_.empty() || loopCtxStack_.top() == true;
}
}