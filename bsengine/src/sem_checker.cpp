#include <bstorm/util.hpp>
#include <bstorm/sem_checker.hpp>

namespace bstorm {
  static static_script_error invalid_return(const std::wstring& path, int line, int column) {
    return static_script_error(path, line, column, L"'return' with a value is available only in function.");
  }

  static static_script_error variable_call(const std::wstring& path, int line, int column, const std::string& name) {
    return static_script_error(path, line, column, L"can't call variable '" + toUnicode(name) + L"' as if it were a function.");
  }

  static static_script_error undefined_name(const std::wstring& path, int line, int column, const std::string& name) {
    return static_script_error(path, line, column, L"'" + toUnicode(name) + L"' is not defined.");
  }

  static static_script_error wrong_number_args(const std::wstring& path, int line, int column, const std::string& name, int passed, int expected) {
    auto wpassed = std::to_wstring(passed);
    auto wexpected = std::to_wstring(expected);
    auto msg = L"wrong number of arguments was passed to '" + toUnicode(name) + L"' (passed : " + wpassed + L", expected : " + wexpected + L").";
    return static_script_error(path, line, column, msg);
  }

  static static_script_error invalid_left_value(const std::wstring& path, int line, int column, const std::string& name) {
    return static_script_error(path, line, column, L"invalid assignment : '" + toUnicode(name) + L"' is not a variable.");
  }

  static static_script_error invalid_sub_call(const std::wstring& path, int line, int column, const std::string& name) {
    return static_script_error(path, line, column, L"can't call subroutine '" + toUnicode(name) + L"' as an expression.");
  }

  static static_script_error invalid_task_call(const std::wstring& path, int line, int column, const std::string& name) {
    return static_script_error(path, line, column, L"can't call micro thread '" + toUnicode(name) + L"' as an expression.");
  }

  static static_script_error invalid_break(const std::wstring& path, int line, int column) {
    return static_script_error(path, line, column, L"'break' outside the loop.");
  }

  static bool isVariable(const std::shared_ptr<NodeDef>& def) {
    if (std::dynamic_pointer_cast<NodeVarDecl>(def)) { return true; }
    if (std::dynamic_pointer_cast<NodeProcParam>(def)) { return true; }
    if (std::dynamic_pointer_cast<NodeLoopParam>(def)) { return true; }
    return false;
  }

  static int getParamCnt(const std::shared_ptr<NodeDef>& def) {
    if (auto b = std::dynamic_pointer_cast<NodeBuiltInFunc>(def)) return b->paramCnt;
    if (auto f = std::dynamic_pointer_cast<NodeFuncDef>(def)) return (int)f->params.size();
    if (auto t = std::dynamic_pointer_cast<NodeTaskDef>(def)) return (int)t->params.size();
    return 0;
  }

  void SemChecker::traverse(NodeNum&) {}
  void SemChecker::traverse(NodeChar&) {}
  void SemChecker::traverse(NodeStr&) {}
  void SemChecker::traverse(NodeArray& arr) {
    for (auto elem : arr.elems) { elem->traverse(*this); }
  }
  void SemChecker::traverse(NodeNeg& exp) { checkMonoOp(exp); }
  void SemChecker::traverse(NodeNot& exp) { checkMonoOp(exp); }
  void SemChecker::traverse(NodeAbs& exp) { checkMonoOp(exp); }
  void SemChecker::traverse(NodeAdd& exp) { checkBinOp(exp); }
  void SemChecker::traverse(NodeSub& exp) { checkBinOp(exp); }
  void SemChecker::traverse(NodeMul& exp) { checkBinOp(exp); }
  void SemChecker::traverse(NodeDiv& exp) { checkBinOp(exp); }
  void SemChecker::traverse(NodeRem& exp) { checkBinOp(exp); }
  void SemChecker::traverse(NodePow& exp) { checkBinOp(exp); }
  void SemChecker::traverse(NodeLt& exp) { checkBinOp(exp); }
  void SemChecker::traverse(NodeGt& exp) { checkBinOp(exp); }
  void SemChecker::traverse(NodeLe& exp) { checkBinOp(exp); }
  void SemChecker::traverse(NodeGe& exp) { checkBinOp(exp); }
  void SemChecker::traverse(NodeEq& exp) { checkBinOp(exp); }
  void SemChecker::traverse(NodeNe& exp) { checkBinOp(exp); }
  void SemChecker::traverse(NodeAnd& exp) { checkBinOp(exp); }
  void SemChecker::traverse(NodeOr& exp) { checkBinOp(exp); }
  void SemChecker::traverse(NodeCat& exp) { checkBinOp(exp); }
  void SemChecker::traverse(NodeNoParenCallExp& call) {
    auto def = env->findDef(call.name);
    if (!def) {
      // 未定義名の使用
      errors.push_back(undefined_name(*call.filePath, call.line, call.column, call.name));
    } else if (std::dynamic_pointer_cast<NodeSubDef>(def)) {
      // サブルーチンの使用
      errors.push_back(invalid_sub_call(*call.filePath, call.line, call.column, call.name));
    } else if (std::dynamic_pointer_cast<NodeTaskDef>(def)) {
      // マイクロスレッドの使用
      errors.push_back(invalid_task_call(*call.filePath, call.line, call.column, call.name));
    } else if (getParamCnt(def) != 0) {
      // 引数の数が定義と一致しない
      errors.push_back(wrong_number_args(*call.filePath, call.line, call.column, call.name, 0, getParamCnt(def)));
    }
  }
  void SemChecker::traverse(NodeCallExp& call) {
    auto def = env->findDef(call.name);
    if (!def) {
      // 未定義名の使用
      errors.push_back(undefined_name(*call.filePath, call.line, call.column, call.name));
    } else if (std::dynamic_pointer_cast<NodeSubDef>(def)) {
      // サブルーチンの使用
      errors.push_back(invalid_sub_call(*call.filePath, call.line, call.column, call.name));
    } else if (std::dynamic_pointer_cast<NodeTaskDef>(def)) {
      // マイクロスレッドの使用
      errors.push_back(invalid_task_call(*call.filePath, call.line, call.column, call.name));
    } else if (isVariable(def)) {
        // 変数を呼び出している
      errors.push_back(variable_call(*call.filePath, call.line, call.column, call.name));
    } else if (getParamCnt(def) != call.args.size()) {
      // 引数の数が定義と一致しない
      errors.push_back(wrong_number_args(*call.filePath, call.line, call.column, call.name, (int)call.args.size(), getParamCnt(def)));
    }
    for (auto arg : call.args) { arg->traverse(*this); }
  }
  void SemChecker::traverse(NodeArrayRef& exp) {
    exp.array->traverse(*this);
    exp.idx->traverse(*this);
  }
  void SemChecker::traverse(NodeRange& range) {
    range.start->traverse(*this);
    range.end->traverse(*this);
  }
  void SemChecker::traverse(NodeArraySlice& exp) {
    exp.array->traverse(*this);
    exp.range->traverse(*this);
  }
  void SemChecker::traverse(NodeNop &) {}
  void SemChecker::traverse(NodeLeftVal& left) {
    auto def = env->findDef(left.name);
    if (!def) {
      // 未定義名の使用
      errors.push_back(undefined_name(*left.filePath, left.line, left.column, left.name));
    } else if (!isVariable(def)) {
      // 変数以外へのの代入
      errors.push_back(invalid_left_value(*left.filePath, left.line, left.column, left.name));
    }
    for (auto& idx : left.indices) idx->traverse(*this);
  }
  void SemChecker::traverse(NodeAssign& stmt) { checkAssign(stmt); }
  void SemChecker::traverse(NodeAddAssign& stmt) { checkAssign(stmt); }
  void SemChecker::traverse(NodeSubAssign& stmt) { checkAssign(stmt); }
  void SemChecker::traverse(NodeMulAssign& stmt) { checkAssign(stmt); }
  void SemChecker::traverse(NodeDivAssign& stmt) { checkAssign(stmt); }
  void SemChecker::traverse(NodeRemAssign& stmt) { checkAssign(stmt); }
  void SemChecker::traverse(NodePowAssign& stmt) { checkAssign(stmt); }
  void SemChecker::traverse(NodeCatAssign& stmt) { checkAssign(stmt); }
  void SemChecker::traverse(NodeCallStmt& call) {
    auto def = env->findDef(call.name);
    if (!def) {
      // 未定義名の使用
      errors.push_back(undefined_name(*call.filePath, call.line, call.column, call.name));
    } else if (isVariable(def)) {
      // 変数を呼び出している
      errors.push_back(variable_call(*call.filePath, call.line, call.column, call.name));
    } else if (getParamCnt(def) != call.args.size()) {
      // 引数の数が定義と一致しない
      errors.push_back(wrong_number_args(*call.filePath, call.line, call.column, call.name, (int)call.args.size(), getParamCnt(def)));
    }
    for (auto arg : call.args) { arg->traverse(*this); }
  }
  void SemChecker::traverse(NodeReturn& stmt) {
    if (!inFunc()) {
      errors.push_back(invalid_return(*stmt.filePath, stmt.line, stmt.column));
    }
    stmt.ret->traverse(*this);
  }
  void SemChecker::traverse(NodeReturnVoid &) {}
  void SemChecker::traverse(NodeYield &) {}
  void SemChecker::traverse(NodeBreak& stmt) {
    if (!inLoop()) {
      errors.push_back(invalid_break(*stmt.filePath, stmt.line, stmt.column));
    }
  }
  void SemChecker::traverse(NodeSucc& stmt) {
    stmt.lhs->traverse(*this);
  }
  void SemChecker::traverse(NodePred& stmt) {
    stmt.lhs->traverse(*this);
  }
  void SemChecker::traverse(NodeVarDecl&) {}
  void SemChecker::traverse(NodeVarInit& stmt) {
    stmt.rhs->traverse(*this);
  }
  void SemChecker::traverse(NodeProcParam &) {}
  void SemChecker::traverse(NodeLoopParam &) {}
  void SemChecker::traverse(NodeBlock& blk) {
    auto newEnv = std::make_shared<Env>();
    newEnv->parent = env;
    newEnv->table = blk.table;
    env = newEnv;
    for (auto& bind : blk.table) {
      bind.second->traverse(*this);
    }
    for (auto& stmt : blk.stmts) stmt->traverse(*this);
    env = env->parent;
  }
  void SemChecker::traverse(NodeSubDef& def) {
    funcCtxStack.push(false);
    loopCtxStack.push(false);
    def.block->traverse(*this);
    loopCtxStack.pop();
    funcCtxStack.pop();
  }
  void SemChecker::traverse(NodeBuiltInSubDef& def) {
    funcCtxStack.push(false);
    loopCtxStack.push(false);
    def.block->traverse(*this);
    loopCtxStack.pop();
    funcCtxStack.pop();
  }
  void SemChecker::checkMonoOp(NodeMonoOp& exp) {
    exp.rhs->traverse(*this);
  }
  void SemChecker::checkBinOp(NodeBinOp& exp) {
    exp.lhs->traverse(*this);
    exp.rhs->traverse(*this);
  }
  void SemChecker::checkAssign(NodeAssign& stmt) {
    stmt.lhs->traverse(*this);
    stmt.rhs->traverse(*this);
  }
  void SemChecker::traverse(NodeFuncDef& def) {
    funcCtxStack.push(true);
    loopCtxStack.push(false);
    def.block->traverse(*this);
    loopCtxStack.pop();
    funcCtxStack.pop();
  }
  void SemChecker::traverse(NodeTaskDef& def) {
    funcCtxStack.push(false);
    loopCtxStack.push(false);
    def.block->traverse(*this);
    loopCtxStack.pop();
    funcCtxStack.pop();
  }
  void SemChecker::traverse(NodeBuiltInFunc &) {}
  void SemChecker::traverse(NodeConst &) {}
  void SemChecker::traverse(NodeLocal& stmt) {
    stmt.block->traverse(*this);
  }
  void SemChecker::traverse(NodeLoop& stmt) {
    loopCtxStack.push(true);
    stmt.block->traverse(*this);
    loopCtxStack.pop();
  }
  void SemChecker::traverse(NodeTimes& stmt) {
    stmt.cnt->traverse(*this);
    loopCtxStack.push(true);
    stmt.block->traverse(*this);
    loopCtxStack.pop();
  }
  void SemChecker::traverse(NodeWhile& stmt) {
    stmt.cond->traverse(*this);
    loopCtxStack.push(true);
    stmt.block->traverse(*this);
    loopCtxStack.pop();
  }
  void SemChecker::traverse(NodeAscent& stmt) {
    stmt.range->traverse(*this);
    loopCtxStack.push(true);
    stmt.block->traverse(*this);
    loopCtxStack.pop();
  }
  void SemChecker::traverse(NodeDescent& stmt) {
    stmt.range->traverse(*this);
    loopCtxStack.push(true);
    stmt.block->traverse(*this);
    loopCtxStack.pop();
  }
  void SemChecker::traverse(NodeElseIf& elsif) {
    elsif.cond->traverse(*this);
    elsif.block->traverse(*this);
  }
  void SemChecker::traverse(NodeIf& stmt) {
    stmt.cond->traverse(*this);
    stmt.thenBlock->traverse(*this);
    for (auto& elsif : stmt.elsifs) elsif->traverse(*this);
    if (stmt.elseBlock) stmt.elseBlock->traverse(*this);
  }
  void SemChecker::traverse(NodeCase& c) {
    c.exp->traverse(*this);
    c.block->traverse(*this);
  }
  void SemChecker::traverse(NodeAlternative& stmt) {
    stmt.cond->traverse(*this);
    for (auto& c : stmt.cases) c->traverse(*this);
    if (stmt.others) stmt.others->traverse(*this);
  }
  void SemChecker::traverse(NodeHeader &) {}
  bool SemChecker::inFunc() const {
    return funcCtxStack.empty() || funcCtxStack.top() == true;
  }
  bool SemChecker::inLoop() const {
    return loopCtxStack.empty() || loopCtxStack.top() == true;
  }
}