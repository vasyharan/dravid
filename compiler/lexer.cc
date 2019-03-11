#include "lexer.h"
#include <iomanip>
#include <sstream>

namespace lang {
namespace compiler {

Token::Operator char_to_op(const char op) {
  switch (op) {
  case '(':
    return Token::Operator::opLPAREN;
  case ')':
    return Token::Operator::opRPAREN;
  case '[':
    return Token::Operator::opLSQUARE;
  case ']':
    return Token::Operator::opRSQUARE;
  case '{':
    return Token::Operator::opLCURLY;
  case '}':
    return Token::Operator::opRCURLY;
  default:
    return Token::Operator::opINVALID;
  }
}

Token check_for_keywords(const std::string &id) {
  if (id == "def") {
    return Token::make_keyword(Token::Keyword::kwDEF);
  } else if (id == "var") {
    return Token::make_keyword(Token::Keyword::kwVAR);
  } else if (id == "val") {
    return Token::make_keyword(Token::Keyword::kwVAL);
  }
  return Token::make_identifier(id);
}

const std::string keyword_to_string(const Token::Keyword keyword) {
  switch (keyword) {
  case Token::Keyword::kwDEF:
    return "def";
  case Token::Keyword::kwVAR:
    return "var";
  case Token::Keyword::kwVAL:
    return "val";
  }
}

const std::string op_to_string(const Token::Operator op) {
  switch (op) {
  case Token::Operator::opLPAREN:
    return "(";
  case Token::Operator::opRPAREN:
    return ")";
  case Token::Operator::opLSQUARE:
    return "[";
  case Token::Operator::opRSQUARE:
    return "]";
  case Token::Operator::opLCURLY:
    return "{";
  case Token::Operator::opRCURLY:
    return "}";
  case Token::Operator::opINVALID:
  default:
    return "INVALID";
  }
}

Lexer::Lexer(const std::string &name, std::istream &in)
    : name_(name), in_(in), line_(), lineit_(line_.cbegin()), lineoff_(0),
      lineno_(0) {}

Lexer::~Lexer() {}

Token &Token::operator=(const Token &o) {
  this->type_ = o.type_;
  switch (type_) {
  case Type::tKEYWORD:
    this->u_.keyword = o.u_.keyword;
    break;
  case Type::tIDENTIFIER:
    this->u_.identifier_value = o.u_.identifier_value;
    break;
  case Type::tSTRING:
    this->u_.string_value = o.u_.string_value;
    break;
  case Type::tINVALID:
  case Type::tEOF:
  case Type::tOPERATOR:
    this->u_.op = o.u_.op;
  case Type::tCHARACTER:
  case Type::tINTEGER:
  case Type::tFLOAT:
    /* nothing */
    break;
  }
  return *this;
}

const std::string Token::string() const {
  std::stringstream buf;
  buf << '(';
  switch (type_) {
  case Type::tINVALID:
    buf << "invalid";
    break;
  case Type::tEOF:
    buf << "eof";
    break;
  case Type::tKEYWORD:
    buf << "keyword ";
    buf << keyword_to_string(u_.keyword);
    break;
  case Type::tIDENTIFIER:
    buf << "id ";
    buf << (*u_.string_value);
    break;
  case Type::tSTRING:
    buf << "str";
    break;
  case Type::tOPERATOR:
    buf << "op ";
    buf << op_to_string(u_.op);
    break;
  case Type::tCHARACTER:
    buf << "char";
    break;
  case Type::tINTEGER:
    buf << "int";
    break;
  case Type::tFLOAT:
    buf << "float";
    break;
  }

  buf << ')';
  return buf.str();
}

Token Lexer::lex() {
  if (!require_line()) {
    return Token::make_eof();
  }

  while (lineit_ != line_.cend()) {
    unsigned char cc = *lineit_;
    switch (cc) {
    case ' ':
    case '\t':
    case '\r':
      ++lineit_;
      while (*lineit_ == ' ' || *lineit_ == '\t' || *lineit_ == '\r')
        ++lineit_;
      break;
      // clang-format off
    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
    case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
    case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
    case 'V': case 'W': case 'X': case 'Y': case 'Z': case 'a': case 'b':
    case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i':
    case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p':
    case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w':
    case 'x': case 'y': case 'z': case '_':
      // clang-format on
      return gather_identifier();
      // clang-format off
    case '(': case ')':
    case '{': case '}':
    case '[': case ']':
      // clang-format on
      ++lineit_; // consume the last char.
      return Token::make_op(char_to_op(cc));
    }
  }
  return Token::make_invalid();
}

Token Lexer::gather_identifier() {
  std::string buf;
  for (; lineit_ != line_.cend(); ++lineit_) {
    unsigned char cc = *lineit_;
    if (cc <= 0x7f) {
      if ((cc < 'A' || cc > 'Z') && (cc < 'a' || cc > 'z') && cc != '_' &&
          (cc < '0' || cc > '9')) {
        if ((cc >= ' ' && cc < 0x7f) || cc == '\t' || cc == '\r' ||
            cc == '\n') {
          break;
        }

        return Token::make_invalid();
      } else {
        buf.push_back(cc);
      }
    } else {
      // TODO: handle non-ascii
      return Token::make_invalid();
    }
  }

  return check_for_keywords(buf);
}

bool Lexer::require_line() {
  if (lineit_ != line_.cend()) {
    return true;
  }

  if (in_.good()) {
    std::getline(in_, line_);
    lineit_ = line_.cbegin();
    ++lineno_;
    lineoff_ = 1;
    return true;
  }

  return false;
}

} // namespace compiler
} // namespace lang
