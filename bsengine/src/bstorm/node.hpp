#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <cstdint>

namespace bstorm
{
struct NodeNum;
struct NodeChar;
struct NodeStr;
struct NodeArray;
struct NodeNeg;
struct NodeNot;
struct NodeAbs;
struct NodeAdd;
struct NodeSub;
struct NodeMul;
struct NodeDiv;
struct NodeRem;
struct NodePow;
struct NodeLt;
struct NodeGt;
struct NodeLe;
struct NodeGe;
struct NodeEq;
struct NodeNe;
struct NodeAnd;
struct NodeOr;
struct NodeCat;
struct NodeNoParenCallExp;
struct NodeCallExp;
struct NodeArrayRef;
struct NodeRange;
struct NodeArraySlice;
struct NodeNop;
struct NodeLeftVal;
struct NodeAssign;
struct NodeAddAssign;
struct NodeSubAssign;
struct NodeMulAssign;
struct NodeDivAssign;
struct NodeRemAssign;
struct NodePowAssign;
struct NodeCatAssign;
struct NodeCallStmt;
struct NodeReturn;
struct NodeReturnVoid;
struct NodeYield;
struct NodeBreak;
struct NodeSucc;
struct NodePred;
struct NodeVarDecl;
struct NodeVarInit;
struct NodeProcParam;
struct NodeLoopParam;
struct NodeBlock;
struct NodeSubDef;
struct NodeBuiltInSubDef;
struct NodeFuncDef;
struct NodeTaskDef;
struct NodeBuiltInFunc;
struct NodeConst;
struct NodeLocal;
struct NodeLoop;
struct NodeTimes;
struct NodeWhile;
struct NodeAscent;
struct NodeDescent;
struct NodeElseIf;
struct NodeIf;
struct NodeCase;
struct NodeAlternative;
struct NodeHeader;

class NodeTraverser
{
public:
    virtual ~NodeTraverser() {}
    virtual void Traverse(NodeNum&) {};
    virtual void Traverse(NodeChar&) {};
    virtual void Traverse(NodeStr&) {};
    virtual void Traverse(NodeArray&) {};
    virtual void Traverse(NodeNeg&) {};
    virtual void Traverse(NodeNot&) {};
    virtual void Traverse(NodeAbs&) {};
    virtual void Traverse(NodeAdd&) {};
    virtual void Traverse(NodeSub&) {};
    virtual void Traverse(NodeMul&) {};
    virtual void Traverse(NodeDiv&) {};
    virtual void Traverse(NodeRem&) {};
    virtual void Traverse(NodePow&) {};
    virtual void Traverse(NodeLt&) {};
    virtual void Traverse(NodeGt&) {};
    virtual void Traverse(NodeLe&) {};
    virtual void Traverse(NodeGe&) {};
    virtual void Traverse(NodeEq&) {};
    virtual void Traverse(NodeNe&) {};
    virtual void Traverse(NodeAnd&) {};
    virtual void Traverse(NodeOr&) {};
    virtual void Traverse(NodeCat&) {};
    virtual void Traverse(NodeNoParenCallExp&) {};
    virtual void Traverse(NodeCallExp&) {};
    virtual void Traverse(NodeArrayRef&) {};
    virtual void Traverse(NodeRange&) {};
    virtual void Traverse(NodeArraySlice&) {};
    virtual void Traverse(NodeNop&) {};
    virtual void Traverse(NodeLeftVal&) {};
    virtual void Traverse(NodeAssign&) {};
    virtual void Traverse(NodeAddAssign&) {};
    virtual void Traverse(NodeSubAssign&) {};
    virtual void Traverse(NodeMulAssign&) {};
    virtual void Traverse(NodeDivAssign&) {};
    virtual void Traverse(NodeRemAssign&) {};
    virtual void Traverse(NodePowAssign&) {};
    virtual void Traverse(NodeCatAssign&) {};
    virtual void Traverse(NodeCallStmt&) {};
    virtual void Traverse(NodeReturn&) {};
    virtual void Traverse(NodeReturnVoid&) {};
    virtual void Traverse(NodeYield&) {};
    virtual void Traverse(NodeBreak&) {};
    virtual void Traverse(NodeSucc&) {};
    virtual void Traverse(NodePred&) {};
    virtual void Traverse(NodeVarDecl&) {};
    virtual void Traverse(NodeVarInit&) {};
    virtual void Traverse(NodeProcParam&) {};
    virtual void Traverse(NodeLoopParam&) {};
    virtual void Traverse(NodeBlock&) {};
    virtual void Traverse(NodeSubDef&) {};
    virtual void Traverse(NodeBuiltInSubDef&) {};
    virtual void Traverse(NodeFuncDef&) {};
    virtual void Traverse(NodeTaskDef&) {};
    virtual void Traverse(NodeBuiltInFunc&) {};
    virtual void Traverse(NodeConst&) {};
    virtual void Traverse(NodeLocal&) {};
    virtual void Traverse(NodeLoop&) {};
    virtual void Traverse(NodeTimes&) {};
    virtual void Traverse(NodeWhile&) {};
    virtual void Traverse(NodeAscent&) {};
    virtual void Traverse(NodeDescent&) {};
    virtual void Traverse(NodeElseIf&) {};
    virtual void Traverse(NodeIf&) {};
    virtual void Traverse(NodeCase&) {};
    virtual void Traverse(NodeAlternative&) {};
    virtual void Traverse(NodeHeader&) {};
};

struct SourcePos;
struct Node
{
    Node() {}
    virtual ~Node() {};
    virtual void Traverse(NodeTraverser& Traverser) = 0;
    std::shared_ptr<SourcePos> srcPos;
};

struct NodeStmt : public Node
{
    NodeStmt() : Node() {}
};
struct NodeExp : public Node
{
    enum class ExpType : uint8_t
    {
        ANY,
        REAL,
        CHAR,
        BOOL,
    };
    NodeExp() : Node(), expType(ExpType::ANY) {}
    ExpType expType;
};

struct NodeNum : public NodeExp
{
    NodeNum(std::string&& n) : NodeExp(), number(std::move(n)) { expType = ExpType::REAL; };
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
    std::string number;
};

struct NodeChar : public NodeExp
{
    NodeChar(wchar_t c) : NodeExp(), c(c) { expType = ExpType::CHAR; };
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
    wchar_t c;
};

struct NodeStr : public NodeExp
{
    NodeStr(std::wstring&& s) : NodeExp(), str(std::move(s)) {};
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
    std::wstring str;
};

struct NodeArray : public NodeExp
{
    NodeArray(std::vector<std::shared_ptr<NodeExp>>&& es) : NodeExp(), elems(std::move(es)) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
    std::vector <std::shared_ptr<NodeExp>> elems;
};

struct NodeMonoOp : public NodeExp
{
    NodeMonoOp(const std::shared_ptr<NodeExp>& r) : NodeExp(), rhs(r) {}
    std::shared_ptr<NodeExp> rhs;
};

struct NodeNeg : public NodeMonoOp
{
    NodeNeg(const std::shared_ptr<NodeExp>& r) : NodeMonoOp(r) { expType = ExpType::REAL; }
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};

struct NodeNot : public NodeMonoOp
{
    NodeNot(const std::shared_ptr<NodeExp>& r) : NodeMonoOp(r) { expType = ExpType::BOOL; }
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};

struct NodeAbs : public NodeMonoOp
{
    NodeAbs(const std::shared_ptr<NodeExp>& r) : NodeMonoOp(r) { expType = ExpType::REAL; }
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};

struct NodeBinOp : public NodeExp
{
    NodeBinOp(const std::shared_ptr<NodeExp>& l, const std::shared_ptr<NodeExp>& r) : NodeExp(), lhs(l), rhs(r) {}
    std::shared_ptr<NodeExp> lhs;
    std::shared_ptr<NodeExp> rhs;
};

struct NodeAdd : public NodeBinOp
{
    NodeAdd(const std::shared_ptr<NodeExp>& l, const std::shared_ptr<NodeExp>& r) : NodeBinOp(l, r)
    {
        if (l->expType == ExpType::REAL && r->expType == ExpType::REAL)
        {
            expType = ExpType::REAL;
        }
    }
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};
struct NodeSub : public NodeBinOp
{
    NodeSub(const std::shared_ptr<NodeExp>& l, const std::shared_ptr<NodeExp>& r) : NodeBinOp(l, r)
    {
        if (l->expType == ExpType::REAL && r->expType == ExpType::REAL)
        {
            expType = ExpType::REAL;
        }
    }
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};
struct NodeMul : public NodeBinOp
{
    NodeMul(const std::shared_ptr<NodeExp>& l, const std::shared_ptr<NodeExp>& r) : NodeBinOp(l, r) { expType = ExpType::REAL; }
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};
struct NodeDiv : public NodeBinOp
{
    NodeDiv(const std::shared_ptr<NodeExp>& l, const std::shared_ptr<NodeExp>& r) : NodeBinOp(l, r) { expType = ExpType::REAL; }
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};
struct NodeRem : public NodeBinOp
{
    NodeRem(const std::shared_ptr<NodeExp>& l, const std::shared_ptr<NodeExp>& r) : NodeBinOp(l, r) { expType = ExpType::REAL; }
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};
struct NodePow : public NodeBinOp
{
    NodePow(const std::shared_ptr<NodeExp>& l, const std::shared_ptr<NodeExp>& r) : NodeBinOp(l, r) { expType = ExpType::REAL; }
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};

struct NodeLt : public NodeBinOp
{
    NodeLt(const std::shared_ptr<NodeExp>& l, const std::shared_ptr<NodeExp>& r) : NodeBinOp(l, r) { expType = ExpType::BOOL; }
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};
struct NodeGt : public NodeBinOp
{
    NodeGt(const std::shared_ptr<NodeExp>& l, const std::shared_ptr<NodeExp>& r) : NodeBinOp(l, r) { expType = ExpType::BOOL; }
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};
struct NodeLe : public NodeBinOp
{
    NodeLe(const std::shared_ptr<NodeExp>& l, const std::shared_ptr<NodeExp>& r) : NodeBinOp(l, r) { expType = ExpType::BOOL; }
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};
struct NodeGe : public NodeBinOp
{
    NodeGe(const std::shared_ptr<NodeExp>& l, const std::shared_ptr<NodeExp>& r) : NodeBinOp(l, r) { expType = ExpType::BOOL; }
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};
struct NodeEq : public NodeBinOp
{
    NodeEq(const std::shared_ptr<NodeExp>& l, const std::shared_ptr<NodeExp>& r) : NodeBinOp(l, r) { expType = ExpType::BOOL; }
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};
struct NodeNe : public NodeBinOp
{
    NodeNe(const std::shared_ptr<NodeExp>& l, const std::shared_ptr<NodeExp>& r) : NodeBinOp(l, r) { expType = ExpType::BOOL; }
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};

struct NodeAnd : public NodeBinOp
{
    NodeAnd(const std::shared_ptr<NodeExp>& l, const std::shared_ptr<NodeExp>& r) : NodeBinOp(l, r) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};
struct NodeOr : public NodeBinOp
{
    NodeOr(const std::shared_ptr<NodeExp>& l, const std::shared_ptr<NodeExp>& r) : NodeBinOp(l, r) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};

struct NodeCat : public NodeBinOp
{
    NodeCat(const std::shared_ptr<NodeExp>& l, const std::shared_ptr<NodeExp>& r) : NodeBinOp(l, r) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};

struct NodeNoParenCallExp : public NodeExp
{
    NodeNoParenCallExp(const std::string& name) : NodeExp(), name(name) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
    std::string name;
};

struct NodeCallExp : public NodeExp
{
    NodeCallExp(const std::string& name, std::vector<std::shared_ptr<NodeExp>>&& as) :NodeExp(), name(name), args(std::move(as)) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
    std::string name;
    std::vector<std::shared_ptr<NodeExp>> args;
};

struct NodeArrayRef : public NodeExp
{
    NodeArrayRef(const std::shared_ptr<NodeExp>& a, const std::shared_ptr<NodeExp>& i) : NodeExp(), array(a), idx(i) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
    std::shared_ptr<NodeExp> array;
    std::shared_ptr<NodeExp> idx;
};

struct NodeRange : public Node
{
    NodeRange(const std::shared_ptr<NodeExp>& s, const std::shared_ptr<NodeExp>& e) : Node(), start(s), end(e) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
    std::shared_ptr<NodeExp> start;
    std::shared_ptr<NodeExp> end;
};

struct NodeArraySlice : public NodeExp
{
    NodeArraySlice(const std::shared_ptr<NodeExp>& a, const std::shared_ptr<NodeRange>& r) : NodeExp(), array(a), range(r) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
    std::shared_ptr<NodeExp> array;
    std::shared_ptr<NodeRange> range;
};

struct NodeNop : public NodeStmt
{
    NodeNop() : NodeStmt() {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};

struct NodeLeftVal : public Node
{
    NodeLeftVal(const std::string& name, std::vector<std::shared_ptr<NodeExp>>&& is) : Node(), name(name), indices(std::move(is)) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
    std::string name;
    std::vector<std::shared_ptr<NodeExp>> indices;
};

struct NodeAssign : public NodeStmt
{
    NodeAssign(const std::shared_ptr<NodeLeftVal>& l, const std::shared_ptr<NodeExp>& r) : NodeStmt(), lhs(l), rhs(r) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
    std::shared_ptr<NodeLeftVal> lhs;
    std::shared_ptr<NodeExp> rhs;
};

struct NodeAddAssign : public NodeAssign
{
    NodeAddAssign(const std::shared_ptr<NodeLeftVal>& l, const std::shared_ptr<NodeExp>& r) : NodeAssign(l, r) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};
struct NodeSubAssign : public NodeAssign
{
    NodeSubAssign(const std::shared_ptr<NodeLeftVal>& l, const std::shared_ptr<NodeExp>& r) : NodeAssign(l, r) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};
struct NodeMulAssign : public NodeAssign
{
    NodeMulAssign(const std::shared_ptr<NodeLeftVal>& l, const std::shared_ptr<NodeExp>& r) : NodeAssign(l, r) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};
struct NodeDivAssign : public NodeAssign
{
    NodeDivAssign(const std::shared_ptr<NodeLeftVal>& l, const std::shared_ptr<NodeExp>& r) : NodeAssign(l, r) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};
struct NodeRemAssign : public NodeAssign
{
    NodeRemAssign(const std::shared_ptr<NodeLeftVal>& l, const std::shared_ptr<NodeExp>& r) : NodeAssign(l, r) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};
struct NodePowAssign : public NodeAssign
{
    NodePowAssign(const std::shared_ptr<NodeLeftVal>& l, const std::shared_ptr<NodeExp>& r) : NodeAssign(l, r) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};
struct NodeCatAssign : public NodeAssign
{
    NodeCatAssign(const std::shared_ptr<NodeLeftVal>& l, const std::shared_ptr<NodeExp>& r) : NodeAssign(l, r) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};

struct NodeCallStmt : public NodeStmt
{
    NodeCallStmt(const std::string& name, std::vector<std::shared_ptr<NodeExp>>&& as) : NodeStmt(), name(name), args(std::move(as)) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
    std::string name;
    std::vector<std::shared_ptr<NodeExp>> args;
};

struct NodeReturn : public NodeStmt
{
    NodeReturn(const std::shared_ptr<NodeExp>& e) : NodeStmt(), ret(e) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
    std::shared_ptr<NodeExp> ret;
};

struct NodeReturnVoid : public NodeStmt
{
    NodeReturnVoid() : NodeStmt() {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};

struct NodeYield : public NodeStmt
{
    NodeYield() : NodeStmt() {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};

struct NodeBreak : public NodeStmt
{
    NodeBreak() : NodeStmt() {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};

struct NodeSucc : public NodeStmt
{
    NodeSucc(const std::shared_ptr<NodeLeftVal>& l) : NodeStmt(), lhs(l) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
    std::shared_ptr<NodeLeftVal> lhs;
};

struct NodePred : public NodeSucc
{
    NodePred(const std::shared_ptr<NodeLeftVal>& l) : NodeSucc(l) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};

struct NodeDef : public Node
{
    NodeDef(const std::string& name) : Node(), name(name), convertedName(name) {}
    std::string name;
    std::string convertedName; // 名前変換用
};

struct NodeVarDecl : public NodeDef
{
    NodeVarDecl(const std::string& name) : NodeDef(name) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};

struct NodeVarInit : public NodeStmt
{
    NodeVarInit(const std::string& name, const std::shared_ptr<NodeExp>& r) : NodeStmt(), name(name), rhs(r) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
    std::string name;
    std::shared_ptr<NodeExp> rhs;
};

struct NodeProcParam : public NodeDef
{
    NodeProcParam(const std::string& name) : NodeDef(name) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};

struct NodeLoopParam : public NodeDef
{
    NodeLoopParam(const std::string& name) : NodeDef(name) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};

using DefNameTable = std::unordered_map<std::string, std::shared_ptr<NodeDef>>;
struct NodeBlock : public Node
{
    NodeBlock(const std::shared_ptr<DefNameTable>& nameTable, std::vector <std::shared_ptr<NodeStmt>>&& ss) : Node(), nameTable(nameTable), stmts(std::move(ss)) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
    std::shared_ptr<DefNameTable> nameTable; // NOTE: shared_ptrが循環参照してしまうのでEnvは直接持たない
    std::vector<std::shared_ptr<NodeStmt>> stmts;
};

struct NodeSubDef : public NodeDef
{
    NodeSubDef(const std::string& name, const std::shared_ptr<NodeBlock>& blk) : NodeDef(name), block(blk) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
    std::shared_ptr<NodeBlock> block;
};

struct NodeBuiltInSubDef : public NodeSubDef
{
    NodeBuiltInSubDef(const std::string& name, const std::shared_ptr<NodeBlock>& blk) : NodeSubDef(name, blk) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};

struct NodeFuncDef : public NodeDef
{
    NodeFuncDef(const std::string& name, std::vector<std::string>&& ps, const std::shared_ptr<NodeBlock>& blk) : NodeDef(name), params_(std::move(ps)), block(blk) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
    std::vector<std::string> params_;
    std::shared_ptr<NodeBlock> block;
};

struct NodeTaskDef : public NodeDef
{
    NodeTaskDef(const std::string& name, std::vector<std::string>&& ps, std::shared_ptr<NodeBlock>& blk) : NodeDef(name), params_(std::move(ps)), block(blk) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
    std::vector<std::string> params_;
    std::shared_ptr<NodeBlock> block;
};

struct NodeBuiltInFunc : public NodeDef
{
    NodeBuiltInFunc(const std::string& name, uint8_t paramc) : NodeDef(name), paramCnt(paramc) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
    uint8_t paramCnt;
};

struct NodeConst : public NodeDef
{
    NodeConst(const std::string& name, const std::string& c) : NodeDef(name), value(c) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
    std::string value; // UTF-8
};

struct NodeLocal : public NodeStmt
{
    NodeLocal(const std::shared_ptr<NodeBlock>& blk) : NodeStmt(), block(blk) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
    std::shared_ptr<NodeBlock> block;
};

struct NodeLoop : public NodeStmt
{
    NodeLoop(const std::shared_ptr<NodeBlock>& blk) : NodeStmt(), block(blk) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
    std::shared_ptr<NodeBlock> block;
};

struct NodeTimes : public NodeStmt
{
    NodeTimes(const std::shared_ptr<NodeExp>& c, const std::shared_ptr<NodeBlock>& blk) : NodeStmt(), cnt(c), block(blk) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
    std::shared_ptr<NodeExp> cnt;
    std::shared_ptr<NodeBlock> block;
};

struct NodeWhile : public NodeStmt
{
    NodeWhile(const std::shared_ptr<NodeExp>& c, const std::shared_ptr<NodeBlock>& blk) : NodeStmt(), cond(c), block(blk) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
    std::shared_ptr<NodeExp> cond;
    std::shared_ptr<NodeBlock> block;
};

struct NodeAscent : public NodeStmt
{
    NodeAscent(const std::string& p, const std::shared_ptr<NodeRange>& r, const std::shared_ptr<NodeBlock>& blk) : NodeStmt(), param(p), range(r), block(blk) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
    std::string param;
    std::shared_ptr<NodeRange> range;
    std::shared_ptr<NodeBlock> block;
};

struct NodeDescent : public NodeStmt
{
    NodeDescent(const std::string& p, const std::shared_ptr<NodeRange>& r, const std::shared_ptr<NodeBlock>& blk) : NodeStmt(), param(p), range(r), block(blk) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
    std::string param;
    std::shared_ptr<NodeRange> range;
    std::shared_ptr<NodeBlock> block;
};

struct NodeElseIf : public NodeStmt
{
    NodeElseIf(const std::shared_ptr<NodeExp>& c, const std::shared_ptr<NodeBlock>& blk) : NodeStmt(), cond(c), block(blk) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
    std::shared_ptr<NodeExp> cond;
    std::shared_ptr<NodeBlock> block;
};

struct NodeIf : public NodeStmt
{
public:
    NodeIf(const std::shared_ptr<NodeExp>& c, const std::shared_ptr<NodeBlock>& t, std::vector<std::shared_ptr<NodeElseIf>>&& elsifs, const std::shared_ptr<NodeBlock>& e) : NodeStmt(), cond(c), thenBlock(t), elsifs(std::move(elsifs)), elseBlock(e) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
    std::shared_ptr<NodeExp> cond;
    std::shared_ptr<NodeBlock> thenBlock;
    std::vector<std::shared_ptr<NodeElseIf>> elsifs;
    std::shared_ptr<NodeBlock> elseBlock; // Nullable
};

struct NodeCase : public Node
{
    NodeCase(std::vector<std::shared_ptr<NodeExp>>&& es, const std::shared_ptr<NodeBlock>& blk) : Node(), exps(std::move(es)), block(blk) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
    std::vector<std::shared_ptr<NodeExp>> exps;
    std::shared_ptr<NodeBlock> block;
};

struct NodeAlternative : public NodeStmt
{
    NodeAlternative(const std::shared_ptr<NodeExp>& c, std::vector<std::shared_ptr<NodeCase>>&& cs, const std::shared_ptr<NodeBlock>& os) : NodeStmt(), cond(c), cases(std::move(cs)), others(os) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
    std::shared_ptr<NodeExp> cond;
    std::vector<std::shared_ptr<NodeCase>> cases;
    std::shared_ptr<NodeBlock> others; // Nullable
};

struct NodeHeader : public NodeStmt
{
    NodeHeader(const std::wstring& name, std::vector<std::wstring>&& params) : NodeStmt(), name(name), params_(std::move(params)) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
    std::wstring name;
    std::vector<std::wstring> params_;
};
}