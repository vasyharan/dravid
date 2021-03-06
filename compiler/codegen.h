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
// #include <llvm/IR/Type.h>
// #include <llvm/IR/Verifier.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>

namespace lang {
namespace compiler {
namespace codegen {
class Codegen : public ast::Visitor {
  Context &ctx_;
  llvm::Module &module_;
  llvm::IRBuilder<> builder_;
  llvm::legacy::FunctionPassManager fpm_;
  std::stack<llvm::Value *> stack_;

public:
  Codegen(Context &ctx);
  ~Codegen();

  void generate();
  const llvm::Module &module() const;

  void visit(std::shared_ptr<const ast::Assignment>);
  void visit(std::shared_ptr<const ast::BinaryExpression>);
  void visit(std::shared_ptr<const ast::Call>);
  void visit(std::shared_ptr<const ast::Function>);
  void visit(std::shared_ptr<const ast::If>);
  void visit(std::shared_ptr<const ast::Identifier>);
  void visit(std::shared_ptr<const ast::Integer>);
  void visit(std::shared_ptr<const ast::Parameter>);
  void visit(std::shared_ptr<const ast::Prototype>);
  void visit(std::shared_ptr<const ast::TupleAssignment>);
  void visit(std::shared_ptr<const ast::Value>);
};

} // namespace codegen
} // namespace compiler
} // namespace lang

#endif // LANG_COMPILER_CODEGEN_H
