#define DOCTEST_CONFIG_IMPLEMENT
#include "compiler/lexer.h"
#include "compiler/parser.h"
#include "config.h"
#include "cxxopts.hpp"
#include "filesystem.h"
#include <fstream>
#include <iostream>
#include <sstream>

namespace lang {
namespace compiler {

class LoggingLexer : public compiler::ILexer {
  compiler::Lexer lexer_;
  std::stringstream outbuf_;
  bool eof_;

public:
  LoggingLexer(const std::string &name, std::istream &in)
      : lexer_{Lexer(name, in)}, eof_(false) {}
  ~LoggingLexer() { finish(); }

  std::unique_ptr<Token> lex() override {
    auto token = lexer_.lex();
    print(*token);
    return token;
  }

  std::vector<std::unique_ptr<Token>> reset() override {
    auto tokens = lexer_.reset();
    for (auto it = tokens.begin(); it != tokens.end(); ++it) {
      print(**it);
    }
    return tokens;
  }

  const std::stringstream &finish() {
    auto token = this->lex();
    while (!token->invalid() && !token->eof()) {
      token = this->lex();
    }
    return outbuf_;
  }

  void print(const Token &token) {
    if (eof_)
      return;
    if (token.eof())
      eof_ = true;

    outbuf_ << token.string() << "\n";
  }
};

std::string with_ext(fs::path path, std::string extension) {
  auto stem = path.stem();
  auto outpath = path.remove_filename();
  outpath /= stem;
  outpath += fs::path(extension);
  return outpath.string();
}

std::stringstream read(std::string path) {
  std::fstream in(path, std::ios::in);
  std::stringstream buf;
  buf << in.rdbuf();
  return buf;
}

void compare(const std::stringstream &buf, const std::string &testname,
             const std::string &testtype, const bool write_output,
             const std::string &snappath, const std::string &outpath) {
  if (read(snappath).str() == buf.str()) {
    std::cout << "PASS: \"" << testname << testtype << "\"\n";
  } else {
    std::cout << "FAIL: \"" << testname << testtype << "\"\n";
  }

  if (write_output) {
    std::fstream out(outpath, std::ios::out);
    out << buf.rdbuf();
  }
}

void run_snapshots(const std::string &dir, const bool write_output = false) {
  for (auto &entry : fs::recursive_directory_iterator(dir)) {
    if (fs::is_directory(entry.path()) ||
        fs::path(".vd") != entry.path().extension()) {
      continue;
    }

    auto testname = fs::relative(entry.path(), fs::path(dir));
    std::fstream in(entry.path().string(), std::ios::in);
    LoggingLexer lexer(entry.path().string(), in);

    Context ctx;
    Parser parser(lexer);
    parser.parse(ctx);

    auto &lexbuf = lexer.finish();
    compare(lexbuf, testname, ".ll", write_output,
            with_ext(entry.path(), ".ll.snap"),
            with_ext(entry.path(), ".ll.out"));

    std::stringstream parsebuf;
    for (auto it = ctx.nodes().begin(); it != ctx.nodes().end(); ++it) {
      parsebuf << **it << "\n";
    }
    parsebuf.flush();
    compare(parsebuf, testname, ".pp", write_output,
            with_ext(entry.path(), ".pp.snap"),
            with_ext(entry.path(), ".pp.out"));
  }
}

} // namespace compiler
} // namespace lang

int main(int argc, char *argv[]) {
  try {
    cxxopts::Options options(argv[0]);
    options.positional_help("TEST...");

    // clang-format off
    options.add_options()
      ("h,help", "Show this message")
      ("t,test", "Run this test", cxxopts::value<std::vector<std::string>>());
    // clang-format on

    options.parse_positional("test");

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
      std::cout << options.help() << std::endl;
      exit(0);
    }

    if (result.count("test")) {
      auto &tests = result["test"].as<std::vector<std::string>>();
      for (const auto &test : tests) {
        lang::compiler::run_snapshots(test, true);
      }
    } else {
      std::cout << options.help() << std::endl;
      exit(1);
    }

  } catch (const cxxopts::OptionException &e) {
    std::cout << "error parsing options: " << e.what() << std::endl;
    exit(1);
  }

  return 0;
}
