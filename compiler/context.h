#ifndef LANG_COMPILER_CONTEXT_H
#define LANG_COMPILER_CONTEXT_H

#include "expressions.h"
#include "stack.h"
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
  static const std::string kind_str(const Kind k) {
    switch (k) {
    case SYNTAX:
      return "SYN";
    default:
      assert(false);
      return "INVALID";
    }
  }

  const Kind _kind;
  const std::string _msg;
  const std::string _explanation;

  Error(Kind kind, const std::string &msg, const std::string &explanation)
      : _kind(kind), _msg(msg), _explanation(explanation) {}

public:
  static std::unique_ptr<Error> unexpected_token(const Token &token) {
    return unexpected_token(token, "");
  }

  static std::unique_ptr<Error>
  unexpected_token(const Token &token, const std::string &explanation) {
    auto error =
        new Error(Kind::SYNTAX, "Unexpected " + token.string(), explanation);
    return std::unique_ptr<Error>(error);
  }

  friend std::ostream &operator<<(std::ostream &out, const Error &err) {
    out << kind_str(err._kind) << ": " << err._msg << "\n" << err._explanation;
    return out;
  }
};

class Scope {
  std::map<std::string, llvm::Value *> values_;

public:
  void symbol_add(const std::string &name, llvm::Value *value) {
    values_[name] = value;
  }
  llvm::Value *symbol_lookup(const std::string &name) { return values_[name]; }
};

class Context {
  const std::string name_;
  std::istream &in_;

  std::vector<std::unique_ptr<const Error>> errors_;
  std::vector<std::unique_ptr<const ast::Expression>> nodes_;

  llvm::LLVMContext llvm_;
  llvm::Module module_;
  llvm::IRBuilder<> builder_;
  stack<Scope> _stack;

public:
  Context(const std::string &name, std::istream &in)
      : name_(name), in_(in), module_{llvm::Module(name, llvm_)},
        builder_(llvm_){};
  Context(const Context &) = delete;

  void report_error(std::unique_ptr<const Error> error) {
    errors_.push_back(std::move(error));
  };
  void push_node(std::unique_ptr<const ast::Expression> node) {
    nodes_.push_back(std::move(node));
  };

  // symbol table
  Scope &push_scope() {
    Scope s;
    return _stack.push(s);
  }
  Scope &top_scope() { return _stack.top(); }

  // getters;
  const std::string &name() const { return name_; }
  std::istream &in() { return in_; }
  std::vector<std::unique_ptr<const ast::Expression>> &nodes() {
    return nodes_;
  }
  std::vector<std::unique_ptr<const Error>> &errors() { return errors_; }

  llvm::LLVMContext &llvm() { return llvm_; }
  llvm::IRBuilder<> &builder() { return builder_; }
  llvm::Module &module() { return module_; }
};

} // namespace compiler
} // namespace lang

#endif // LANG_COMPILER_CONTEXT_H
