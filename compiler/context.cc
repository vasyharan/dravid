#include "context.h"

namespace lang {
namespace compiler {

// -----------------------------------------------------------------------------
// Error
// -----------------------------------------------------------------------------
namespace err {
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

std::unique_ptr<Error> unexpected_token(const lex::Token &token) {
  return unexpected_token(token, "");
}

std::unique_ptr<Error> unexpected_token(const lex::Token &token,
                                        const std::string &explanation) {
  auto error =
      new Error(Kind::SYNTAX, "Unexpected " + token.string(), explanation);
  return std::unique_ptr<Error>(error);
}

std::unique_ptr<Error> unknown(const std::string &msg,
                               const std::string &explanation) {
  auto error = new Error(Kind::INVALID, msg, explanation);
  return std::unique_ptr<Error>(error);
}

std::ostream &operator<<(std::ostream &out, const Error &err) {
  out << Error::to_string(err._kind) << ": " << err._msg << "\n"
      << err._explanation;
  return out;
}

void Error::accept(Visitor &visitor) const { visitor.visit(*this); }
} // namespace err

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

void Context::report_error(std::unique_ptr<const err::Error> error) {
  _errors.push_back(std::move(error));
};

void Context::push_node(std::shared_ptr<const ast::Expression> node) {
  _nodes.push_back(node);
};

void Context::push_block(std::unique_ptr<const cfg::BasicBlock> block) {
  _blocks.push_back(std::move(block));
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
bool Context::good() const { return !_errors.empty(); }

void Context::each_expr(std::function<void(const ast::Expression &)> fn) {
  for (auto &node : _nodes) {
    fn(*node);
  }
}

void Context::each_block(std::function<void(const cfg::BasicBlock &)> fn) {
  for (auto &block : _blocks) {
    fn(*block);
  }
}

void Context::each_error(std::function<void(const err::Error &)> fn) {
  for (auto &err : _errors) {
    fn(*err);
  }
}

// visitors
void Context::visit_ast(ast::Visitor &visitor) {
  each_expr([&visitor](const ast::Expression &expr) -> void {
    expr.accept(visitor);
  });
}

// void Context::visit_block(cfg::Visitor &visitor) {
//   each_block([&visitor](const cfg::BasicBlock &block) -> void {
//     block.accept(visitor);
//   });
// }

} // namespace compiler
} // namespace lang
