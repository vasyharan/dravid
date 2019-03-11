#include "parser.h"
#include <cassert>

namespace lang {
namespace compiler {

// Location UNKNOWN_LOC = Location();

Parser::Parser()
    : lexer_(nullptr), curr_token_(nullptr), next_token_(nullptr) {}

Parser::~Parser() {}

void Parser::parse(ILexer &lexer, Context &ctx) {
  lexer_ = &lexer;
  next_token_ = lexer_->lex();
  auto peep = peek();
  while (!peep->eof()) {
    if (!peep->is_keyword()) {
      ctx.report_error(Error::unexpected_token(*peep));
      break;
    }

    switch (peep->keyword()) {
    case Token::Keyword::kwDEF:
      if (auto fn = parse_fn(ctx)) {
        ctx.push_node(std::move(fn));
      }
    default:
      // TODO: report error.
      break;
    }

    peep = peek();
  }

  lexer_ = nullptr;
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

  for (auto expr = parse_stmt(ctx); expr != nullptr; expr = parse_stmt(ctx)) {
    body.push_back(std::move(expr));
  }

  token = advance();
  if (!token->is_operator(Token::Operator::opRCURLY)) {
    // TODO: report error.
    return body;
  }

  return body;
}

std::unique_ptr<const ast::Expression> Parser::parse_stmt(Context &ctx) {
  switch (peek()->type()) {
  case Token::Type::tKEYWORD:
    switch (peek()->keyword()) {
    case Token::Keyword::kwVAL:
      return parse_decl(ctx);
    default:
      return parse_expr(ctx);
    }
  default:
    return parse_expr(ctx);
  }
}

std::unique_ptr<const ast::Expression> Parser::parse_decl(Context &ctx) {
  auto token = advance();
  if (!token->is_keyword(Token::Keyword::kwVAL)) {
    // TODO: report error.
    return nullptr;
  }

  std::vector<std::string> names;
  for (token = advance(); token->is_identifier(); token = advance()) {
    names.push_back(token->identifier());

    if (!peek()->is_operator(Token::Operator::opCOMMA)) {
      break;
    }
  }

  token = advance();
  if (!token->is_operator(Token::Operator::opEQUAL)) {
    // TODO: report error.
    return nullptr;
  }

  std::vector<std::unique_ptr<const ast::Expression>> values;
  for (auto it = names.begin(); it != names.end(); ++it) {
    values.push_back(parse_expr(ctx));

    if (!peek()->is_operator(Token::Operator::opCOMMA)) {
      break;
    }
  }

  if (names.size() != values.size()) {
    // TODO: report error.
    return nullptr;
  }

  if (names.size() == 1) {
    return std::make_unique<const ast::Value>(true, names[0],
                                              std::move(values[0]));
  } else {
    // TODO: report error.
    return nullptr;
  }
}

std::unique_ptr<const ast::Expression> Parser::parse_expr(Context &ctx) {
  auto lhs = parse_primary(ctx);
  if (!lhs) {
    // TODO: report error.
    return nullptr;
  }

  auto token = peek();
  if (!token->is_operator()) {
    // ctx.report_error(Error::unexpected_token(*token));
    return lhs;
  }

  switch (token->op()) {
  // case Token::Operator::opEQUAL:
  //   return parse_assign(ctx, std::move(lhs));
  default:
    return parse_binary_expr(ctx, Precedence::NORMAL, std::move(lhs));
  }
}

std::unique_ptr<const ast::Expression> Parser::parse_primary(Context &ctx) {
  auto peep = peek();
  switch (peep->type()) {
  case Token::Type::tIDENTIFIER: {
    auto token = advance();
    peep = peek();
    if (peep->is_operator(Token::Operator::opLPAREN)) {
      return parse_call(ctx, token->identifier());
    } else {
      return std::make_unique<ast::Identifier>(token->identifier());
    }
  }
  case Token::Type::tINTEGER:
    return parse_integer(ctx);
  case Token::Type::tOPERATOR:
    if (peep->is_operator(Token::Operator::opLPAREN)) {
      return parse_paren_expr(ctx);
    }
  default:
    // TODO: report error
    return nullptr;
  }
}

std::unique_ptr<const ast::Expression>
Parser::parse_call(Context &ctx, const std::string &name) {
  std::vector<std::unique_ptr<const ast::Expression>> args;

  auto token = advance();
  if (!token->is_operator(Token::Operator::opLPAREN)) {
    // TODO: report error.
    return nullptr;
  }

  for (auto peep = peek(); !peep->is_operator(); peep = peek()) {
    args.push_back(parse_expr(ctx));

    token = advance();
    if (!token->is_operator(Token::Operator::opCOMMA)) {
      break;
    }
  }

  if (!token->is_operator(Token::Operator::opRPAREN)) {
    // TODO: report error.
    return nullptr;
  }

  return std::make_unique<const ast::Call>(name, std::move(args));
}

std::unique_ptr<const ast::Expression> Parser::parse_operand(Context &ctx) {
  auto token = peek();
  switch (token->type()) {
  case Token::Type::tIDENTIFIER:
    return parse_identifier(ctx);
  case Token::Type::tINTEGER:
    return parse_integer(ctx);
  case Token::Type::tOPERATOR:
    if (token->is_operator(Token::Operator::opLPAREN)) {
      return parse_paren_expr(ctx);
    }
  default:
    // TODO: report error
    return nullptr;
  }
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

  return std::make_unique<const ast::Assignment>(std::move(lhs),
                                                 parse_expr(ctx));
}

Parser::Precedence determine_precedence(const Token &token) {
  if (!token.is_operator())
    return Parser::Precedence::INVALID;
  switch (token.op()) {
  case Token::Operator::opSTAR:
  case Token::Operator::opSLASH:
    return Parser::Precedence::MULOP;
  case Token::Operator::opPLUS:
  case Token::Operator::opDASH:
    return Parser::Precedence::ADDOP;
  default:
    return Parser::Precedence::INVALID;
  }
}
std::unique_ptr<const ast::Expression>
Parser::parse_binary_expr(Context &ctx, int expr_precedence,
                          std::unique_ptr<const ast::Expression> lhs) {
  while (true) {
    auto token = peek();

    int tok_precedence = determine_precedence(*token);
    if (tok_precedence < expr_precedence)
      return lhs;

    auto op = advance()->op();
    auto rhs = parse_primary(ctx);
    if (!rhs) {
      // TODO: report error.
      return nullptr;
    }

    token = peek();

    int next_precedence = determine_precedence(*token);
    if (tok_precedence < next_precedence) {
      rhs = parse_binary_expr(ctx, tok_precedence, std::move(rhs));
      if (!rhs) {
        // TODO: report error.
        return nullptr;
      }
    }

    lhs = std::make_unique<const ast::BinaryExpression>(op, std::move(lhs),
                                                        std::move(rhs));
  }

  return lhs;
} // namespace compiler

std::unique_ptr<Token> Parser::advance() {
  if (curr_token_ == next_token_)
    next_token_ = lexer_->lex();

  // auto retval = std::move(curr_token_);
  curr_token_ = std::move(next_token_);
  next_token_ = lexer_->lex();
  return std::move(curr_token_);
}

Token *Parser::peek() const { return next_token_.get(); }

} // namespace compiler
} // namespace lang
