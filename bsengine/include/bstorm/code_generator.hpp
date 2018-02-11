#pragma once

#include <string>
#include <stack>
#include <unordered_map>

#include <bstorm/env.hpp>
#include <bstorm/node.hpp>
#include <bstorm/source_map.hpp>

namespace bstorm {
  class CodeGenerator : public NodeTraverser {
  public:
    CodeGenerator();
    void generate(Node& n) {
      code.clear();
      code.reserve(4096);
      n.traverse(*this);
    }
    SourceMap getSourceMap() const { return srcMap; }
    const char* getCode() const { return code.c_str(); }
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
    void addCode(const std::wstring& s);
    void addCode(const std::string& s);
    void newLine();
    void newLine(const std::shared_ptr<SourcePos>& srcPos);
    void indent();
    void unindent();
    void genMonoOp(const std::string& fname, NodeMonoOp& exp);
    void genBinOp(const std::string& fname, NodeBinOp& exp);
    void genCmpBinOp(const std::string& op, NodeBinOp& exp);
    void genLogBinOp(const std::string& fname, NodeBinOp& exp);
    void genCheckNil(const std::string& name);
    void genProc(std::shared_ptr<NodeDef> def, const std::vector<std::string>& params, NodeBlock& blk);
    void genOpAssign(const std::string& fname, const std::shared_ptr<NodeLeftVal>& left, std::shared_ptr<NodeExp> right);
    void genCopy(std::shared_ptr<NodeExp> exp);
    bool isCopyNeeded(const std::shared_ptr<NodeExp>& exp);
    std::shared_ptr<Env> env;
    std::stack<std::shared_ptr<NodeDef>> procStack;
    std::string code;
    SourceMap srcMap;
    int indentLevel;
    int outputLine;
    bool isLineHead;
  };
}