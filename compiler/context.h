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
#include <functional>
#include <memory>
#include <stack>
#include <vector>

namespace lang {
namespace compiler {

namespace err {
class Visitor;

enum Kind {
  INVALID = -1,
  SYNTAX = 1,
};

class Error {
private:
  static const std::string to_string(const Kind k);

  const Kind _kind;
  const std::string _msg;
  const std::string _explanation;

public:
  Error(Kind kind, const std::string &msg, const std::string &explanation);

  friend std::ostream &operator<<(std::ostream &out, const Error &err);

  void accept(Visitor &) const;
};

// UnexpectedToken error
std::unique_ptr<Error> unexpected_token(const lex::Token &);
// UnexpectedToken error
std::unique_ptr<Error> unexpected_token(const lex::Token &,
                                        const std::string &);
// Unknown error
std::unique_ptr<Error> unknown(const std::string &, const std::string &);

class Visitor {
public:
  virtual void visit(const Error &err) = 0;
};

} // namespace err

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

  std::vector<std::unique_ptr<const err::Error>> _errors;
  std::vector<std::shared_ptr<const ast::Expression>> _nodes;
  std::vector<std::unique_ptr<const cfg::BasicBlock>> _blocks;

  GlobalContext &_global;
  std::stack<Scope> _stack;

public:
  Context(GlobalContext &global, const std::string &name, std::istream &in);
  Context(const Context &) = delete;

  void report_error(std::unique_ptr<const err::Error> error);
  void push_node(std::shared_ptr<const ast::Expression> node);
  void push_block(std::unique_ptr<const cfg::BasicBlock> block);

  // symbol table
  Scope &push_scope();
  Scope &top_scope();

  // getters;
  const std::string &name() const;
  std::istream &in();
  GlobalContext &global();
  llvm::LLVMContext &llvm();
  bool good() const;

  void visit_ast(ast::Visitor &vistor);
  // void visit_block(cfg::Visitor &vistor);

  void each_expr(std::function<void(const ast::Expression &)>);
  void each_block(std::function<void(const cfg::BasicBlock &)>);
  void each_error(std::function<void(const err::Error &)>);

  // llvm::LLVMContext &llvm() { return _llvm; }
  // llvm::IRBuilder<> &builder() { return _builder; }
  // llvm::Module &module() { return _module; }
  // llvm::legacy::FunctionPassManager &fpm() { return _fpm; }
  // llvm::FunctionAnalysisManager &fam() { return _fam; }
}; // namespace compiler

} // namespace compiler
} // namespace lang

#endif // LANG_COMPILER_CONTEXT_H
