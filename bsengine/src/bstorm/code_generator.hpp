#pragma once

#include <bstorm/env.hpp>
#include <bstorm/node.hpp>
#include <bstorm/source_map.hpp>

#include <string>
#include <stack>
#include <unordered_map>

namespace bstorm
{
class CodeGenerator : public NodeTraverser
{
public:
    CodeGenerator();
    void Generate(Node& n);
    SourceMap GetSourceMap() const { return srcMap_; }
    const std::string& GetCode() const { return code_; }
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
    void AddCode(const std::wstring& s);
    void AddCode(const std::string& s);
    void NewLine();
    void NewLine(const std::shared_ptr<SourcePos>& srcPos);
    void Indent();
    void Unindent();
    void GenMonoOp(const std::string& fname, NodeMonoOp& exp);
    void GenBinOp(const std::string& fname, NodeBinOp& exp);
    void GenLogBinOp(const std::string& fname, NodeBinOp& exp);
    void GenCheckNil(const std::string& name);
    void GenProc(std::shared_ptr<NodeDef> def, const std::vector<std::string>& params_, NodeBlock& blk);
    void GenOpAssign(const std::string& fname, const std::shared_ptr<NodeLeftVal>& left, std::shared_ptr<NodeExp> right);
    void GenCopy(std::shared_ptr<NodeExp> exp);
    bool IsCopyNeeded(const std::shared_ptr<NodeExp>& exp);
    std::shared_ptr<Env> env_;
    std::stack<std::shared_ptr<NodeDef>> procStack_;
    std::string code_;
    SourceMap srcMap_;
    int indentLevel_;
    int outputLine_;
    bool isLineHead_;
};
}