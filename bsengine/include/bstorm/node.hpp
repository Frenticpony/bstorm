#pragma once

#include <string>
#include <vector>
#include <memory>

#include <bstorm/env.hpp>

namespace bstorm {
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

  class NodeTraverser {
  public:
    virtual ~NodeTraverser() {}
    virtual void traverse(NodeNum&) {};
    virtual void traverse(NodeChar&) {};
    virtual void traverse(NodeStr&) {};
    virtual void traverse(NodeArray&) {};
    virtual void traverse(NodeNeg&) {};
    virtual void traverse(NodeNot&) {};
    virtual void traverse(NodeAbs&) {};
    virtual void traverse(NodeAdd&) {};
    virtual void traverse(NodeSub&) {};
    virtual void traverse(NodeMul&) {};
    virtual void traverse(NodeDiv&) {};
    virtual void traverse(NodeRem&) {};
    virtual void traverse(NodePow&) {};
    virtual void traverse(NodeLt&) {};
    virtual void traverse(NodeGt&) {};
    virtual void traverse(NodeLe&) {};
    virtual void traverse(NodeGe&) {};
    virtual void traverse(NodeEq&) {};
    virtual void traverse(NodeNe&) {};
    virtual void traverse(NodeAnd&) {};
    virtual void traverse(NodeOr&) {};
    virtual void traverse(NodeCat&) {};
    virtual void traverse(NodeNoParenCallExp&) {};
    virtual void traverse(NodeCallExp&) {};
    virtual void traverse(NodeArrayRef&) {};
    virtual void traverse(NodeRange&) {};
    virtual void traverse(NodeArraySlice&) {};
    virtual void traverse(NodeNop&) {};
    virtual void traverse(NodeLeftVal&) {};
    virtual void traverse(NodeAssign&) {};
    virtual void traverse(NodeAddAssign&) {};
    virtual void traverse(NodeSubAssign&) {};
    virtual void traverse(NodeMulAssign&) {};
    virtual void traverse(NodeDivAssign&) {};
    virtual void traverse(NodeRemAssign&) {};
    virtual void traverse(NodePowAssign&) {};
    virtual void traverse(NodeCatAssign&) {};
    virtual void traverse(NodeCallStmt&) {};
    virtual void traverse(NodeReturn&) {};
    virtual void traverse(NodeReturnVoid&) {};
    virtual void traverse(NodeYield&) {};
    virtual void traverse(NodeBreak&) {};
    virtual void traverse(NodeSucc&) {};
    virtual void traverse(NodePred&) {};
    virtual void traverse(NodeVarDecl&) {};
    virtual void traverse(NodeVarInit&) {};
    virtual void traverse(NodeProcParam&) {};
    virtual void traverse(NodeLoopParam&) {};
    virtual void traverse(NodeBlock&) {};
    virtual void traverse(NodeSubDef&) {};
    virtual void traverse(NodeBuiltInSubDef&) {};
    virtual void traverse(NodeFuncDef&) {};
    virtual void traverse(NodeTaskDef&) {};
    virtual void traverse(NodeBuiltInFunc&) {};
    virtual void traverse(NodeConst&) {};
    virtual void traverse(NodeLocal&) {};
    virtual void traverse(NodeLoop&) {};
    virtual void traverse(NodeTimes&) {};
    virtual void traverse(NodeWhile&) {};
    virtual void traverse(NodeAscent&) {};
    virtual void traverse(NodeDescent&) {};
    virtual void traverse(NodeElseIf&) {};
    virtual void traverse(NodeIf&) {};
    virtual void traverse(NodeCase&) {};
    virtual void traverse(NodeAlternative&) {};
    virtual void traverse(NodeHeader&) {};
  };

  struct SourcePos;
  struct Node {
    Node() {}
    virtual ~Node() {};
    virtual void traverse(NodeTraverser& traverser) = 0;
    std::shared_ptr<SourcePos> srcPos;
  };

  struct NodeStmt : public Node {
    NodeStmt() : Node() {}
  };
  struct NodeExp : public Node {
    NodeExp() : Node() {}
  };

  struct NodeNum : public NodeExp {
    NodeNum(const std::string& n) : NodeExp(), number(n) {};
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
    std::string number;
  };

  struct NodeChar : public NodeExp {
    NodeChar(wchar_t c) : NodeExp(), c(c) {};
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
    wchar_t c;
  };

  struct NodeStr : public NodeExp {
    NodeStr(const std::wstring& s) : NodeExp(), str(s) {};
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
    std::wstring str;
  };

  struct NodeArray : public NodeExp {
    NodeArray(const std::vector<std::shared_ptr<NodeExp>>& es) : NodeExp(), elems(es) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
    std::vector <std::shared_ptr<NodeExp>> elems;
  };

  struct NodeMonoOp : public NodeExp {
    NodeMonoOp(const std::shared_ptr<NodeExp>& r) : NodeExp(), rhs(r) {}
    std::shared_ptr<NodeExp> rhs;
  };

  struct NodeNeg : public NodeMonoOp {
    NodeNeg(const std::shared_ptr<NodeExp>& r) : NodeMonoOp(r) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
  };

  struct NodeNot : public NodeMonoOp {
    NodeNot(const std::shared_ptr<NodeExp>& r) : NodeMonoOp(r) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
  };

  struct NodeAbs : public NodeMonoOp {
    NodeAbs(const std::shared_ptr<NodeExp>& r) : NodeMonoOp(r) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
  };

  struct NodeBinOp : public NodeExp {
    NodeBinOp(const std::shared_ptr<NodeExp>& l, const std::shared_ptr<NodeExp>& r) : NodeExp(), lhs(l), rhs(r) {}
    std::shared_ptr<NodeExp> lhs;
    std::shared_ptr<NodeExp> rhs;
  };

  struct NodeAdd : public NodeBinOp {
    NodeAdd(const std::shared_ptr<NodeExp>& l, const std::shared_ptr<NodeExp>& r) : NodeBinOp(l, r) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
  };
  struct NodeSub : public NodeBinOp {
    NodeSub(const std::shared_ptr<NodeExp>& l, const std::shared_ptr<NodeExp>& r) : NodeBinOp(l, r) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
  };
  struct NodeMul : public NodeBinOp {
    NodeMul(const std::shared_ptr<NodeExp>& l, const std::shared_ptr<NodeExp>& r) : NodeBinOp(l, r) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
  };
  struct NodeDiv : public NodeBinOp {
    NodeDiv(const std::shared_ptr<NodeExp>& l, const std::shared_ptr<NodeExp>& r) : NodeBinOp(l, r) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
  };
  struct NodeRem : public NodeBinOp {
    NodeRem(const std::shared_ptr<NodeExp>& l, const std::shared_ptr<NodeExp>& r) : NodeBinOp(l, r) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
  };
  struct NodePow : public NodeBinOp {
    NodePow(const std::shared_ptr<NodeExp>& l, const std::shared_ptr<NodeExp>& r) : NodeBinOp(l, r) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
  };

  struct NodeLt : public NodeBinOp {
    NodeLt(const std::shared_ptr<NodeExp>& l, const std::shared_ptr<NodeExp>& r) : NodeBinOp(l, r) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
  };
  struct NodeGt : public NodeBinOp {
    NodeGt(const std::shared_ptr<NodeExp>& l, const std::shared_ptr<NodeExp>& r) : NodeBinOp(l, r) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
  };
  struct NodeLe : public NodeBinOp {
    NodeLe(const std::shared_ptr<NodeExp>& l, const std::shared_ptr<NodeExp>& r) : NodeBinOp(l, r) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
  };
  struct NodeGe : public NodeBinOp {
    NodeGe(const std::shared_ptr<NodeExp>& l, const std::shared_ptr<NodeExp>& r) : NodeBinOp(l, r) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
  };
  struct NodeEq : public NodeBinOp {
    NodeEq(const std::shared_ptr<NodeExp>& l, const std::shared_ptr<NodeExp>& r) : NodeBinOp(l, r) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
  };
  struct NodeNe : public NodeBinOp {
    NodeNe(const std::shared_ptr<NodeExp>& l, const std::shared_ptr<NodeExp>& r) : NodeBinOp(l, r) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
  };

  struct NodeAnd : public NodeBinOp {
    NodeAnd(const std::shared_ptr<NodeExp>& l, const std::shared_ptr<NodeExp>& r) : NodeBinOp(l, r) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
  };
  struct NodeOr : public NodeBinOp {
    NodeOr(const std::shared_ptr<NodeExp>& l, const std::shared_ptr<NodeExp>& r) : NodeBinOp(l, r) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
  };

  struct NodeCat : public NodeBinOp {
    NodeCat(const std::shared_ptr<NodeExp>& l, const std::shared_ptr<NodeExp>& r) : NodeBinOp(l, r) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
  };

  struct NodeNoParenCallExp : public NodeExp {
    NodeNoParenCallExp(const std::string& name) : NodeExp(), name(name) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
    std::string name;
  };

  struct NodeCallExp : public NodeExp {
    NodeCallExp(const std::string& name, const std::vector<std::shared_ptr<NodeExp>>& as) :NodeExp(),  name(name), args(as) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
    std::string name;
    std::vector<std::shared_ptr<NodeExp>> args;
  };

  struct NodeArrayRef : public NodeExp {
    NodeArrayRef(const std::shared_ptr<NodeExp>& a, const std::shared_ptr<NodeExp>& i) : NodeExp(), array(a), idx(i) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
    std::shared_ptr<NodeExp> array;
    std::shared_ptr<NodeExp> idx;
  };

  struct NodeRange : public Node {
    NodeRange(const std::shared_ptr<NodeExp>& s, const std::shared_ptr<NodeExp>& e) : Node(), start(s), end(e) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
    std::shared_ptr<NodeExp> start;
    std::shared_ptr<NodeExp> end;
  };

  struct NodeArraySlice : public NodeExp {
    NodeArraySlice(const std::shared_ptr<NodeExp>& a, const std::shared_ptr<NodeRange>& r) : NodeExp(), array(a), range(r) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
    std::shared_ptr<NodeExp> array;
    std::shared_ptr<NodeRange> range;
  };

  struct NodeNop : public NodeStmt {
    NodeNop() : NodeStmt() {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
  };

  struct NodeLeftVal : public Node {
    NodeLeftVal(const std::string& name, const std::vector<std::shared_ptr<NodeExp>>& is) : Node(), name(name), indices(is) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
    std::string name;
    std::vector<std::shared_ptr<NodeExp>> indices;
  };

  struct NodeAssign : public NodeStmt {
    NodeAssign(const std::shared_ptr<NodeLeftVal>& l, const std::shared_ptr<NodeExp>& r) : NodeStmt(), lhs(l), rhs(r) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
    std::shared_ptr<NodeLeftVal> lhs;
    std::shared_ptr<NodeExp> rhs;
  };

  struct NodeAddAssign : public NodeAssign {
    NodeAddAssign(const std::shared_ptr<NodeLeftVal>& l, const std::shared_ptr<NodeExp>& r) : NodeAssign(l, r) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
  };
  struct NodeSubAssign : public NodeAssign {
    NodeSubAssign(const std::shared_ptr<NodeLeftVal>& l, const std::shared_ptr<NodeExp>& r) : NodeAssign(l, r) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
  };
  struct NodeMulAssign : public NodeAssign {
    NodeMulAssign(const std::shared_ptr<NodeLeftVal>& l, const std::shared_ptr<NodeExp>& r) : NodeAssign(l, r) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
  };
  struct NodeDivAssign : public NodeAssign {
    NodeDivAssign(const std::shared_ptr<NodeLeftVal>& l, const std::shared_ptr<NodeExp>& r) : NodeAssign(l, r) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
  };
  struct NodeRemAssign : public NodeAssign {
    NodeRemAssign(const std::shared_ptr<NodeLeftVal>& l, const std::shared_ptr<NodeExp>& r) : NodeAssign(l, r) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
  };
  struct NodePowAssign : public NodeAssign {
    NodePowAssign(const std::shared_ptr<NodeLeftVal>& l, const std::shared_ptr<NodeExp>& r) : NodeAssign(l, r) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
  };
  struct NodeCatAssign : public NodeAssign {
    NodeCatAssign(const std::shared_ptr<NodeLeftVal>& l, const std::shared_ptr<NodeExp>& r) : NodeAssign(l, r) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
  };

  struct NodeCallStmt : public NodeStmt {
    NodeCallStmt(const std::string& name, const std::vector<std::shared_ptr<NodeExp>>& as) : NodeStmt(), name(name), args(as) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
    std::string name;
    std::vector<std::shared_ptr<NodeExp>> args;
  };

  struct NodeReturn : public NodeStmt {
    NodeReturn(const std::shared_ptr<NodeExp>& e) : NodeStmt(), ret(e) { }
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
    std::shared_ptr<NodeExp> ret;
  };

  struct NodeReturnVoid : public NodeStmt {
    NodeReturnVoid() : NodeStmt() {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
  };

  struct NodeYield : public NodeStmt {
    NodeYield() : NodeStmt() {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
  };

  struct NodeBreak : public NodeStmt {
    NodeBreak() : NodeStmt() {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
  };

  struct NodeSucc : public NodeStmt {
    NodeSucc(const std::shared_ptr<NodeLeftVal>& l) : NodeStmt(), lhs(l) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
    std::shared_ptr<NodeLeftVal> lhs;
  };

  struct NodePred : public NodeSucc {
    NodePred(const std::shared_ptr<NodeLeftVal>& l) : NodeSucc(l) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
  };

  struct NodeDef : public Node {
    NodeDef(const std::string& name) : Node(), name(name) {}
    std::string name;
  };

  struct NodeVarDecl : public NodeDef {
    NodeVarDecl(const std::string& name) : NodeDef(name) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
  };

  struct NodeVarInit : public NodeStmt {
    NodeVarInit(const std::string& name, const std::shared_ptr<NodeExp>& r) : NodeStmt(), name(name), rhs(r) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
    std::string name;
    std::shared_ptr<NodeExp> rhs;
  };

  struct NodeProcParam : public NodeDef {
    NodeProcParam(const std::string& name) : NodeDef(name) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
  };

  struct NodeLoopParam : public NodeDef {
    NodeLoopParam(const std::string& name) : NodeDef(name) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
  };

  struct NodeBlock : public Node {
    NodeBlock(const NameTable& t, const std::vector <std::shared_ptr<NodeStmt>>& ss) : Node(), table(t), stmts(ss) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
    NameTable table;
    std::vector <std::shared_ptr<NodeStmt>> stmts;
  };

  struct NodeSubDef : public NodeDef {
    NodeSubDef(const std::string& name, const std::shared_ptr<NodeBlock>& blk) : NodeDef(name), block(blk) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
    std::shared_ptr<NodeBlock> block;
  };

  struct NodeBuiltInSubDef : public NodeSubDef {
    NodeBuiltInSubDef(const std::string& name, const std::shared_ptr<NodeBlock>& blk) : NodeSubDef(name, blk) {  }
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
  };

  struct NodeFuncDef : public NodeDef {
    NodeFuncDef(const std::string& name, const std::vector<std::string>& ps, const std::shared_ptr<NodeBlock>& blk) : NodeDef(name), params(ps), block(blk) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
    std::vector<std::string> params;
    std::shared_ptr<NodeBlock> block;
  };

  struct NodeTaskDef : public NodeDef {
    NodeTaskDef(const std::string& name, const std::vector<std::string>& ps, std::shared_ptr<NodeBlock>& blk) : NodeDef(name), params(ps), block(blk) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
    std::vector<std::string> params;
    std::shared_ptr<NodeBlock> block;
  };

  struct NodeBuiltInFunc : public NodeDef {
    NodeBuiltInFunc(const std::string& name, int paramc, void* func) : NodeDef(name), paramCnt(paramc), funcPointer(func) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
    int paramCnt;
    void* funcPointer;
  };

  struct NodeConst : public NodeDef {
    NodeConst(const std::string& name, const std::wstring& c) : NodeDef(name), value(c) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
    std::wstring value;
  };

  struct NodeLocal : public NodeStmt {
    NodeLocal(const std::shared_ptr<NodeBlock>& blk) : NodeStmt(), block(blk) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
    std::shared_ptr<NodeBlock> block;
  };

  struct NodeLoop : public NodeStmt {
    NodeLoop(const std::shared_ptr<NodeBlock>& blk) : NodeStmt(), block(blk) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
    std::shared_ptr<NodeBlock> block;
  };

  struct NodeTimes : public NodeStmt {
    NodeTimes(const std::shared_ptr<NodeExp>& c, const std::shared_ptr<NodeBlock>& blk) : NodeStmt(), cnt(c), block(blk) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
    std::shared_ptr<NodeExp> cnt;
    std::shared_ptr<NodeBlock> block;
  };

  struct NodeWhile : public NodeStmt {
    NodeWhile(const std::shared_ptr<NodeExp>& c, const std::shared_ptr<NodeBlock>& blk) : NodeStmt(), cond(c), block(blk) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
    std::shared_ptr<NodeExp> cond;
    std::shared_ptr<NodeBlock> block;
  };

  struct NodeAscent : public NodeStmt {
    NodeAscent(const std::string& p, const std::shared_ptr<NodeRange>& r, const std::shared_ptr<NodeBlock>& blk) : NodeStmt(), param(p), range(r), block(blk) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
    std::string param;
    std::shared_ptr<NodeRange> range;
    std::shared_ptr<NodeBlock> block;
  };

  struct NodeDescent : public NodeStmt {
    NodeDescent(const std::string& p, const std::shared_ptr<NodeRange>& r, const std::shared_ptr<NodeBlock>& blk) : NodeStmt(), param(p), range(r), block(blk) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
    std::string param;
    std::shared_ptr<NodeRange> range;
    std::shared_ptr<NodeBlock> block;
  };

  struct NodeElseIf : public NodeStmt {
    NodeElseIf(const std::shared_ptr<NodeExp>& c, const std::shared_ptr<NodeBlock>& blk) : NodeStmt(), cond(c), block(blk) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
    std::shared_ptr<NodeExp> cond;
    std::shared_ptr<NodeBlock> block;
  };

  struct NodeIf : public NodeStmt {
  public:
    NodeIf(const std::shared_ptr<NodeExp>& c, const std::shared_ptr<NodeBlock>& t, const std::vector<std::shared_ptr<NodeElseIf>>& elsifs, const std::shared_ptr<NodeBlock>& e) : NodeStmt(), cond(c), thenBlock(t), elsifs(elsifs), elseBlock(e) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
    std::shared_ptr<NodeExp> cond;
    std::shared_ptr<NodeBlock> thenBlock;
    std::vector<std::shared_ptr<NodeElseIf>> elsifs;
    std::shared_ptr<NodeBlock> elseBlock; // Nullable
  };

  struct NodeCase : public Node {
    NodeCase(const std::vector<std::shared_ptr<NodeExp>>& es, const std::shared_ptr<NodeBlock>& blk) : Node(), exps(es), block(blk) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
    std::vector<std::shared_ptr<NodeExp>> exps;
    std::shared_ptr<NodeBlock> block;
  };

  struct NodeAlternative : public NodeStmt {
    NodeAlternative(const std::shared_ptr<NodeExp>& c, const std::vector<std::shared_ptr<NodeCase>>& cs, const std::shared_ptr<NodeBlock>& os) : NodeStmt(), cond(c), cases(cs), others(os) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
    std::shared_ptr<NodeExp> cond;
    std::vector<std::shared_ptr<NodeCase>> cases;
    std::shared_ptr<NodeBlock> others; // Nullable
  };

  struct NodeHeader : public NodeStmt {
    NodeHeader(const std::wstring& name, const std::vector<std::wstring>& params) : NodeStmt(), name(name), params(params) {}
    void traverse(NodeTraverser& traverser) { traverser.traverse(*this); }
    std::wstring name;
    std::vector<std::wstring> params;
  };
}