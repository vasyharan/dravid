#ifndef LANG_COMPILER_CFG_H
#define LANG_COMPILER_CFG_H

#include "context.h"
#include "expressions.h"
#include <memory>
#include <vector>

namespace lang {
namespace compiler {
namespace cfg {

class CFGParser : public ast::Visitor {
  Context &_ctx;
  std::unique_ptr<BasicBlock> _block;

  CFGParser(Context &ctx);
  ~CFGParser();

  void parse();
  void new_block();

public:
  static void parse_into(Context &ctx) {
    CFGParser parser(ctx);
    parser.parse();
  }

  void visit(std::shared_ptr<const ast::Expression>);
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

} // namespace cfg
} // namespace compiler
} // namespace lang

#endif // LANG_COMPILER_CFG_H
