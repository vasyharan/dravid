#ifndef LANG_COMPILER_CONTEXT_H
#define LANG_COMPILER_CONTEXT_H

#include "expressions.h"
#include "stack.h"
#include "token.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
// #include "llvm/Transforms/Scalar/Reassociate.h"
#include <memory>
#include <stack>
#include <vector>

namespace lang {
namespace compiler {

class Error {
private:
  enum Kind {
    SYNTAX = 1,
  };
  static const std::string to_string(const Kind k);

  const Kind _kind;
  const std::string _msg;
  const std::string _explanation;

  Error(Kind kind, const std::string &msg, const std::string &explanation);

public:
  static std::unique_ptr<Error> unexpected_token(const Token &token);
  static std::unique_ptr<Error>
  unexpected_token(const Token &token, const std::string &explanation);

  friend std::ostream &operator<<(std::ostream &out, const Error &err);
};

class Scope {
  std::map<std::string, llvm::Value *> values_;

public:
  void symbol_add(const std::string &name, llvm::Value *value);
  llvm::Value *symbol_lookup(const std::string &name);
};

class GlobalContext {
  llvm::LLVMContext _llvm;

public:
  llvm::LLVMContext &llvm();
}; // namespace compiler

class Context {
  const std::string _name;
  std::istream &_in;

  std::vector<std::unique_ptr<const Error>> _errors;
  std::vector<std::unique_ptr<const ast::Expression>> _nodes;

  GlobalContext &_global;
  std::stack<Scope> _stack;

public:
  Context(GlobalContext &global, const std::string &name, std::istream &in);
  Context(const Context &) = delete;

  void report_error(std::unique_ptr<const Error> error);
  void push_node(std::unique_ptr<const ast::Expression> node);

  // symbol table
  Scope &push_scope();
  Scope &top_scope();

  // getters;
  const std::string &name() const;
  std::istream &in();
  GlobalContext &global();
  llvm::LLVMContext &llvm();
  std::vector<std::unique_ptr<const ast::Expression>> &nodes();
  std::vector<std::unique_ptr<const Error>> &errors();

  // llvm::LLVMContext &llvm() { return _llvm; }
  // llvm::IRBuilder<> &builder() { return _builder; }
  // llvm::Module &module() { return _module; }
  // llvm::legacy::FunctionPassManager &fpm() { return _fpm; }
  // llvm::FunctionAnalysisManager &fam() { return _fam; }
}; // namespace compiler

} // namespace compiler
} // namespace lang

#endif // LANG_COMPILER_CONTEXT_H
