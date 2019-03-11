#include "codegen.h"
#include <vector>

#include <llvm/ADT/APInt.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>

using namespace llvm;

namespace lang {
namespace compiler {
namespace codegen {

Codegen::Codegen(Context &ctx)
    : _ctx(ctx), _module{llvm::Module(ctx.name(), ctx.llvm())},
      _builder(ctx.llvm()), _fpm(&_module) {
  // Do simple "peephole" optimizations and bit-twiddling optzns.
  _fpm.add(llvm::createInstructionCombiningPass());
  // Reassociate expressions.
  _fpm.add(llvm::createReassociatePass());
  // Eliminate Common SubExpressions.
  _fpm.add(llvm::createGVNPass());
  // Simplify the control flow graph (deleting unreachable blocks, etc).
  _fpm.add(llvm::createCFGSimplificationPass());
}

Codegen::~Codegen() {}

const llvm::Module &Codegen::module() const { return _module; }

void Codegen::generate() {
  for (auto &node : _ctx.nodes()) {
    node->accept(*this);
  }
}

void Codegen::visit(const ast::Assignment &asgn) {}

void Codegen::visit(const ast::BinaryExpression &expr) {
  expr.left().accept(*this);
  expr.right().accept(*this);

  auto right = _stack.top();
  _stack.pop();
  auto left = _stack.top();
  _stack.pop();

  Value *val = nullptr;
  switch (expr.op()) {
  case '+':
    val = _builder.CreateAdd(left, right, "addtmp");
    break;
  case '-':
    val = _builder.CreateSub(left, right, "subtmp");
    break;
  case '*':
    val = _builder.CreateMul(left, right, "multmp");
    break;
  case '/':
    val = _builder.CreateExactSDiv(left, right, "divtmp");
    break;
  default:
    // log error
    break;
  }

  _stack.push(val);
}

void Codegen::visit(const ast::Call &call) {
  Function *callee = _module.getFunction(call.name());
  if (!callee) {
    // log error;
    _stack.push(nullptr);
    return;
  }

  if (callee->arg_size() != call.args().size()) {
    // log error;
    _stack.push(nullptr);
    return;
  }

  std::vector<Value *> args;
  for (auto &expr : call.args()) {
    expr->accept(*this);
    auto arg = _stack.top();
    _stack.pop();
    if (!arg) {
      // log error;
      _stack.push(nullptr);
      return;
    }
    args.emplace_back(arg);
  }

  auto val = _builder.CreateCall(callee, args, "calltmp");
  _stack.push(val);
}

void Codegen::visit(const ast::Function &fn) {
  Function *val = _module.getFunction(fn.proto().name());
  if (!val) {
    fn.proto().accept(*this);
    auto created = _stack.top(); // function created by proto.
    _stack.pop();
    val = _module.getFunction(fn.proto().name());
    assert(created == val);
  }
  if (!val) {
    // report error.
    _stack.push(nullptr);
    return;
  }

  if (!val->empty()) {
    // return error (fn cannot be redefined).
    _stack.push(nullptr);
    return;
  }

  BasicBlock *block = BasicBlock::Create(_ctx.llvm(), "entry", val);
  _builder.SetInsertPoint(block);

  // values_.clear();
  auto &scope = _ctx.push_scope();
  for (auto &arg : val->args()) {
    scope.symbol_add(arg.getName(), &arg);
  }

  assert(fn.body().size() == 1);
  fn.body()[0]->accept(*this);
  Value *retval = _stack.top();
  _stack.pop();
  if (!retval) {
    val->eraseFromParent();
    _stack.push(nullptr);
    return;
  }

  _builder.CreateRet(retval);
  verifyFunction(*val);
  _fpm.run(*val);

  _stack.push(val);
}

void Codegen::visit(const ast::Identifier &id) {
  auto &scope = _ctx.top_scope();
  auto val = scope.symbol_lookup(id.name());
  if (!val) {
    // report error
    _stack.push(nullptr);
    return;
  }

  _stack.push(val);
}

void Codegen::visit(const ast::Integer &integer) {
  auto val = ConstantInt::get(_ctx.llvm(), APInt(64, integer.value(), true));
  _stack.push(val);
}

void Codegen::visit(const ast::Parameter &param) {}

void Codegen::visit(const ast::Prototype &proto) {
  std::vector<Type *> params(proto.params().size(),
                             Type::getInt64Ty(_ctx.llvm()));
  FunctionType *fntype = FunctionType::get(Type::getInt64Ty(_ctx.llvm()),
                                           params, false /* IsVarArgs */);
  Function *fn = Function::Create(fntype, Function::ExternalLinkage,
                                  proto.name(), &_module);

  auto arg_it = fn->args().begin();
  auto param_it = proto.params().begin();
  for (; arg_it != fn->args().end() && param_it != proto.params().end();
       ++arg_it, ++param_it) {
    (*arg_it).setName((*param_it)->name());
  }

  _stack.push(fn);
}

void Codegen::visit(const ast::TupleAssignment &param) {}
void Codegen::visit(const ast::Value &param) {}

} // namespace codegen
} // namespace compiler
} // namespace lang
