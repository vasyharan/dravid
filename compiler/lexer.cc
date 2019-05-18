#include "lexer.h"
#include <iomanip>
#include <sstream>

namespace lang {
namespace compiler {
namespace lex {

// Static Methods
//------------------------------------------------------------------------------
const std::string to_string(const Keyword keyword) {
  switch (keyword) {
  case Keyword::kwFN:
    return "fn";
  case Keyword::kwIF:
    return "if";
  case Keyword::kwELSE:
    return "else";
  case Keyword::kwELIF:
    return "elif";
  case Keyword::kwVAR:
    return "var";
  case Keyword::kwVAL:
    return "val";
  default:
    return "kwINVALID";
  }
}

const std::string to_string(const Operator op) {
  switch (op) {
  case Operator::opLPAREN:
    return "(";
  case Operator::opRPAREN:
    return ")";
  case Operator::opLSQUARE:
    return "[";
  case Operator::opRSQUARE:
    return "]";
  case Operator::opLCURLY:
    return "{";
  case Operator::opRCURLY:
    return "}";
  case Operator::opCOMMA:
    return ",";
  case Operator::opEQUAL:
    return "=";
  case Operator::opPLUS:
    return "+";
  case Operator::opDASH:
    return "-";
  case Operator::opSTAR:
    return "*";
  case Operator::opSLASH:
    return "/";
  case Operator::opCOMPARE:
    return "==";
  case Operator::opINVALID:
  default:
    return "opINVALID";
  }
}

// Reader
//------------------------------------------------------------------------------
Reader::Reader(const std::string &name, std::istream &in)
    : name_(name), in_(in), line_(), lineit_(line_.cbegin()), lineoff_(0),
      lineno_(0) {}
Reader::~Reader() {}

bool Reader::good() { return lineit_ != line_.cend(); }

Reader &Reader::operator++() {
  ++lineoff_;
  ++lineit_;
  return *this;
}

bool Reader::require_line() {
  while (in_.good() && lineit_ == line_.cend()) {
    std::getline(in_, line_);
    lineit_ = line_.cbegin();
    ++lineno_;
    lineoff_ = 0;
  }

  return lineit_ != line_.cend();
}
unsigned char Reader::read() { return *lineit_; }

Location Reader::loc() { return Location(lineno_, lineoff_); }

const std::string &Reader::name() const { return name_; }

// Lexer
//------------------------------------------------------------------------------
Lexer::Lexer(Context &ctx) : reader_(Reader(ctx.name(), ctx.in())) {}

Lexer::~Lexer() {}

Token::~Token() {
  switch (type_) {
  case Type::tIDENTIFIER:
  case Type::tSTRING:
    delete u_.string;
    break;
  case Type::tCHARACTER:
  case Type::tEOF:
  case Type::tFLOAT:
  case Type::tINTEGER:
  case Type::tINVALID:
  case Type::tKEYWORD:
  case Type::tOPERATOR:
    break;
  }
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
    buf << to_string(u_.keyword);
    break;
  case Type::tIDENTIFIER:
    buf << "id ";
    buf << *u_.string;
    break;
  case Type::tSTRING:
    buf << "str";
    buf << *u_.string;
    break;
  case Type::tOPERATOR:
    buf << "op ";
    buf << to_string(u_.op);
    break;
  case Type::tCHARACTER:
    buf << "char";
    break;
  case Type::tINTEGER:
    buf << "int " << u_.integer;
    break;
  case Type::tFLOAT:
    buf << "float";
    break;
  }

  buf << " " << loc_.line << ":" << loc_.col;

  buf << ')';
  return buf.str();
}

std::vector<std::unique_ptr<Token>> Lexer::reset() {
  std::vector<std::unique_ptr<Token>> tokens;
  while (reader_.good()) {
    tokens.emplace_back(lex());
  }
  return tokens;
}

std::unique_ptr<Token> Lexer::lex() {
  if (!reader_.require_line()) {
    return Token::make_eof();
  }

  while (reader_.good()) {
    Location loc = reader_.loc();
    unsigned char cc = reader_.read();

    switch (cc) {
    case ' ':
    case '\t':
    case '\r':
      ++reader_;
      cc = reader_.read();
      while (cc == ' ' || cc == '\t' || cc == '\r') {
        ++reader_;
        cc = reader_.read();
      }
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
      // clang-format off
    case '0': case '1': case '2': case '3': case '4': case '5': case '6':
    case '7': case '8': case '9':
      return gather_numeric();
      // clang-format off
    case '+': case '-': case '*': case '/':
    case '(': case ')':
    case '{': case '}':
    case '[': case ']':
    case ':': case ';':
    case ',':
    case '=':
      // clang-format on
      return Token::make_op(parse_op(), loc);
    default:
      return Token::make_invalid();
    }
  }
  return Token::make_invalid();
}

Operator Lexer::parse_op() {
  unsigned char op = reader_.read();
  ++reader_; // consume the last char.

  switch (op) {
  case '(':
    return Operator::opLPAREN;
  case ')':
    return Operator::opRPAREN;
  case '[':
    return Operator::opLSQUARE;
  case ']':
    return Operator::opRSQUARE;
  case '{':
    return Operator::opLCURLY;
  case '}':
    return Operator::opRCURLY;
  case ',':
    return Operator::opCOMMA;
  case ':':
    return Operator::opCOLON;
  case '+':
    return Operator::opPLUS;
  case '-':
    return Operator::opDASH;
  case ';':
    return Operator::opSEMICOLON;
  case '*':
    return Operator::opSTAR;
  case '/':
    return Operator::opSLASH;
  case '=': {
    op = reader_.read();
    switch (op) {
    case '=':
      ++reader_; // consume the last =.
      return Operator::opCOMPARE;
    default:
      return Operator::opEQUAL;
    }
  }
  default:
    return Operator::opINVALID;
  }
}

Keyword Lexer::parse_keyword(const std::string &id) {
  if (id == "fn") {
    return Keyword::kwFN;
  } else if (id == "var") {
    return Keyword::kwVAR;
  } else if (id == "val") {
    return Keyword::kwVAL;
  } else if (id == "if") {
    return Keyword::kwIF;
  } else if (id == "else") {
    return Keyword::kwELSE;
  } else if (id == "elif") {
    return Keyword::kwELIF;
  }

  return Keyword::kwINVALID;
}

std::unique_ptr<Token> Lexer::gather_identifier() {
  auto loc = reader_.loc();
  auto buf = std::make_unique<std::string>();

  for (; reader_.good(); ++reader_) {
    unsigned char cc = reader_.read();
    if (cc <= 0x7f) {
      if ((cc < 'A' || cc > 'Z') && (cc < 'a' || cc > 'z') && cc != '_' &&
          (cc < '0' || cc > '9')) {
        if ((cc >= ' ' && cc < 0x7f) || cc == '\t' || cc == '\r' ||
            cc == '\n') {
          break;
        }

        return Token::make_invalid();
      } else {
        buf->push_back(cc);
      }
    } else {
      // TODO: handle non-ascii
      return Token::make_invalid();
    }
  }

  auto keyword = parse_keyword(*buf);
  return keyword == Keyword::kwINVALID
             ? Token::make_identifier(std::move(buf), loc)
             : Token::make_keyword(keyword, loc);
}

std::unique_ptr<Token> Lexer::gather_numeric() {
  std::string buf;
  auto loc = reader_.loc();

  for (; reader_.good(); ++reader_) {
    unsigned char cc = reader_.read();
    if (cc <= 0x7f) {
      if ((cc < '0' || cc > '9')) {
        if ((cc >= ' ' && cc < 0x7f) || cc == '\t' || cc == '\r' ||
            cc == '\n') {
          break;
        }

        return Token::make_invalid();
      } else {
        buf.push_back(cc);
      }
    } else {
      return Token::make_invalid();
    }
  }

  return Token::make_integer(std::stoi(buf), loc);
}

} // namespace lex
} // namespace compiler
} // namespace lang
