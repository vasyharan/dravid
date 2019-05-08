#include "cfg.h"
#include "parser.h"
#include <cassert>
#include <memory>

namespace lang {
namespace compiler {

// Location UNKNOWN_LOC = Location();

Parser::Parser(ILexer &lexer, Context &ctx)
    : _ctx(ctx), _lexer(lexer), _curr_token(nullptr), _next_token(nullptr) {}

Parser::~Parser() {}

void Parser::parse() {
  _next_token = _lexer.lex();
  auto peep = peek();
  while (!peep->eof()) {
    if (!peep->is_keyword()) {
      _ctx.report_error(err::unexpected_token(*peep));
      break;
    }

    switch (peep->keyword()) {
    case Token::Keyword::kwFN:
      if (auto fn = parse_fn()) {
        _ctx.push_node(std::move(fn));
      }
    default:
      // TODO: report error.
      break;
    }

    peep = peek();
  }

  cfg::CFGParser::parse_into(_ctx);
}

std::shared_ptr<const ast::Function> Parser::parse_fn() {
  auto token = advance();
  if (!token->is_keyword(Token::Keyword::kwFN)) {
    _ctx.report_error(err::unexpected_token(*token, "Expected `fn'"));

    return nullptr;
  }

  auto prototype = parse_prototype();
  if (!prototype)
    return nullptr;

  auto body = parse_fn_body();

  return std::make_shared<const ast::Function>(std::move(prototype),
                                               std::move(body));
}

std::shared_ptr<const ast::Prototype> Parser::parse_prototype() {
  auto token = advance();
  if (!token->is_identifier()) {
    _ctx.report_error(err::unexpected_token(*token, "Expected fn name"));
    return nullptr;
  }

  const std::string name = token->identifier();
  auto params = parse_parameters();

  return std::make_shared<const ast::Prototype>(name, std::move(params));
}

std::vector<std::shared_ptr<const ast::Parameter>> Parser::parse_parameters() {
  std::vector<std::shared_ptr<const ast::Parameter>> params;

  auto token = advance();
  if (!token->is_operator(Token::Operator::opLPAREN)) {
    _ctx.report_error(err::unexpected_token(*token, "Expected params '('"));
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
    _ctx.report_error(err::unexpected_token(*token, "Expected params ')'"));
    return params;
  }

  return params;
}

std::vector<std::shared_ptr<const ast::Expression>> Parser::parse_fn_body() {
  std::vector<std::shared_ptr<const ast::Expression>> body;

  auto token = advance();
  if (!token->is_operator(Token::Operator::opEQUAL)) {
    _ctx.report_error(err::unexpected_token(*token, "Expected fn '='"));
    return body;
  }

  gather_block(body);
  return body;
}

std::shared_ptr<const ast::Expression> Parser::parse_stmt() {
  switch (peek()->type()) {
  case Token::Type::tKEYWORD:
    switch (peek()->keyword()) {
    case Token::Keyword::kwVAL:
      return parse_decl();
    case Token::Keyword::kwIF:
      return parse_if();
    default:
      return parse_expr();
    }
  default:
    return parse_expr();
  }
}

std::shared_ptr<const ast::Expression> Parser::parse_if() {
  auto token = advance();
  if (!token->is_keyword(Token::Keyword::kwIF) &&
      !token->is_keyword(Token::Keyword::kwELIF)) {
    _ctx.report_error(err::unexpected_token(*token, "Expected `if' or `elif'"));
    return nullptr;
  }

  auto cond = parse_expr();
  std::vector<std::shared_ptr<const ast::Expression>> thn;
  std::vector<std::shared_ptr<const ast::Expression>> els;

  gather_block(thn);

  if (peek()->is_keyword(Token::Keyword::kwELSE)) {
    advance(); // eat 'else'
    gather_block(els);
  } else if (peek()->is_keyword(Token::Keyword::kwELIF)) {
    auto expr = parse_if();
    if (expr != nullptr) {
      els.push_back(std::move(expr));
    }
  }

  return std::make_unique<ast::If>(std::move(cond), std::move(thn),
                                   std::move(els));
}

void Parser::gather_block(
    std::vector<std::shared_ptr<const ast::Expression>> &body) {
  if (!peek()->is_operator(Token::Operator::opLCURLY)) {
    auto expr = parse_stmt();
    if (expr != nullptr) {
      body.push_back(std::move(expr));
    }
    return;
  } else {
    advance(); // eat '{'
  }

  for (auto expr = parse_stmt(); expr != nullptr; expr = parse_stmt()) {
    if (expr != nullptr) {
      body.push_back(std::move(expr));
    } else {
      break;
    }

    if (peek()->is_operator(Token::Operator::opRCURLY)) {
      break;
    }
  }

  auto token = advance();
  if (!token->is_operator(Token::Operator::opRCURLY)) {
    _ctx.report_error(err::unexpected_token(*token, "Expected fn '}'"));
  }
}

std::shared_ptr<const ast::Expression> Parser::parse_decl() {
  auto token = advance();
  if (!token->is_keyword(Token::Keyword::kwVAL)) {
    _ctx.report_error(err::unexpected_token(*token, "Expected `val'"));
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
    _ctx.report_error(err::unexpected_token(*token, "Expected `='"));
    return nullptr;
  }

  std::vector<std::shared_ptr<const ast::Expression>> values;
  for (auto it = names.begin(); it != names.end(); ++it) {
    values.push_back(parse_expr());

    if (!peek()->is_operator(Token::Operator::opCOMMA)) {
      break;
    }
  }

  if (names.size() != values.size()) {
    _ctx.report_error(err::unexpected_token(
        *token, "num of declarations: " + std::to_string(names.size()) +
                    "; does not match initialization: " +
                    std::to_string(values.size())));
    return nullptr;
  }

  if (names.size() == 1) {
    return std::make_unique<const ast::Value>(true, names[0],
                                              std::move(values[0]));
  } else {
    _ctx.report_error(
        err::unexpected_token(*token, "NOT IMPLEMENTED: tuple assignment"));
    return nullptr;
  }
}

std::shared_ptr<const ast::Expression> Parser::parse_expr() {
  auto lhs = parse_primary();
  if (!lhs) {
    return nullptr;
  }

  auto token = peek();
  if (!token->is_operator()) {
    _ctx.report_error(
        err::unexpected_token(*token, "Expected binary operator"));
    return lhs;
  }

  switch (token->op()) {
  // case Token::Operator::opEQUAL:
  //   return parse_assign(ctx, std::move(lhs));
  default:
    return parse_binary_expr(Precedence::NORMAL, std::move(lhs));
  }
}

std::shared_ptr<const ast::Expression> Parser::parse_primary() {
  auto peep = peek();
  switch (peep->type()) {
  case Token::Type::tIDENTIFIER: {
    auto token = advance();
    peep = peek();
    if (peep->is_operator(Token::Operator::opLPAREN)) {
      return parse_call(token->identifier());
    } else {
      return std::make_unique<ast::Identifier>(token->identifier());
    }
  }
  case Token::Type::tINTEGER:
    return parse_integer();
  case Token::Type::tOPERATOR:
    if (peep->is_operator(Token::Operator::opLPAREN)) {
      return parse_paren_expr();
    }
  default:
    // TODO: report error
    // ctx.report_error(err::unexpected_token(*peep, "Expected token type"));
    return nullptr;
  }
}

std::shared_ptr<const ast::Expression>
Parser::parse_call(const std::string &name) {
  std::vector<std::shared_ptr<const ast::Expression>> args;

  auto token = advance();
  if (!token->is_operator(Token::Operator::opLPAREN)) {
    _ctx.report_error(err::unexpected_token(*token, "Expected call '('"));
    return nullptr;
  }

  for (auto peep = peek(); !peep->is_operator(); peep = peek()) {
    args.push_back(parse_expr());

    token = advance();
    if (!token->is_operator(Token::Operator::opCOMMA)) {
      break;
    }
  }

  if (!token->is_operator(Token::Operator::opRPAREN)) {
    _ctx.report_error(err::unexpected_token(*token, "Expected call ')'"));
    return nullptr;
  }

  return std::make_unique<const ast::Call>(name, std::move(args));
}

std::shared_ptr<const ast::Expression> Parser::parse_operand() {
  auto token = peek();
  switch (token->type()) {
  case Token::Type::tIDENTIFIER:
    return parse_identifier();
  case Token::Type::tINTEGER:
    return parse_integer();
  case Token::Type::tOPERATOR:
    if (token->is_operator(Token::Operator::opLPAREN)) {
      return parse_paren_expr();
    }
  default:
    _ctx.report_error(err::unexpected_token(*token, "Expected operand"));
    return nullptr;
  }
}

std::shared_ptr<const ast::Identifier> Parser::parse_identifier() {
  assert(peek()->is_identifier());
  auto token = advance();
  return std::make_unique<ast::Identifier>(token->identifier());
}

std::shared_ptr<const ast::Integer> Parser::parse_integer() {
  assert(peek()->is_integer());
  auto token = advance();
  return std::make_unique<ast::Integer>(token->integer());
}

std::shared_ptr<const ast::Expression> Parser::parse_paren_expr() {
  if (!peek()->is_operator(Token::Operator::opLPAREN)) {
    _ctx.report_error(
        err::unexpected_token(*peek(), "Expected paren expr '('"));
    return nullptr;
  }
  advance();

  auto expr = parse_expr();
  if (!expr)
    return nullptr;

  if (!peek()->is_operator(Token::Operator::opRPAREN)) {
    _ctx.report_error(
        err::unexpected_token(*peek(), "Expected paren expr ')'"));
    return nullptr;
  }
  advance();

  return expr;
}
std::shared_ptr<const ast::Expression>
Parser::parse_assign(std::shared_ptr<const ast::Expression> lhs) {
  auto token = advance();
  if (!token->is_operator(Token::Operator::opEQUAL)) {
    _ctx.report_error(err::unexpected_token(*token, "Expected '='"));
  }

  return std::make_unique<const ast::Assignment>(std::move(lhs), parse_expr());
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
std::shared_ptr<const ast::Expression>
Parser::parse_binary_expr(int expr_precedence,
                          std::shared_ptr<const ast::Expression> lhs) {
  while (true) {
    auto token = peek();

    int tok_precedence = determine_precedence(*token);
    if (tok_precedence < expr_precedence)
      return lhs;

    auto op = advance()->op();
    auto rhs = parse_primary();
    if (!rhs) {
      return nullptr;
    }

    token = peek();

    int next_precedence = determine_precedence(*token);
    if (tok_precedence < next_precedence) {
      rhs = parse_binary_expr(tok_precedence, std::move(rhs));
      if (!rhs) {
        return nullptr;
      }
    }

    lhs = std::make_unique<const ast::BinaryExpression>(op, std::move(lhs),
                                                        std::move(rhs));
  }

  return lhs;
}

std::unique_ptr<Token> Parser::advance() {
  if (_curr_token == _next_token)
    _next_token = _lexer.lex();

  // auto retval = std::move(_curr_token);
  _curr_token = std::move(_next_token);
  _next_token = _lexer.lex();
  return std::move(_curr_token);
}

Token *Parser::peek() const { return _next_token.get(); }

} // namespace compiler
} // namespace lang
