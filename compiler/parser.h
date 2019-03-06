#ifndef LANG_COMPILER_PARSER_H
#define LANG_COMPILER_PARSER_H

#include "context.h"
#include "expressions.h"
#include "lexer.h"

namespace lang {
namespace compiler {

class Parser {
  ILexer &lexer_;
  std::unique_ptr<Token> curr_token_, next_token_;

  std::unique_ptr<Token> advance();
  Token *peek() const;

  std::unique_ptr<const ast::Function> parse_fn(Context &);
  std::unique_ptr<const ast::Prototype> parse_prototype(Context &);
  std::vector<std::unique_ptr<const ast::Parameter>>
  parse_parameters(Context &);
  std::vector<std::unique_ptr<const ast::Expression>> parse_fn_body(Context &);

  std::unique_ptr<const ast::Expression>
  parse_assign(Context &, std::unique_ptr<const ast::Expression> lhs);

  std::unique_ptr<const ast::Expression> parse_stmt(Context &);
  std::unique_ptr<const ast::Expression> parse_expr(Context &);
  std::unique_ptr<const ast::Expression> parse_decl(Context &);
  std::unique_ptr<const ast::Expression> parse_primary(Context &);
  std::unique_ptr<const ast::Expression> parse_operand(Context &);
  std::unique_ptr<const ast::Expression> parse_call(Context &,
                                                    const std::string &);
  std::unique_ptr<const ast::Expression> parse_paren_expr(Context &);
  std::unique_ptr<const ast::Expression>
  parse_binary_expr(Context &, int, std::unique_ptr<const ast::Expression> lhs);

  std::unique_ptr<const ast::Identifier> parse_identifier(Context &);
  std::unique_ptr<const ast::Integer> parse_integer(Context &);

public:
  enum Precedence {
    INVALID = -1,
    NORMAL = 0,
    ADDOP,
    MULOP,
  };

  Parser(ILexer &lexer);
  Parser(const Parser &) = delete;
  Parser(Parser &&) = delete;
  ~Parser();

  void parse(Context &);
};

} // namespace compiler
} // namespace lang

#endif // LANG_COMPILER_PARSER_H
