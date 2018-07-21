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
struct NodeResult;
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
    virtual void Traverse(NodeNum&) = 0;
    virtual void Traverse(NodeChar&) = 0;
    virtual void Traverse(NodeStr&) = 0;
    virtual void Traverse(NodeArray&) = 0;
    virtual void Traverse(NodeNeg&) = 0;
    virtual void Traverse(NodeNot&) = 0;
    virtual void Traverse(NodeAbs&) = 0;
    virtual void Traverse(NodeAdd&) = 0;
    virtual void Traverse(NodeSub&) = 0;
    virtual void Traverse(NodeMul&) = 0;
    virtual void Traverse(NodeDiv&) = 0;
    virtual void Traverse(NodeRem&) = 0;
    virtual void Traverse(NodePow&) = 0;
    virtual void Traverse(NodeLt&) = 0;
    virtual void Traverse(NodeGt&) = 0;
    virtual void Traverse(NodeLe&) = 0;
    virtual void Traverse(NodeGe&) = 0;
    virtual void Traverse(NodeEq&) = 0;
    virtual void Traverse(NodeNe&) = 0;
    virtual void Traverse(NodeAnd&) = 0;
    virtual void Traverse(NodeOr&) = 0;
    virtual void Traverse(NodeCat&) = 0;
    virtual void Traverse(NodeNoParenCallExp&) = 0;
    virtual void Traverse(NodeCallExp&) = 0;
    virtual void Traverse(NodeArrayRef&) = 0;
    virtual void Traverse(NodeRange&) = 0;
    virtual void Traverse(NodeArraySlice&) = 0;
    virtual void Traverse(NodeNop&) = 0;
    virtual void Traverse(NodeLeftVal&) = 0;
    virtual void Traverse(NodeAssign&) = 0;
    virtual void Traverse(NodeAddAssign&) = 0;
    virtual void Traverse(NodeSubAssign&) = 0;
    virtual void Traverse(NodeMulAssign&) = 0;
    virtual void Traverse(NodeDivAssign&) = 0;
    virtual void Traverse(NodeRemAssign&) = 0;
    virtual void Traverse(NodePowAssign&) = 0;
    virtual void Traverse(NodeCatAssign&) = 0;
    virtual void Traverse(NodeCallStmt&) = 0;
    virtual void Traverse(NodeReturn&) = 0;
    virtual void Traverse(NodeReturnVoid&) = 0;
    virtual void Traverse(NodeYield&) = 0;
    virtual void Traverse(NodeBreak&) = 0;
    virtual void Traverse(NodeSucc&) = 0;
    virtual void Traverse(NodePred&) = 0;
    virtual void Traverse(NodeVarDecl&) = 0;
    virtual void Traverse(NodeVarInit&) = 0;
    virtual void Traverse(NodeProcParam&) = 0;
    virtual void Traverse(NodeLoopParam&) = 0;
    virtual void Traverse(NodeResult&) = 0;
    virtual void Traverse(NodeBlock&) = 0;
    virtual void Traverse(NodeSubDef&) = 0;
    virtual void Traverse(NodeBuiltInSubDef&) = 0;
    virtual void Traverse(NodeFuncDef&) = 0;
    virtual void Traverse(NodeTaskDef&) = 0;
    virtual void Traverse(NodeBuiltInFunc&) = 0;
    virtual void Traverse(NodeConst&) = 0;
    virtual void Traverse(NodeLocal&) = 0;
    virtual void Traverse(NodeLoop&) = 0;
    virtual void Traverse(NodeTimes&) = 0;
    virtual void Traverse(NodeWhile&) = 0;
    virtual void Traverse(NodeAscent&) = 0;
    virtual void Traverse(NodeDescent&) = 0;
    virtual void Traverse(NodeElseIf&) = 0;
    virtual void Traverse(NodeIf&) = 0;
    virtual void Traverse(NodeCase&) = 0;
    virtual void Traverse(NodeAlternative&) = 0;
    virtual void Traverse(NodeHeader&) = 0;
};

struct SourcePos;
struct Node
{
    Node() : noSubEffect(false) {}
    virtual ~Node() {};
    virtual void Traverse(NodeTraverser& Traverser) = 0;
    std::shared_ptr<SourcePos> srcPos;
    bool noSubEffect;
};

struct NodeStmt : public Node
{
    NodeStmt() : Node() {}
};

struct NodeExp : public Node
{
    using ExpType = uint64_t;
    static constexpr ExpType T_ANY = 1;
    static constexpr ExpType T_REAL = 3;
    static constexpr ExpType T_CHAR = 5;
    static constexpr ExpType T_BOOL = 7;
    static constexpr ExpType T_NIL = 9;
    static constexpr ExpType T_ARRAY(ExpType t)
    {
        return (t & 0x8000000000000000) ? T_ARRAY(T_ANY) : (t << 1);
    }
    static constexpr ExpType T_STRING = 10;
    static constexpr bool IsArrayType(ExpType t)
    {
        return !(t & 1);
    }
    static constexpr ExpType T_ARRAY_ELEM(ExpType t)
    {
        return IsArrayType(t) ? (t >> 1) : T_ANY;
    }
    static constexpr bool ContainsAnyType(ExpType t)
    {
        return !(t & (t - 1)); // 2の冪乗
    }
    NodeExp() :
        Node(),
        expType(T_ANY),
        copyRequired(true)
    {
    }
    ExpType expType;
    bool copyRequired;
};

struct NodeNum : public NodeExp
{
    NodeNum(std::string&& n) :
        NodeExp(),
        number(std::move(n))
    {
        expType = T_REAL;
        noSubEffect = true;
        copyRequired = false;
    };
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
    std::string number;
};

struct NodeChar : public NodeExp
{
    NodeChar(wchar_t c) :
        NodeExp(),
        c(c)
    {
        expType = T_CHAR;
        noSubEffect = true;
        copyRequired = false;
    };
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
    wchar_t c;
};

struct NodeStr : public NodeExp
{
    NodeStr(std::wstring&& s) :
        NodeExp(),
        str(std::move(s))
    {
        expType = str.empty() ? T_ARRAY(T_ANY) : T_STRING;
        noSubEffect = true;
        copyRequired = false;
    };
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
    std::wstring str;
};

struct NodeArray : public NodeExp
{
    NodeArray(std::vector<std::shared_ptr<NodeExp>>&& es) :
        NodeExp(),
        elems(std::move(es))
    {
    }
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
    std::vector <std::shared_ptr<NodeExp>> elems;
};

struct NodeMonoOp : public NodeExp
{
    NodeMonoOp(const std::shared_ptr<NodeExp>& r) : NodeExp(), rhs(r)
    {
        copyRequired = false;
    }
    std::shared_ptr<NodeExp> rhs;
};

struct NodeNeg : public NodeMonoOp
{
    NodeNeg(const std::shared_ptr<NodeExp>& r) : NodeMonoOp(r) { expType = T_REAL; }
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};

struct NodeNot : public NodeMonoOp
{
    NodeNot(const std::shared_ptr<NodeExp>& r) : NodeMonoOp(r) { expType = T_BOOL; }
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};

struct NodeAbs : public NodeMonoOp
{
    NodeAbs(const std::shared_ptr<NodeExp>& r) : NodeMonoOp(r) { expType = T_REAL; }
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};

struct NodeBinOp : public NodeExp
{
    NodeBinOp(const std::shared_ptr<NodeExp>& l, const std::shared_ptr<NodeExp>& r) : NodeExp(), lhs(l), rhs(r)
    {
        copyRequired = false;
    }
    std::shared_ptr<NodeExp> lhs;
    std::shared_ptr<NodeExp> rhs;
};

struct NodeAdd : public NodeBinOp
{
    NodeAdd(const std::shared_ptr<NodeExp>& l, const std::shared_ptr<NodeExp>& r) : NodeBinOp(l, r)
    {
        if (l->expType == T_REAL && r->expType == T_REAL)
        {
            expType = T_REAL;
        }
    }
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};
struct NodeSub : public NodeBinOp
{
    NodeSub(const std::shared_ptr<NodeExp>& l, const std::shared_ptr<NodeExp>& r) : NodeBinOp(l, r)
    {
        if (l->expType == T_REAL && r->expType == T_REAL)
        {
            expType = T_REAL;
        }
    }
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};
struct NodeMul : public NodeBinOp
{
    NodeMul(const std::shared_ptr<NodeExp>& l, const std::shared_ptr<NodeExp>& r) : NodeBinOp(l, r) { expType = T_REAL; }
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};
struct NodeDiv : public NodeBinOp
{
    NodeDiv(const std::shared_ptr<NodeExp>& l, const std::shared_ptr<NodeExp>& r) : NodeBinOp(l, r) { expType = T_REAL; }
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};
struct NodeRem : public NodeBinOp
{
    NodeRem(const std::shared_ptr<NodeExp>& l, const std::shared_ptr<NodeExp>& r) : NodeBinOp(l, r) { expType = T_REAL; }
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};
struct NodePow : public NodeBinOp
{
    NodePow(const std::shared_ptr<NodeExp>& l, const std::shared_ptr<NodeExp>& r) : NodeBinOp(l, r) { expType = T_REAL; }
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};

struct NodeLt : public NodeBinOp
{
    NodeLt(const std::shared_ptr<NodeExp>& l, const std::shared_ptr<NodeExp>& r) : NodeBinOp(l, r) { expType = T_BOOL; }
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};
struct NodeGt : public NodeBinOp
{
    NodeGt(const std::shared_ptr<NodeExp>& l, const std::shared_ptr<NodeExp>& r) : NodeBinOp(l, r) { expType = T_BOOL; }
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};
struct NodeLe : public NodeBinOp
{
    NodeLe(const std::shared_ptr<NodeExp>& l, const std::shared_ptr<NodeExp>& r) : NodeBinOp(l, r) { expType = T_BOOL; }
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};
struct NodeGe : public NodeBinOp
{
    NodeGe(const std::shared_ptr<NodeExp>& l, const std::shared_ptr<NodeExp>& r) : NodeBinOp(l, r) { expType = T_BOOL; }
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};
struct NodeEq : public NodeBinOp
{
    NodeEq(const std::shared_ptr<NodeExp>& l, const std::shared_ptr<NodeExp>& r) : NodeBinOp(l, r) { expType = T_BOOL; }
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};
struct NodeNe : public NodeBinOp
{
    NodeNe(const std::shared_ptr<NodeExp>& l, const std::shared_ptr<NodeExp>& r) : NodeBinOp(l, r) { expType = T_BOOL; }
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
    NodeArrayRef(const std::shared_ptr<NodeExp>& a, const std::shared_ptr<NodeExp>& i) : NodeExp(), array(a), idx(i)
    {
        copyRequired = false;
    }
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
    NodeArraySlice(const std::shared_ptr<NodeExp>& a, const std::shared_ptr<NodeRange>& r) : NodeExp(), array(a), range(r)
    {
        copyRequired = false;
    }
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
    NodeDef(const std::string& name) : Node(), name(name), convertedName(name), unreachable(true) {}
    virtual bool IsVariable() const = 0;
    std::string name;
    std::string convertedName; // 名前変換用
    bool unreachable; // 到達不可能フラグ
};

struct NodeVarDecl : public NodeDef
{
    NodeVarDecl(const std::string& name) :
        NodeDef(name),
        assignCnt(0u),
        refCnt(0u)
    {
        noSubEffect = true;
    }
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
    virtual bool IsVariable() const override { return true; }
    std::string name;
    uint32_t assignCnt;
    uint32_t refCnt;
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
    NodeProcParam(const std::string& name) : NodeDef(name)
    {
        unreachable = false;
        noSubEffect = true;
    }
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
    virtual bool IsVariable() const override { return true; }
};

struct NodeLoopParam : public NodeDef
{
    NodeLoopParam(const std::string& name) : NodeDef(name)
    {
        noSubEffect = true;
    }
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
    virtual bool IsVariable() const override { return true; }
};

struct NodeResult : public NodeDef
{
    NodeResult() : NodeDef("result")
    {
        noSubEffect = true;
    }
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
    virtual bool IsVariable() const override { return true; }
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
    virtual bool IsVariable() const override { return false; }
    std::shared_ptr<NodeBlock> block;
};

struct NodeBuiltInSubDef : public NodeSubDef
{
    NodeBuiltInSubDef(const std::string& name, const std::shared_ptr<NodeBlock>& blk) : NodeSubDef(name, blk) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
};

struct NodeFuncDef : public NodeDef
{
    NodeFuncDef(const std::string& name, std::vector<std::string>&& ps, const std::shared_ptr<NodeBlock>& blk) : NodeDef(name), params(std::move(ps)), block(blk) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
    virtual bool IsVariable() const override { return false; }
    std::vector<std::string> params;
    std::shared_ptr<NodeBlock> block;
};

struct NodeTaskDef : public NodeDef
{
    NodeTaskDef(const std::string& name, std::vector<std::string>&& ps, std::shared_ptr<NodeBlock>& blk) : NodeDef(name), params(std::move(ps)), block(blk) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
    virtual bool IsVariable() const override { return false; }
    std::vector<std::string> params;
    std::shared_ptr<NodeBlock> block;
};

struct NodeBuiltInFunc : public NodeDef
{
    NodeBuiltInFunc(const std::string& name, uint8_t paramc) : NodeDef(name), paramCnt(paramc) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
    virtual bool IsVariable() const override { return false; }
    uint8_t paramCnt;
};

struct NodeConst : public NodeDef
{
    NodeConst(const std::string& name, const std::string& c) : NodeDef(name), value(c) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
    virtual bool IsVariable() const override { return false; }
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
    NodeHeader(const std::wstring& name, std::vector<std::wstring>&& params) : NodeStmt(), name(name), params(std::move(params)) {}
    void Traverse(NodeTraverser& Traverser) { Traverser.Traverse(*this); }
    std::wstring name;
    std::vector<std::wstring> params;
};
}