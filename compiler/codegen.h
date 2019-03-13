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
  Context &_ctx;
  llvm::Module _module;
  llvm::IRBuilder<> _builder;
  llvm::legacy::FunctionPassManager _fpm;
  std::stack<llvm::Value *> _stack;

public:
  Codegen(Context &ctx);
  ~Codegen();

  void generate();
  const llvm::Module &module() const;

  void visit(const ast::Assignment &);
  void visit(const ast::BinaryExpression &);
  void visit(const ast::Call &);
  void visit(const ast::Function &);
  void visit(const ast::If &);
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
