#include <bstorm/code_analyzer.hpp>

#include <bstorm/env.hpp>
#include <bstorm/script_entry_routine_names.hpp>

namespace bstorm
{
// NOTE: “ž’B‰Â”\‚È•”•ª‚¾‚¯‰ðÍ

void CodeAnalyzer::Analyze(Node & n)
{
    env_ = nullptr;
    n.Traverse(*this);
}
void CodeAnalyzer::Traverse(NodeNum &) {}
void CodeAnalyzer::Traverse(NodeChar &) {}
void CodeAnalyzer::Traverse(NodeStr &) {}
void CodeAnalyzer::Traverse(NodeArray & array)
{
    for (auto& e : array.elems)
    {
        e->Traverse(*this);
    }
}
void CodeAnalyzer::Traverse(NodeNeg& exp) { AnalyzeMonoOp(exp); }
void CodeAnalyzer::Traverse(NodeNot& exp) { AnalyzeMonoOp(exp); }
void CodeAnalyzer::Traverse(NodeAbs& exp) { AnalyzeMonoOp(exp); }
void CodeAnalyzer::Traverse(NodeAdd& exp) { AnalyzeBinOp(exp); }
void CodeAnalyzer::Traverse(NodeSub& exp) { AnalyzeBinOp(exp); }
void CodeAnalyzer::Traverse(NodeMul& exp) { AnalyzeBinOp(exp); }
void CodeAnalyzer::Traverse(NodeDiv& exp) { AnalyzeBinOp(exp); }
void CodeAnalyzer::Traverse(NodeRem& exp) { AnalyzeBinOp(exp); }
void CodeAnalyzer::Traverse(NodePow& exp) { AnalyzeBinOp(exp); }
void CodeAnalyzer::Traverse(NodeLt& exp) { AnalyzeBinOp(exp); }
void CodeAnalyzer::Traverse(NodeGt& exp) { AnalyzeBinOp(exp); }
void CodeAnalyzer::Traverse(NodeLe& exp) { AnalyzeBinOp(exp); }
void CodeAnalyzer::Traverse(NodeGe& exp) { AnalyzeBinOp(exp); }
void CodeAnalyzer::Traverse(NodeEq& exp) { AnalyzeBinOp(exp); }
void CodeAnalyzer::Traverse(NodeNe& exp) { AnalyzeBinOp(exp); }
void CodeAnalyzer::Traverse(NodeAnd& exp) { AnalyzeBinOp(exp); }
void CodeAnalyzer::Traverse(NodeOr& exp) { AnalyzeBinOp(exp); }
void CodeAnalyzer::Traverse(NodeCat& exp) { AnalyzeBinOp(exp); }
void CodeAnalyzer::Traverse(NodeNoParenCallExp & call)
{
    AnalyzeDef(call.name);
}
void CodeAnalyzer::Traverse(NodeCallExp & call)
{
    AnalyzeDef(call.name);
    for (auto& arg : call.args)
    {
        arg->Traverse(*this);
    }
}
void CodeAnalyzer::Traverse(NodeArrayRef& exp)
{
    exp.array->Traverse(*this);
    exp.idx->Traverse(*this);
}
void CodeAnalyzer::Traverse(NodeRange& range)
{
    range.start->Traverse(*this);
    range.end->Traverse(*this);
}
void CodeAnalyzer::Traverse(NodeArraySlice& exp)
{
    exp.array->Traverse(*this);
    exp.range->Traverse(*this);
}
void CodeAnalyzer::Traverse(NodeNop &) {}
void CodeAnalyzer::Traverse(NodeLeftVal & left)
{
    AnalyzeDef(left.name);
    for (auto& idx : left.indices)
    {
        idx->Traverse(*this);
    }
}
void CodeAnalyzer::Traverse(NodeAssign& stmt) { AnalyzeAssign(stmt); }
void CodeAnalyzer::Traverse(NodeAddAssign& stmt) { AnalyzeAssign(stmt); }
void CodeAnalyzer::Traverse(NodeSubAssign& stmt) { AnalyzeAssign(stmt); }
void CodeAnalyzer::Traverse(NodeMulAssign& stmt) { AnalyzeAssign(stmt); }
void CodeAnalyzer::Traverse(NodeDivAssign& stmt) { AnalyzeAssign(stmt); }
void CodeAnalyzer::Traverse(NodeRemAssign& stmt) { AnalyzeAssign(stmt); }
void CodeAnalyzer::Traverse(NodePowAssign& stmt) { AnalyzeAssign(stmt); }
void CodeAnalyzer::Traverse(NodeCatAssign& stmt) { AnalyzeAssign(stmt); }
void CodeAnalyzer::Traverse(NodeBlock & blk)
{
    env_ = std::make_shared<Env>(blk.nameTable, env_);

    if (env_->IsRoot())
    {
        for (auto&& name : SCRIPT_ENTRY_ROUTINE_NAMES)
        {
            AnalyzeDef(name);
        }
    }

    for (const auto& stmt : blk.stmts)
    {
        stmt->Traverse(*this);
    }

    env_ = env_->GetParent();
}
void CodeAnalyzer::Traverse(NodeSubDef & def)
{
    def.block->Traverse(*this);
}
void CodeAnalyzer::Traverse(NodeBuiltInSubDef & def)
{
    def.block->Traverse(*this);
}
void CodeAnalyzer::Traverse(NodeFuncDef & def)
{
    def.block->Traverse(*this);
}
void CodeAnalyzer::Traverse(NodeTaskDef & def)
{
    def.block->Traverse(*this);
}
void CodeAnalyzer::Traverse(NodeBuiltInFunc &) {}
void CodeAnalyzer::Traverse(NodeConst &) {}
void CodeAnalyzer::Traverse(NodeLocal & stmt)
{
    stmt.block->Traverse(*this);
}
void CodeAnalyzer::Traverse(NodeLoop & stmt)
{
    stmt.block->Traverse(*this);
}
void CodeAnalyzer::Traverse(NodeTimes & stmt)
{
    stmt.cnt->Traverse(*this);
    stmt.block->Traverse(*this);
}
void CodeAnalyzer::Traverse(NodeWhile & stmt)
{
    stmt.cond->Traverse(*this);
    stmt.block->Traverse(*this);
}
void CodeAnalyzer::Traverse(NodeAscent & stmt)
{
    stmt.range->Traverse(*this);
    stmt.block->Traverse(*this);
}
void CodeAnalyzer::Traverse(NodeDescent & stmt)
{
    stmt.range->Traverse(*this);
    stmt.block->Traverse(*this);
}

void CodeAnalyzer::Traverse(NodeElseIf& elsif)
{
    elsif.cond->Traverse(*this);
    elsif.block->Traverse(*this);
}
void CodeAnalyzer::Traverse(NodeIf& stmt)
{
    stmt.cond->Traverse(*this);
    stmt.thenBlock->Traverse(*this);
    for (auto& elsif : stmt.elsifs) elsif->Traverse(*this);
    if (stmt.elseBlock) stmt.elseBlock->Traverse(*this);
}
void CodeAnalyzer::Traverse(NodeCase& c)
{
    for (auto& exp : c.exps)
    {
        exp->Traverse(*this);
    }
    c.block->Traverse(*this);
}
void CodeAnalyzer::Traverse(NodeAlternative& stmt)
{
    stmt.cond->Traverse(*this);
    for (auto& c : stmt.cases) c->Traverse(*this);
    if (stmt.others) stmt.others->Traverse(*this);
}
void CodeAnalyzer::Traverse(NodeCallStmt & call)
{
    AnalyzeDef(call.name);
    for (auto& arg : call.args)
    {
        arg->Traverse(*this);
    }
}
void CodeAnalyzer::Traverse(NodeReturn & stmt)
{
    stmt.ret->Traverse(*this);
}
void CodeAnalyzer::Traverse(NodeReturnVoid &) {}
void CodeAnalyzer::Traverse(NodeYield &) {}
void CodeAnalyzer::Traverse(NodeBreak &) {}
void CodeAnalyzer::Traverse(NodeSucc& stmt)
{
    stmt.lhs->Traverse(*this);
}
void CodeAnalyzer::Traverse(NodePred& stmt)
{
    stmt.lhs->Traverse(*this);
}
void CodeAnalyzer::Traverse(NodeVarDecl&) {}

void CodeAnalyzer::Traverse(NodeVarInit& stmt)
{
    AnalyzeDef(stmt.name);
    stmt.rhs->Traverse(*this);
}
void CodeAnalyzer::Traverse(NodeProcParam &) {}
void CodeAnalyzer::Traverse(NodeLoopParam &) {}

void CodeAnalyzer::Traverse(NodeResult &) {}

void CodeAnalyzer::Traverse(NodeHeader &) {}

void CodeAnalyzer::AnalyzeDef(const std::string& name)
{
    auto defEnv = env_;

    while (defEnv && defEnv->GetCurrentBlockNameTable()->count(name) == 0)
    {
        defEnv = defEnv->GetParent();
    }

    if (defEnv)
    {
        auto& def = (*(defEnv->GetCurrentBlockNameTable()))[name];
        if (def->unreachable)
        {
            auto prevEnv = env_;
            env_ = defEnv;
            def->unreachable = false;
            def->Traverse(*this);
            env_ = prevEnv;
        }
    }
}
void CodeAnalyzer::AnalyzeMonoOp(NodeMonoOp & exp)
{
    exp.rhs->Traverse(*this);
}
void CodeAnalyzer::AnalyzeBinOp(NodeBinOp & exp)
{
    exp.lhs->Traverse(*this);
    exp.rhs->Traverse(*this);
}
void CodeAnalyzer::AnalyzeAssign(NodeAssign & stmt)
{
    stmt.lhs->Traverse(*this);
    stmt.rhs->Traverse(*this);
}
}