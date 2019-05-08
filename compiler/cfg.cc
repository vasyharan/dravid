#include "cfg.h"

namespace lang {
namespace compiler {
namespace cfg {

void BasicBlock::emplace_back(std::shared_ptr<const ast::Expression> expr) {
  _expressions.push_back(expr);
}

bool BasicBlock::empty() const { return _expressions.empty(); }

CFGParser::CFGParser(Context &ctx) : _ctx(ctx), _block(nullptr) {}
CFGParser::~CFGParser() {}

void CFGParser::new_block() {
  if (_block == nullptr) {
    _block = std::make_unique<BasicBlock>();
    return;
  }

  if (_block->empty())
    return;

  _ctx.push_block(std::move(_block));
  _block = std::make_unique<BasicBlock>();
}

void CFGParser::parse() {
  new_block();
  _ctx.visit_ast(*this);
}

void CFGParser::visit(std::shared_ptr<const ast::Expression> expr) {
  // _block->emplace_back(expr);
}

void CFGParser::visit(std::shared_ptr<const ast::Assignment> expr) {
  _block->emplace_back(expr);
}

void CFGParser::visit(std::shared_ptr<const ast::BinaryExpression> expr) {
  _block->emplace_back(expr);
}

void CFGParser::visit(std::shared_ptr<const ast::Call> expr) {
  // new_block();
  _block->emplace_back(expr);
  // new_block();
}

void CFGParser::visit(std::shared_ptr<const ast::Function> expr) {
  new_block();
  for (auto &expr : expr->body()) {
    expr->accept(*this);
  }

  new_block();
}

void CFGParser::visit(std::shared_ptr<const ast::If> expr) {
  expr->cond().accept(*this);

  new_block();
  for (auto &expr : expr->thn()) {
    expr->accept(*this);
  }

  new_block();
  for (auto &expr : expr->els()) {
    expr->accept(*this);
  }

  new_block();
}

void CFGParser::visit(std::shared_ptr<const ast::Identifier> expr) {
  _block->emplace_back(expr);
}

void CFGParser::visit(std::shared_ptr<const ast::Integer> expr) {
  _block->emplace_back(expr);
}

void CFGParser::visit(std::shared_ptr<const ast::Parameter> expr) {
  _block->emplace_back(expr);
}

void CFGParser::visit(std::shared_ptr<const ast::Prototype> expr) {
  _block->emplace_back(expr);
}

void CFGParser::visit(std::shared_ptr<const ast::TupleAssignment> expr) {
  _block->emplace_back(expr);
}

void CFGParser::visit(std::shared_ptr<const ast::Value> expr) {
  _block->emplace_back(expr);
}

} // namespace cfg
} // namespace compiler
} // namespace lang
