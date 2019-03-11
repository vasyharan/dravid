#ifndef LANG_COMPILER_CONTEXT_H
#define LANG_COMPILER_CONTEXT_H

#include "expressions.h"
#include "token.h"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <memory>
#include <vector>

namespace lang {
namespace compiler {

class Error {
private:
  enum Kind {
    SYNTAX = 1,
  };

  // const Kind kind_;
  const std::string msg_;
  const std::string explanation_;

  Error(Kind kind, const std::string &msg, const std::string &explanation)
      : // kind_(kind),
        msg_(msg), explanation_(explanation) {}

public:
  static std::unique_ptr<Error> unexpected_token(const Token &token) {
    auto error = new Error(Kind::SYNTAX, "Unexpected " + token.string(), "");
    return std::unique_ptr<Error>(error);
  }
};

class Context {
  const std::string name_;
  std::istream &in_;

  std::vector<std::unique_ptr<const Error>> errors_;
  std::vector<std::unique_ptr<const ast::Expression>> nodes_;

  llvm::LLVMContext llvm_;
  llvm::Module module_;
  llvm::IRBuilder<> builder_;
  std::map<std::string, llvm::Value *> values_;

public:
  Context(const std::string &name, std::istream &in)
      : name_(name), in_(in), module_{llvm::Module(name, llvm_)},
        builder_(llvm_){};
  Context(const Context &) = delete;

  const std::string &name() const { return name_; }
  std::istream &in() { return in_; }

  llvm::LLVMContext &llvm() { return llvm_; }
  llvm::IRBuilder<> &builder() { return builder_; }
  llvm::Module &module() { return module_; }
  std::map<std::string, llvm::Value *> &values() { return values_; }

  void report_error(std::unique_ptr<const Error> error) {
    errors_.push_back(std::move(error));
  };
  void push_node(std::unique_ptr<const ast::Expression> node) {
    nodes_.push_back(std::move(node));
  };

  std::vector<std::unique_ptr<const ast::Expression>> &nodes() {
    return nodes_;
  }
};

} // namespace compiler
} // namespace lang

#endif // LANG_COMPILER_CONTEXT_H
