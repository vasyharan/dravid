#ifndef LANG_COMPILER_PARSER_H
#define LANG_COMPILER_PARSER_H

#include "context.h"
#include "expressions.h"
#include "lexer.h"

namespace lang {
namespace compiler {

class Parser {
  Context &_ctx;
  ILexer &_lexer;
  std::unique_ptr<Token> _curr_token, _next_token;

  std::unique_ptr<Token> advance();
  Token *peek() const;

  std::shared_ptr<const ast::Function> parse_fn();
  std::shared_ptr<const ast::Prototype> parse_prototype();
  std::vector<std::shared_ptr<const ast::Parameter>> parse_parameters();
  std::vector<std::shared_ptr<const ast::Expression>> parse_fn_body();

  std::shared_ptr<const ast::Expression> parse_stmt();
  std::shared_ptr<const ast::Expression> parse_decl();
  std::shared_ptr<const ast::Expression> parse_if();
  std::shared_ptr<const ast::Expression>
  parse_assign(std::shared_ptr<const ast::Expression> lhs);

  std::shared_ptr<const ast::Expression> parse_expr();
  std::shared_ptr<const ast::Expression> parse_primary();
  std::shared_ptr<const ast::Expression> parse_operand();
  std::shared_ptr<const ast::Expression> parse_call(const std::string &);
  std::shared_ptr<const ast::Expression> parse_paren_expr();
  std::shared_ptr<const ast::Expression>
  parse_binary_expr(int, std::shared_ptr<const ast::Expression>);

  std::shared_ptr<const ast::Identifier> parse_identifier();
  std::shared_ptr<const ast::Integer> parse_integer();

  void gather_block(std::vector<std::shared_ptr<const ast::Expression>> &);

public:
  enum Precedence {
    INVALID = -1,
    NORMAL = 0,
    ADDOP,
    MULOP,
  };

  Parser(ILexer &, Context &);
  Parser(const Parser &) = delete;
  Parser(Parser &&) = delete;
  ~Parser();

  void parse();
};

} // namespace compiler
} // namespace lang

#endif // LANG_COMPILER_PARSER_H
