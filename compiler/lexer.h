#ifndef LANG_COMPILER_LEXER_H
#define LANG_COMPILER_LEXER_H

#include "context.h"
#include "token.h"
#include <cassert>
#include <istream>
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace lang {
namespace compiler {
namespace lex {

class Reader {
  const std::string &name_;
  std::istream &in_;

  std::string line_;
  std::string::const_iterator lineit_;
  size_t lineoff_;
  size_t lineno_;

public:
  Reader(const std::string &name, std::istream &in);
  Reader(const Reader &) = delete;
  Reader(Reader &&) = default;
  ~Reader();

  bool good();
  bool require_line();
  Location loc();
  unsigned char read();
  Reader &operator++();
  const std::string &name() const;
};

class ILexer {
public:
  ILexer() {}
  virtual ~ILexer(){};

  virtual std::unique_ptr<Token> lex() = 0;
  virtual std::vector<std::unique_ptr<Token>> reset() = 0;
};

class Lexer final : public ILexer {
  bool require_line();

  Keyword parse_keyword(const std::string &id);
  Operator parse_op();

  std::unique_ptr<Token> gather_identifier();
  std::unique_ptr<Token> gather_numeric();

  std::unique_ptr<Token> check_for_keyword(std::unique_ptr<std::string> id);

  Reader reader_;

public:
  static std::string to_string(const Keyword);
  static std::string to_string(const Operator);

  Lexer(Context &);
  Lexer(const Lexer &) = delete;
  Lexer(Lexer &&) = default;
  ~Lexer();

  std::unique_ptr<Token> lex() override;
  std::vector<std::unique_ptr<Token>> reset() override;
};

} // namespace lex
} // namespace compiler
} // namespace lang

#endif // LANG_COMPILER_LEXER_H
