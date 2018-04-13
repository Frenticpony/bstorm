#pragma once

#include <bstorm/env.hpp>
#include <bstorm/node.hpp>
#include <bstorm/logger.hpp>

#include <vector>
#include <stack>

namespace bstorm {
  class SemChecker : public NodeTraverser {
  public:
    std::vector<Log> check(Node& n);
    void traverse(NodeNum&);
    void traverse(NodeChar&);
    void traverse(NodeStr&);
    void traverse(NodeArray&);
    void traverse(NodeNeg&);
    void traverse(NodeNot&);
    void traverse(NodeAbs&);
    void traverse(NodeAdd&);
    void traverse(NodeSub&);
    void traverse(NodeMul&);
    void traverse(NodeDiv&);
    void traverse(NodeRem&);
    void traverse(NodePow&);
    void traverse(NodeLt&);
    void traverse(NodeGt&);
    void traverse(NodeLe&);
    void traverse(NodeGe&);
    void traverse(NodeEq&);
    void traverse(NodeNe&);
    void traverse(NodeAnd&);
    void traverse(NodeOr&);
    void traverse(NodeCat&);
    void traverse(NodeNoParenCallExp&);
    void traverse(NodeCallExp&);
    void traverse(NodeArrayRef&);
    void traverse(NodeRange&);
    void traverse(NodeArraySlice&);
    void traverse(NodeNop&);
    void traverse(NodeLeftVal&);
    void traverse(NodeAssign&);
    void traverse(NodeAddAssign&);
    void traverse(NodeSubAssign&);
    void traverse(NodeMulAssign&);
    void traverse(NodeDivAssign&);
    void traverse(NodeRemAssign&);
    void traverse(NodePowAssign&);
    void traverse(NodeCatAssign&);
    void traverse(NodeCallStmt&);
    void traverse(NodeReturn&);
    void traverse(NodeReturnVoid&);
    void traverse(NodeYield&);
    void traverse(NodeBreak&);
    void traverse(NodeSucc&);
    void traverse(NodePred&);
    void traverse(NodeVarDecl&);
    void traverse(NodeVarInit&);
    void traverse(NodeProcParam&);
    void traverse(NodeLoopParam&);
    void traverse(NodeBlock&);
    void traverse(NodeSubDef&);
    void traverse(NodeBuiltInSubDef&);
    void traverse(NodeFuncDef&);
    void traverse(NodeTaskDef&);
    void traverse(NodeBuiltInFunc&);
    void traverse(NodeConst&);
    void traverse(NodeLocal&);
    void traverse(NodeLoop&);
    void traverse(NodeTimes&);
    void traverse(NodeWhile&);
    void traverse(NodeAscent&);
    void traverse(NodeDescent&);
    void traverse(NodeElseIf&);
    void traverse(NodeIf&);
    void traverse(NodeCase&);
    void traverse(NodeAlternative&);
    void traverse(NodeHeader&);
  protected:
    bool inFunc() const;
    bool inLoop() const;
    std::vector<Log> errors;
    std::stack<bool> funcCtxStack;
    std::stack<bool> loopCtxStack;
    std::shared_ptr<Env> env;
    void checkMonoOp(NodeMonoOp& exp);
    void checkBinOp(NodeBinOp& exp);
    void checkAssign(NodeAssign& stmt);
  };
}