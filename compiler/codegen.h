#ifndef LANG_COMPILER_CODEGEN_H
#define LANG_COMPILER_CODEGEN_H

#include "context.h"
#include "expressions.h"
#include <map>
#include <memory>
#include <stack>

// #include <llvm/ADT/STLExtras.h>
// #include <llvm/IR/BasicBlock.h>
// #include <llvm/IR/Constants.h>
// #include <llvm/IR/DerivedTypes.h>
// #include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
// #include <llvm/IR/Type.h>
// #include <llvm/IR/Verifier.h>
#include <llvm/IR/Value.h>

namespace lang {
namespace compiler {
namespace codegen {
class Codegen : public ast::Visitor {
  Context *ctx_;

  std::stack<llvm::Value *> stack_;
  void push(llvm::Value *);
  llvm::Value *pop();

  inline llvm::IRBuilder<> &builder() { return ctx_->builder(); }
  inline llvm::Module &module() { return ctx_->module(); }
  inline llvm::LLVMContext &ctx() { return ctx_->llvm(); }
  inline void clear_lookup() { return ctx_->values().clear(); }
  inline void add_lookup(const std::string &name, llvm::Value *value) {
    ctx_->values()[name] = value;
  }
  inline llvm::Value *lookup_value(const std::string &name) {
    return ctx_->values()[name];
  }

public:
  Codegen();
  ~Codegen();

  void generate(Context &);

  void visit(const ast::Assignment &);
  void visit(const ast::BinaryExpression &);
  void visit(const ast::Call &);
  void visit(const ast::Function &);
  void visit(const ast::Identifier &);
  void visit(const ast::Integer &);
  void visit(const ast::Parameter &);
  void visit(const ast::Prototype &);
  void visit(const ast::TupleAssignment &);
  void visit(const ast::Value &);
};

} // namespace codegen
} // namespace compiler
} // namespace lang

#endif // LANG_COMPILER_CODEGEN_H
