#ifndef LANG_COMPILER_LEXER_H
#define LANG_COMPILER_LEXER_H

#include <cassert>
#include <istream>
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace lang {
namespace compiler {

class Token final {
public:
  enum Keyword {
    kwDEF = 1,
    kwVAR,
    kwVAL,
  };

  enum Operator {
    opINVALID = -1,
    opLPAREN = 40,
    opRPAREN = 41,
    opSTAR = 42,
    opPLUS = 43,
    opCOMMA = 44,
    opDASH = 45,
    opSLASH = 47,
    opEQUAL = 61,
    opLSQUARE = 91,
    opRSQUARE = 93,
    opLCURLY = 123,
    // opPIPE = 124,
    opRCURLY = 125,
    opCOMPARE = 128,
  };

  enum Type {
    tEOF = -1,
    tINVALID = 0,
    tKEYWORD = 1,
    tIDENTIFIER,
    tSTRING,
    tOPERATOR,
    tCHARACTER,
    tINTEGER,
    tFLOAT
  };

  Token(const Token &) = delete;
  Token(Token &&) = delete;
  ~Token();

  Token &operator=(const Token &);

  bool invalid() const { return type_ == Type::tINVALID; }
  bool eof() const { return type_ == Type::tEOF; }

  bool is_keyword() const { return type_ == Type::tKEYWORD; }
  bool is_keyword(Keyword kw) const { return is_keyword() && u_.keyword == kw; }
  bool is_identifier() const { return type_ == Type::tIDENTIFIER; }
  bool is_integer() const { return type_ == Type::tINTEGER; }
  bool is_operator() const { return type_ == Type::tOPERATOR; }
  bool is_operator(Operator op) const { return is_operator() && u_.op == op; }

  Type type() const { return type_; }
  Keyword keyword() const {
    assert(is_keyword());
    return u_.keyword;
  }
  Operator op() const {
    assert(is_operator());
    return u_.op;
  }
  const std::string identifier() const {
    assert(is_identifier());
    return std::string(*u_.string);
  }
  int64_t integer() const {
    assert(is_integer());
    return u_.integer;
  }

  const std::string string() const;

  static std::unique_ptr<Token> make_invalid() { return make(Type::tINVALID); }
  static std::unique_ptr<Token> make_eof() { return make(Type::tEOF); }
  static std::unique_ptr<Token> make_op(const Operator op) {
    auto token = make(Type::tOPERATOR);
    token->u_.op = op;
    return token;
  }
  static std::unique_ptr<Token> make_keyword(const Keyword keyword) {
    auto token = make(Type::tKEYWORD);
    token->u_.keyword = keyword;
    return token;
  }
  static std::unique_ptr<Token>
  make_identifier(std::unique_ptr<std::string> name) {
    auto token = make(Type::tIDENTIFIER);
    token->u_.string = name.release();
    return token;
  }
  static std::unique_ptr<Token> make_string(std::unique_ptr<std::string> name) {
    auto token = make(Type::tIDENTIFIER);
    token->u_.string = name.release();
    return token;
  }
  static std::unique_ptr<Token> make_integer(const int value) {
    auto token = make(Type::tINTEGER);
    token->u_.integer = value;
    return token;
  }

private:
  static std::unique_ptr<Token> make(const Type type) {
    auto token = new Token(type);
    return std::unique_ptr<Token>(token);
  }
  Token() : type_(Type::tINVALID){};
  Token(const Type type) : type_(type){};
  void copy(const Token &);

  Type type_;
  union {
    Keyword keyword;
    Operator op;
    int64_t integer;
    const std::string *string;
  } u_;
};

class ILexer {
public:
  ILexer() {}
  virtual ~ILexer(){};

  virtual std::unique_ptr<Token> lex() = 0;
  virtual std::vector<std::unique_ptr<Token>> reset() = 0;
};

class Lexer final : public ILexer {
  std::unique_ptr<Token> gather_identifier();
  std::unique_ptr<Token> gather_numeric();
  Token::Operator gather_op();
  bool require_line();

  const std::string name_;
  std::istream &in_;

  std::string line_;
  std::string::const_iterator lineit_;
  size_t lineoff_;
  size_t lineno_;

public:
  static std::string to_string(const Token::Keyword);
  static std::string to_string(const Token::Operator);

  Lexer(const std::string &name, std::istream &in);
  Lexer(const Lexer &) = delete;
  Lexer(Lexer &&) = default;
  ~Lexer();

  std::unique_ptr<Token> lex() override;
  std::vector<std::unique_ptr<Token>> reset() override;
};

} // namespace compiler
} // namespace lang

#endif // LANG_COMPILER_LEXER_H
