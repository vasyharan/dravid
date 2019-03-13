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
    uint32_t line_;
    uint32_t col_;

  public:
    Location() : line_(0), col_(0) {}
    Location(uint32_t line, uint32_t col) : line_(line), col_(col) {}

    uint32_t line() const { return line_; }
    uint32_t col() const { return col_; }
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
  static std::unique_ptr<Token> make_op(const Operator op, const Location loc) {
    auto token = make(Type::tOPERATOR);
    token->u_.op = op;
    token->loc_ = loc;
    return token;
  }
  static std::unique_ptr<Token> make_keyword(const Keyword keyword,
                                             const Location loc) {
    auto token = make(Type::tKEYWORD);
    token->u_.keyword = keyword;
    token->loc_ = loc;
    return token;
  }
  static std::unique_ptr<Token>
  make_identifier(std::unique_ptr<std::string> name, const Location loc) {
    auto token = make(Type::tIDENTIFIER);
    token->u_.string = name.release();
    token->loc_ = loc;
    return token;
  }
  static std::unique_ptr<Token> make_string(std::unique_ptr<std::string> name,
                                            const Location loc) {
    auto token = make(Type::tIDENTIFIER);
    token->u_.string = name.release();
    token->loc_ = loc;
    return token;
  }
  static std::unique_ptr<Token> make_integer(const int value,
                                             const Location loc) {
    auto token = make(Type::tINTEGER);
    token->u_.integer = value;
    token->loc_ = loc;
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
  Location loc_;
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
