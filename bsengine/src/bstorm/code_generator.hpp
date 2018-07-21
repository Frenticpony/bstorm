#pragma once

#include <bstorm/env.hpp>
#include <bstorm/node.hpp>
#include <bstorm/source_map.hpp>
#include <bstorm/nullable_shared_ptr.hpp>

#include <string>
#include <stack>
#include <unordered_map>

namespace bstorm
{
class CodeGenerator : public NodeTraverser
{
public:
    struct Option
    {
        bool embedLocalVarName = true;
        bool deleteUnreachableDefinition = false;
        bool deleteUnneededAssign = false;
    };
    CodeGenerator(const Option& option);
    void Generate(Node& n);
    const SourceMap& GetSourceMap() const { return srcMap_; }
    const std::string& GetCode() const { return code_; }
    void Traverse(NodeNum&) override;
    void Traverse(NodeChar&) override;
    void Traverse(NodeStr&) override;
    void Traverse(NodeArray&) override;
    void Traverse(NodeNeg&) override;
    void Traverse(NodeNot&) override;
    void Traverse(NodeAbs&) override;
    void Traverse(NodeAdd&) override;
    void Traverse(NodeSub&) override;
    void Traverse(NodeMul&) override;
    void Traverse(NodeDiv&) override;
    void Traverse(NodeRem&) override;
    void Traverse(NodePow&) override;
    void Traverse(NodeLt&) override;
    void Traverse(NodeGt&) override;
    void Traverse(NodeLe&) override;
    void Traverse(NodeGe&) override;
    void Traverse(NodeEq&) override;
    void Traverse(NodeNe&) override;
    void Traverse(NodeAnd&) override;
    void Traverse(NodeOr&) override;
    void Traverse(NodeCat&) override;
    void Traverse(NodeNoParenCallExp&) override;
    void Traverse(NodeCallExp&) override;
    void Traverse(NodeArrayRef&) override;
    void Traverse(NodeRange&) override;
    void Traverse(NodeArraySlice&) override;
    void Traverse(NodeNop&) override;
    void Traverse(NodeLeftVal&) override;
    void Traverse(NodeAssign&) override;
    void Traverse(NodeAddAssign&) override;
    void Traverse(NodeSubAssign&) override;
    void Traverse(NodeMulAssign&) override;
    void Traverse(NodeDivAssign&) override;
    void Traverse(NodeRemAssign&) override;
    void Traverse(NodePowAssign&) override;
    void Traverse(NodeCatAssign&) override;
    void Traverse(NodeCallStmt&) override;
    void Traverse(NodeReturn&) override;
    void Traverse(NodeReturnVoid&) override;
    void Traverse(NodeYield&) override;
    void Traverse(NodeBreak&) override;
    void Traverse(NodeSucc&) override;
    void Traverse(NodePred&) override;
    void Traverse(NodeVarDecl&) override;
    void Traverse(NodeVarInit&) override;
    void Traverse(NodeProcParam&) override;
    void Traverse(NodeLoopParam&) override;
    void Traverse(NodeResult&) override;
    void Traverse(NodeBlock&) override;
    void Traverse(NodeSubDef&) override;
    void Traverse(NodeBuiltInSubDef&) override;
    void Traverse(NodeFuncDef&) override;
    void Traverse(NodeTaskDef&) override;
    void Traverse(NodeBuiltInFunc&) override;
    void Traverse(NodeConst&) override;
    void Traverse(NodeLocal&) override;
    void Traverse(NodeLoop&) override;
    void Traverse(NodeTimes&) override;
    void Traverse(NodeWhile&) override;
    void Traverse(NodeAscent&) override;
    void Traverse(NodeDescent&) override;
    void Traverse(NodeElseIf&) override;
    void Traverse(NodeIf&) override;
    void Traverse(NodeCase&) override;
    void Traverse(NodeAlternative&) override;
    void Traverse(NodeHeader&) override;
private:
    void AddCode(const std::wstring& s);
    void AddCode(const std::string& s);
    void NewLine();
    void NewLine(const std::shared_ptr<SourcePos>& srcPos);
    void Indent();
    void Unindent();
    void GenMonoOp(const std::string& fname, NodeMonoOp& exp);
    void GenBinOp(const std::string& fname, NodeBinOp& exp);
    void GenArithBinOp(const std::string& fname, const std::string& op, NodeBinOp& exp);
    void GenLogBinOp(const std::string& fname, NodeBinOp& exp);
    void GenNilCheck(const std::string& name);
    void GenProc(const std::shared_ptr<NodeDef>& def, const std::vector<std::string>& params_, NodeBlock& blk);
    void GenOpAssign(const std::string& fname, const std::shared_ptr<NodeLeftVal>& left, const NullableSharedPtr<NodeExp>& right);
    void GenCopy(std::shared_ptr<NodeExp>& exp);
    void GenCondition(std::shared_ptr<NodeExp>& exp);
    bool IsCopyNeeded(const std::shared_ptr<NodeExp>& exp);
    std::shared_ptr<Env> env_;
    std::stack<std::shared_ptr<NodeDef>> procStack_;
    std::string code_;
    SourceMap srcMap_;
    int indentLevel_;
    int outputLine_;
    bool isLineHead_;
    const Option option_;
};
}