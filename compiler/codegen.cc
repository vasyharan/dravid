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

Codegen::Codegen() {}
Codegen::~Codegen() {}

void Codegen::generate(Context &c) {
  ctx_ = &c;
  for (auto &node : ctx_->nodes()) {
    node->accept(*this);
  }
  ctx_ = nullptr;
}

void Codegen::visit(const ast::Assignment &asgn) {}

void Codegen::visit(const ast::BinaryExpression &expr) {
  expr.left().accept(*this);
  expr.right().accept(*this);

  auto right = pop();
  auto left = pop();

  Value *val = nullptr;
  switch (expr.op()) {
  case '+':
    val = builder().CreateAdd(left, right, "addtmp");
    break;
  case '-':
    val = builder().CreateSub(left, right, "subtmp");
    break;
  case '*':
    val = builder().CreateMul(left, right, "multmp");
    break;
  case '/':
    val = builder().CreateExactSDiv(left, right, "divtmp");
    break;
  default:
    // log error
    break;
  }

  push(val);
}

void Codegen::visit(const ast::Call &call) {
  Function *callee = module().getFunction(call.name());
  if (!callee) {
    // log error;
    push(nullptr);
    return;
  }

  if (callee->arg_size() != call.args().size()) {
    // log error;
    push(nullptr);
    return;
  }

  std::vector<Value *> args;
  for (auto &expr : call.args()) {
    expr->accept(*this);
    auto arg = pop();
    if (!arg) {
      // log error;
      push(nullptr);
      return;
    }
    args.emplace_back(arg);
  }

  auto val = builder().CreateCall(callee, args, "calltmp");
  push(val);
}

void Codegen::visit(const ast::Function &fn) {
  Function *llfn = module().getFunction(fn.proto().name());
  if (!llfn) {
    fn.proto().accept(*this);
    auto created = pop(); // function created by proto.
    llfn = module().getFunction(fn.proto().name());
    assert(created == llfn);
  }
  if (!llfn) {
    // report error.
    push(nullptr);
    return;
  }

  if (!llfn->empty()) {
    // return error (fn cannot be redefined).
    push(nullptr);
    return;
  }

  BasicBlock *block = BasicBlock::Create(ctx(), "entry", llfn);
  builder().SetInsertPoint(block);

  // values_.clear();
  clear_lookup();
  for (auto &arg : llfn->args()) {
    add_lookup(arg.getName(), &arg);
  }

  assert(fn.body().size() == 1);
  fn.body()[0]->accept(*this);
  Value *retval = pop();
  if (!retval) {
    llfn->eraseFromParent();
    push(nullptr);
    return;
  }

  builder().CreateRet(retval);
  push(llfn);
}

void Codegen::visit(const ast::Identifier &id) {
  auto val = lookup_value(id.name());
  if (!val) {
    // report error
    push(nullptr);
    return;
  }

  push(val);
}

void Codegen::visit(const ast::Integer &integer) {
  auto val = ConstantInt::get(ctx(), APInt(64, integer.value(), true));
  push(val);
}

void Codegen::visit(const ast::Parameter &param) {}

void Codegen::visit(const ast::Prototype &proto) {
  std::vector<Type *> params(proto.params().size(), Type::getInt64Ty(ctx()));
  FunctionType *fntype =
      FunctionType::get(Type::getInt64Ty(ctx()), params, false /* IsVarArgs */);
  Function *fn = Function::Create(fntype, Function::ExternalLinkage,
                                  proto.name(), &module());

  auto arg_it = fn->args().begin();
  auto param_it = proto.params().begin();
  for (; arg_it != fn->args().end() && param_it != proto.params().end();
       ++arg_it, ++param_it) {
    (*arg_it).setName((*param_it)->name());
  }

  push(fn);
}

void Codegen::visit(const ast::TupleAssignment &param) {}
void Codegen::visit(const ast::Value &param) {}

void Codegen::push(Value *v) { stack_.emplace(v); }

Value *Codegen::pop() {
  auto v = stack_.top();
  stack_.pop();
  return v;
}

} // namespace codegen
} // namespace compiler
} // namespace lang
