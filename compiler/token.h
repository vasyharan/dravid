#ifndef LANG_COMPILER_TOKEN_H
#define LANG_COMPILER_TOKEN_H

#include <cassert>
#include <cstdint>
#include <memory>

namespace lang {
namespace compiler {

class Token final {
public:
  struct Location {
    const uint32_t line;
    const uint32_t col;

    Location() : line(0), col(0) {}
    Location(uint32_t l, uint32_t c) : line(l), col(c) {}
  };

  enum Keyword {
    kwINVALID = -1,
    kwFN = 1,
    kwVAR,
    kwVAL,
    kwIF,
    kwELSE,
    kwELIF,
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
    opCOLON = 58,
    opSEMICOLON = 59,
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

  Token(const Type type, const Location loc) : type_(type), loc_(loc){};
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

  static std::unique_ptr<Token> make_invalid() {
    // TODO: introduce a constant here
    return make(Type::tINVALID, Location());
  }
  static std::unique_ptr<Token> make_eof() {
    // TODO: introduce a constant here
    return make(Type::tEOF, Location());
  }
  static std::unique_ptr<Token> make_op(const Operator op, const Location loc) {
    auto token = make(Type::tOPERATOR, loc);
    token->u_.op = op;
    return token;
  }
  static std::unique_ptr<Token> make_keyword(const Keyword keyword,
                                             const Location loc) {
    auto token = make(Type::tKEYWORD, loc);
    token->u_.keyword = keyword;
    return token;
  }
  static std::unique_ptr<Token>
  make_identifier(std::unique_ptr<std::string> name, const Location loc) {
    auto token = make(Type::tIDENTIFIER, loc);
    token->u_.string = name.release();
    return token;
  }
  static std::unique_ptr<Token> make_string(std::unique_ptr<std::string> name,
                                            const Location loc) {
    auto token = make(Type::tIDENTIFIER, loc);
    token->u_.string = name.release();
    return token;
  }
  static std::unique_ptr<Token> make_integer(const int value,
                                             const Location loc) {
    auto token = make(Type::tINTEGER, loc);
    token->u_.integer = value;
    return token;
  }

  static std::unique_ptr<Token> make(const Type type, const Location loc) {
    std::unique_ptr<Token> token = std::make_unique<Token>(type, loc);
    return token;
  }

private:
  const Type type_;
  const Location loc_;
  union {
    Keyword keyword;
    Operator op;
    int64_t integer;
    const std::string *string;
  } u_;
};

} // namespace compiler
} // namespace lang

#endif // LANG_COMPILER_TOKEN_H
