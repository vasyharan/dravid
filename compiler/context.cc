#include "context.h"

namespace lang {
namespace compiler {

// -----------------------------------------------------------------------------
// Error
// -----------------------------------------------------------------------------
const std::string Error::to_string(const Kind k) {
  switch (k) {
  case SYNTAX:
    return "SYN";
  default:
    assert(false);
    return "INVALID";
  }
}

Error::Error(Kind kind, const std::string &msg, const std::string &explanation)
    : _kind(kind), _msg(msg), _explanation(explanation) {}

std::unique_ptr<Error> Error::unexpected_token(const Token &token) {
  return unexpected_token(token, "");
}

std::unique_ptr<Error> Error::unexpected_token(const Token &token,
                                               const std::string &explanation) {
  auto error =
      new Error(Kind::SYNTAX, "Unexpected " + token.string(), explanation);
  return std::unique_ptr<Error>(error);
}

std::unique_ptr<Error> Error::unknown(const std::string &msg,
                                      const std::string &explanation) {
  auto error = new Error(Kind::INVALID, msg, explanation);
  return std::unique_ptr<Error>(error);
}

std::ostream &operator<<(std::ostream &out, const Error &err) {
  out << Error::to_string(err._kind) << ": " << err._msg << "\n"
      << err._explanation;
  return out;
}

// -----------------------------------------------------------------------------
// Scope
// -----------------------------------------------------------------------------
void Scope::symbol_add(const std::string &name, llvm::Value *value) {
  values_[name] = value;
}

llvm::Value *Scope::symbol_lookup(const std::string &name) {
  return values_[name];
}

// -----------------------------------------------------------------------------
// GlobalContext
// -----------------------------------------------------------------------------
llvm::LLVMContext &GlobalContext::llvm() { return _llvm; }

// -----------------------------------------------------------------------------
// Context
// -----------------------------------------------------------------------------
Context::Context(GlobalContext &global, const std::string &name,
                 std::istream &in)
    : _name(name), _in(in), _global(global) {}

void Context::report_error(std::unique_ptr<const Error> error) {
  _errors.push_back(std::move(error));
};

void Context::push_node(std::unique_ptr<const ast::Expression> node) {
  _nodes.push_back(std::move(node));
};

Scope &Context::push_scope() {
  Scope s;
  _stack.push(s);
  return _stack.top();
}

// getters
Scope &Context::top_scope() { return _stack.top(); }
const std::string &Context::name() const { return _name; }
GlobalContext &Context::global() { return _global; }
llvm::LLVMContext &Context::llvm() { return _global.llvm(); }
std::istream &Context::in() { return _in; }
std::vector<std::unique_ptr<const ast::Expression>> &Context::nodes() {
  return _nodes;
}
std::vector<std::unique_ptr<const Error>> &Context::errors() { return _errors; }

} // namespace compiler
} // namespace lang
