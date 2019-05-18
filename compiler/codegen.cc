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
    : ctx_(ctx), module_(*(new llvm::Module(ctx.name(), ctx.llvm()))),
      builder_(ctx.llvm()), fpm_(&module_) {
  // Do simple "peephole" optimizations and bit-twiddling optzns.
  // _fpm.add(llvm::createInstructionCombiningPass());
  // Reassociate expressions.
  // _fpm.add(llvm::createReassociatePass());
  // Eliminate Common SubExpressions.
  // _fpm.add(llvm::createGVNPass());
  // Simplify the control flow graph (deleting unreachable blocks, etc).
  // _fpm.add(llvm::createCFGSimplificationPass());
}

Codegen::~Codegen() {}

const llvm::Module &Codegen::module() const { return module_; }

void Codegen::generate() { ctx_.visit_ast(*this); }

// void Codegen::visit(std::shared_ptr<const ast::Expression>) {}

void Codegen::visit(std::shared_ptr<const ast::Assignment> asgn) {
  ctx_.report_error(err::unknown("assignment codegen unimplemented", ""));
}

void Codegen::visit(std::shared_ptr<const ast::BinaryExpression> expr) {
  expr->left().accept(*this);
  expr->right().accept(*this);

  auto right = stack_.top();
  stack_.pop();
  auto left = stack_.top();
  stack_.pop();

  Value *val = nullptr;
  switch (expr->op()) {
  case '+':
    val = builder_.CreateAdd(left, right, "addtmp");
    break;
  case '-':
    val = builder_.CreateSub(left, right, "subtmp");
    break;
  case '*':
    val = builder_.CreateMul(left, right, "multmp");
    break;
  case '/':
    val = builder_.CreateExactSDiv(left, right, "divtmp");
    break;
  default:
    // log error
    break;
  }

  stack_.push(val);
}

void Codegen::visit(std::shared_ptr<const ast::Call> call) {
  Function *callee = module_.getFunction(call->name());
  if (!callee) {
    // log error;
    stack_.push(nullptr);
    return;
  }

  if (callee->arg_size() != call->args().size()) {
    // log error;
    stack_.push(nullptr);
    return;
  }

  std::vector<Value *> args;
  for (auto &expr : call->args()) {
    expr->accept(*this);
    auto arg = stack_.top();
    stack_.pop();
    if (!arg) {
      // log error;
      stack_.push(nullptr);
      return;
    }
    args.emplace_back(arg);
  }

  auto val = builder_.CreateCall(callee, args, "calltmp");
  stack_.push(val);
}

void Codegen::visit(std::shared_ptr<const ast::Function> fn) {
  Function *val = module_.getFunction(fn->proto().name());
  if (!val) {
    fn->proto().accept(*this);
    auto created = stack_.top(); // function created by proto.
    stack_.pop();
    val = module_.getFunction(fn->proto().name());
    assert(created == val);
  }
  if (!val) {
    // report error.
    stack_.push(nullptr);
    return;
  }

  if (!val->empty()) {
    // return error (fn cannot be redefined).
    stack_.push(nullptr);
    return;
  }

  BasicBlock *block = BasicBlock::Create(ctx_.llvm(), "entry", val);
  builder_.SetInsertPoint(block);

  // values_.clear();
  auto &scope = ctx_.push_scope();
  for (auto &arg : val->args()) {
    scope.symbol_add(arg.getName(), &arg);
  }

  for (auto &expr : fn->body()) {
    expr->accept(*this);
  }

  Value *retval = stack_.top();
  for (uint32_t i = 0; i < fn->body().size(); ++i) {
    stack_.pop();
  }
  if (!retval) {
    val->eraseFromParent();
    stack_.push(nullptr);
    return;
  }

  builder_.CreateRet(retval);
  verifyFunction(*val);
  fpm_.run(*val);

  stack_.push(val);
}

void Codegen::visit(std::shared_ptr<const ast::If> expr) {
  expr->cond().accept(*this);
  auto cond = stack_.top();
  stack_.pop();

  if (!cond) {
    stack_.push(nullptr);
    return;
  }

  cond = builder_.CreateICmpEQ(
      cond, ConstantInt::get(ctx_.llvm(), APInt(64, 1, true)), "ifcond");

  Function *fn = builder_.GetInsertBlock()->getParent();
  BasicBlock *thn = BasicBlock::Create(ctx_.llvm(), "then", fn);
  BasicBlock *els = BasicBlock::Create(ctx_.llvm(), "else");
  BasicBlock *mrg = BasicBlock::Create(ctx_.llvm(), "ifcont");
  builder_.CreateCondBr(cond, thn, els);

  // THEN
  builder_.SetInsertPoint(thn);
  for (auto &expr : expr->thn()) {
    expr->accept(*this);
  }
  Value *thnV = stack_.top();
  for (uint32_t i = 0; i < expr->thn().size(); ++i) {
    stack_.pop();
  }

  builder_.CreateBr(mrg);
  thn = builder_.GetInsertBlock(); // codegen can change the block, so restore

  // ELSE
  fn->getBasicBlockList().push_back(els);
  builder_.SetInsertPoint(els);
  for (auto &expr : expr->els()) {
    expr->accept(*this);
  }
  Value *elsV = stack_.top();
  for (uint32_t i = 0; i < expr->els().size(); ++i) {
    stack_.pop();
  }
  builder_.CreateBr(mrg);
  els = builder_.GetInsertBlock(); // codegen can change the block, so restore

  // MERGE
  fn->getBasicBlockList().push_back(mrg);
  builder_.SetInsertPoint(mrg);
  PHINode *phi = builder_.CreatePHI(Type::getInt64Ty(ctx_.llvm()), 2, "iftmp");

  phi->addIncoming(thnV, thn);
  phi->addIncoming(elsV, els);
  stack_.push(phi);
}

void Codegen::visit(std::shared_ptr<const ast::Identifier> id) {
  auto &scope = ctx_.top_scope();
  auto val = scope.symbol_lookup(id->name());
  if (!val) {
    // report error
    stack_.push(nullptr);
    return;
  }

  stack_.push(val);
}

void Codegen::visit(std::shared_ptr<const ast::Integer> integer) {
  auto val = ConstantInt::get(ctx_.llvm(), APInt(64, integer->value(), true));
  stack_.push(val);
}

void Codegen::visit(std::shared_ptr<const ast::Parameter> param) {}

void Codegen::visit(std::shared_ptr<const ast::Prototype> proto) {
  std::vector<Type *> params(proto->params().size(),
                             Type::getInt64Ty(ctx_.llvm()));
  FunctionType *fntype = FunctionType::get(Type::getInt64Ty(ctx_.llvm()),
                                           params, false /* IsVarArgs */);
  Function *fn = Function::Create(fntype, Function::ExternalLinkage,
                                  proto->name(), &module_);

  auto arg_it = fn->args().begin();
  auto param_it = proto->params().begin();
  for (; arg_it != fn->args().end() && param_it != proto->params().end();
       ++arg_it, ++param_it) {
    (*arg_it).setName((*param_it)->name());
  }

  stack_.push(fn);
}

void Codegen::visit(std::shared_ptr<const ast::TupleAssignment> param) {
  ctx_.report_error(err::unknown("tuple assignment codegen unimplemented", ""));
}

void Codegen::visit(std::shared_ptr<const ast::Value> v) {
  v->value().accept(*this);
  auto val = stack_.top();
  if (v->constant()) {
    ctx_.top_scope().symbol_add(v->name(), val);
  }
}

} // namespace codegen
} // namespace compiler
} // namespace lang
