#include "parser.h"
#include <cassert>

namespace lang {
namespace compiler {

// Location UNKNOWN_LOC = Location();

Parser::Parser(ILexer &lexer)
    : lexer_(lexer), curr_token_(Token::make_invalid().release()),
      next_token_(Token::make_invalid().release()) {
  advance();
}

Parser::~Parser() {}

void Parser::parse(Context &ctx) {
  auto token = peek();
  while (!token->eof()) {
    if (!token->is_keyword()) {
      ctx.report_error(Error::unexpected_token(*token));
      break;
    }

    switch (token->keyword()) {
    case Token::Keyword::kwDEF:
      if (auto fn = parse_fn(ctx)) {
        ctx.push_node(std::move(fn));
      }
    default:
      // TODO: report error.
      break;
    }

    token = peek();
  }
}

std::unique_ptr<const ast::Function> Parser::parse_fn(Context &ctx) {
  auto token = advance();
  if (!token->is_keyword(Token::Keyword::kwDEF)) {
    // TODO: report error.
    ctx.report_error(Error::unexpected_token(*token));
    return nullptr;
  }

  auto prototype = parse_prototype(ctx);
  if (!prototype)
    return nullptr;

  auto body = parse_fn_body(ctx);

  return std::make_unique<const ast::Function>(std::move(prototype),
                                               std::move(body));
}

std::unique_ptr<const ast::Prototype> Parser::parse_prototype(Context &ctx) {
  auto token = advance();
  if (!token->is_identifier()) {
    // TODO: report error.
    return nullptr;
  }

  const std::string name = token->identifier();
  auto params = parse_parameters(ctx);

  return std::make_unique<const ast::Prototype>(name, std::move(params));
}

std::vector<std::unique_ptr<const ast::Parameter>>
Parser::parse_parameters(Context &ctx) {
  std::vector<std::unique_ptr<const ast::Parameter>> params;

  auto token = advance();
  if (!token->is_operator(Token::Operator::opLPAREN)) {
    // TODO: report error.
    return params;
  }

  for (token = advance(); token->is_identifier(); token = advance()) {
    auto param =
        std::make_unique<const ast::Parameter>(false, token->identifier());
    params.push_back(std::move(param));

    token = advance();
    if (!token->is_operator(Token::Operator::opCOMMA)) {
      break;
    }
  }

  if (!token->is_operator(Token::Operator::opRPAREN)) {
    // TODO: report error.
    return params;
  }

  return params;
}

std::vector<std::unique_ptr<const ast::Expression>>
Parser::parse_fn_body(Context &ctx) {
  std::vector<std::unique_ptr<const ast::Expression>> body;

  auto token = advance();
  if (!token->is_operator(Token::Operator::opLCURLY)) {
    // TODO: report error.
    return body;
  }

  for (auto expr = parse_expr(ctx); expr != nullptr; expr = parse_expr(ctx)) {
    body.push_back(std::move(expr));
  }

  token = advance();
  if (!token->is_operator(Token::Operator::opRPAREN)) {
    // TODO: report error.
    return body;
  }

  return body;
}

std::unique_ptr<const ast::Expression> Parser::parse_primary(Context &ctx) {
  auto token = peek();
  switch (token->type()) {
  default:
    break;
  case Token::Type::tIDENTIFIER:
    return parse_identifier(ctx);
  case Token::Type::tINTEGER:
    return parse_integer(ctx);
  case Token::Type::tOPERATOR:
    if (token->is_operator(Token::Operator::opLPAREN)) {
      return parse_paren_expr(ctx);
    }
  }

  // TODO: report error
  return nullptr;
}

std::unique_ptr<const ast::Identifier> Parser::parse_identifier(Context &ctx) {
  assert(peek()->is_identifier());
  auto token = advance();
  return std::make_unique<ast::Identifier>(token->identifier());
}

std::unique_ptr<const ast::Integer> Parser::parse_integer(Context &ctx) {
  assert(peek()->is_integer());
  auto token = advance();
  return std::make_unique<ast::Integer>(token->integer());
}

std::unique_ptr<const ast::Expression> Parser::parse_paren_expr(Context &ctx) {
  if (!peek()->is_operator(Token::Operator::opLPAREN)) {
    // TODO: report error.
    return nullptr;
  }
  advance();

  auto expr = parse_expr(ctx);
  if (!expr)
    return nullptr;

  if (!peek()->is_operator(Token::Operator::opRPAREN)) {
    // TODO: report error.
    return nullptr;
  }
  advance();

  return expr;
}
std::unique_ptr<const ast::Expression>
Parser::parse_assign(Context &ctx, std::unique_ptr<const ast::Expression> lhs) {
  auto token = advance();
  if (!token->is_operator(Token::Operator::opEQUAL)) {
    // TODO: report error.
  }

  return std::make_unique<const ast::Assign>(std::move(lhs), parse_expr(ctx));
}

std::unique_ptr<const ast::Expression> Parser::parse_expr(Context &ctx) {
  auto lhs = parse_primary(ctx);
  if (!lhs) {
    // TODO: report error.
    return nullptr;
  }

  auto token = peek();
  if (!token->is_operator()) {
    ctx.report_error(Error::unexpected_token(*token));
    return nullptr;
  }

  switch (token->op()) {
  case Token::Operator::opEQUAL:
    return parse_assign(ctx, std::move(lhs));
  default:
    return parse_binary_expr(ctx, 0, std::move(lhs));
  }
}

std::unique_ptr<const ast::Expression>
Parser::parse_binary_expr(Context &ctx, int expr_precedence,
                          std::unique_ptr<const ast::Expression> lhs) {
  while (true) {
    auto token = peek();
    auto op = token->op();

    int tok_precedence;
    switch (op) {
    default:
      tok_precedence = Precedence::INVALID;
      break;
    case Token::Operator::opSTAR:
    case Token::Operator::opSLASH:
      tok_precedence = Precedence::MULOP;
      break;
    case Token::Operator::opPLUS:
    case Token::Operator::opDASH:
      tok_precedence = Precedence::ADDOP;
      break;
    }

    if (tok_precedence < expr_precedence)
      return lhs;

    advance(); // eat op token

    auto rhs = parse_primary(ctx);
    if (!rhs)
      return nullptr;

    int next_precedence;
    switch (op) {
    default:
      next_precedence = Precedence::INVALID;
      break;
    case Token::Operator::opSTAR:
    case Token::Operator::opSLASH:
      next_precedence = Precedence::MULOP;
      break;
    case Token::Operator::opPLUS:
    case Token::Operator::opDASH:
      next_precedence = Precedence::ADDOP;
      break;
    }

    if (tok_precedence < next_precedence) {
      rhs = parse_binary_expr(ctx, tok_precedence, std::move(rhs));
      if (!rhs)
        return nullptr;
    }

    lhs = std::make_unique<const ast::BinaryExpression>(op, std::move(lhs),
                                                        std::move(rhs));
  }

  return lhs;
} // namespace compiler

Token *Parser::advance() {
  if (curr_token_ == next_token_)
    next_token_ = lexer_.lex();

  curr_token_ = std::move(next_token_);
  next_token_ = lexer_.lex();
  return curr_token_.get();
}

Token *Parser::peek() const { return next_token_.get(); }

} // namespace compiler
} // namespace lang
