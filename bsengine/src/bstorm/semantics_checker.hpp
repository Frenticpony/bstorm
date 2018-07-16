#pragma once

#include <bstorm/node.hpp>
#include <bstorm/logger.hpp>

#include <vector>
#include <stack>

namespace bstorm
{
class Env;
class SemanticsChecker : public NodeTraverser
{
public:
    std::vector<Log> Check(Node& n);
    void Traverse(NodeNum&);
    void Traverse(NodeChar&);
    void Traverse(NodeStr&);
    void Traverse(NodeArray&);
    void Traverse(NodeNeg&);
    void Traverse(NodeNot&);
    void Traverse(NodeAbs&);
    void Traverse(NodeAdd&);
    void Traverse(NodeSub&);
    void Traverse(NodeMul&);
    void Traverse(NodeDiv&);
    void Traverse(NodeRem&);
    void Traverse(NodePow&);
    void Traverse(NodeLt&);
    void Traverse(NodeGt&);
    void Traverse(NodeLe&);
    void Traverse(NodeGe&);
    void Traverse(NodeEq&);
    void Traverse(NodeNe&);
    void Traverse(NodeAnd&);
    void Traverse(NodeOr&);
    void Traverse(NodeCat&);
    void Traverse(NodeNoParenCallExp&);
    void Traverse(NodeCallExp&);
    void Traverse(NodeArrayRef&);
    void Traverse(NodeRange&);
    void Traverse(NodeArraySlice&);
    void Traverse(NodeNop&);
    void Traverse(NodeLeftVal&);
    void Traverse(NodeAssign&);
    void Traverse(NodeAddAssign&);
    void Traverse(NodeSubAssign&);
    void Traverse(NodeMulAssign&);
    void Traverse(NodeDivAssign&);
    void Traverse(NodeRemAssign&);
    void Traverse(NodePowAssign&);
    void Traverse(NodeCatAssign&);
    void Traverse(NodeCallStmt&);
    void Traverse(NodeReturn&);
    void Traverse(NodeReturnVoid&);
    void Traverse(NodeYield&);
    void Traverse(NodeBreak&);
    void Traverse(NodeSucc&);
    void Traverse(NodePred&);
    void Traverse(NodeVarDecl&);
    void Traverse(NodeVarInit&);
    void Traverse(NodeProcParam&);
    void Traverse(NodeLoopParam&);
    void Traverse(NodeBlock&);
    void Traverse(NodeSubDef&);
    void Traverse(NodeBuiltInSubDef&);
    void Traverse(NodeFuncDef&);
    void Traverse(NodeTaskDef&);
    void Traverse(NodeBuiltInFunc&);
    void Traverse(NodeConst&);
    void Traverse(NodeLocal&);
    void Traverse(NodeLoop&);
    void Traverse(NodeTimes&);
    void Traverse(NodeWhile&);
    void Traverse(NodeAscent&);
    void Traverse(NodeDescent&);
    void Traverse(NodeElseIf&);
    void Traverse(NodeIf&);
    void Traverse(NodeCase&);
    void Traverse(NodeAlternative&);
    void Traverse(NodeHeader&);
private:
    bool IsInFunc() const;
    bool IsInLoop() const;
    std::vector<Log> errors_;
    std::stack<bool> funcCtxStack_;
    std::stack<bool> loopCtxStack_;
    std::shared_ptr<Env> env_;
    void CheckMonoOp(NodeMonoOp& exp);
    void CheckBinOp(NodeBinOp& exp);
    void CheckAssign(NodeAssign& stmt);
};
}