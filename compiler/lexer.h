#ifndef COMPILER_LEXER_H
#define COMPILER_LEXER_H

#include <istream>
#include <string>

namespace lang {
namespace compiler {

class Token final {
public:
  enum Keyword {
    kwDEF = 1,
    kwVAR,
    kwVAL,
  };

  enum Type {
    tINVALID = -2,
    tEOF = -1,
    tKEYWORD = 1,
    tIDENTIFIER,
    tSTRING,
    tOPERATOR,
    tCHARACTER,
    tINTEGER,
    tFLOAT
  };

  Token(const Token &t) : type_(t.type_) {}
  ~Token() {}

  Token &operator=(const Token &);

  bool invalid() const { return type_ == Type::tINVALID; }
  bool eof() const { return type_ == Type::tEOF; }
  const std::string string() const;

  static Token make_invalid() { return Token(Type::tINVALID); }
  static Token make_eof() { return Token(Type::tEOF); }
  static Token make_keyword(const Keyword keyword) {
    Token token(Type::tKEYWORD);
    token.u_.keyword = keyword;
    return token;
  }
  static Token make_identifier(const std::string &name) {
    Token token(Type::tIDENTIFIER);
    token.u_.identifier_value = new std::string(name);
    return token;
  }
  static Token make_string(const std::string &name) {
    Token token(Type::tSTRING);
    token.u_.identifier_value = new std::string(name);
    return token;
  }

private:
  Token() : type_(Type::tINVALID){};
  Token(const Type type) : type_(type){};

  Type type_;
  union {
    Keyword keyword;
    std::string *identifier_value;
    std::string *string_value;
  } u_;
};

class Lexer final {

public:
  Lexer(const std::string &name, std::istream &in);
  Lexer(const Lexer &) = delete;
  Lexer(Lexer &&) = delete;
  ~Lexer();

  Token lex();

private:
  Token gather_identifier();
  bool require_line();

  const std::string name_;
  std::istream &in_;

  std::string line_;
  std::string::const_iterator lineit_;
  size_t lineoff_;
  size_t lineno_;
};

} // namespace compiler
} // namespace lang

#endif // COMPILER_LEXER_H
