#define DOCTEST_CONFIG_IMPLEMENT
#include "compiler/codegen.h"
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

class LoggingLexer : public lex::ILexer {
  lex::Lexer lexer_;
  std::stringstream outbuf_;
  bool eof_;

public:
  LoggingLexer(Context &ctx) : lexer_{lex::Lexer(ctx)}, eof_(false) {}
  ~LoggingLexer() { finish(); }

  std::unique_ptr<lex::Token> lex() override {
    auto token = lexer_.lex();
    print(*token);
    return token;
  }

  std::vector<std::unique_ptr<lex::Token>> reset() override {
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

  void print(const lex::Token &token) {
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

void compare(const std::string &buf, const std::string &testname,
             const std::string &testtype, const bool write_output,
             const std::string &snappath, const std::string &outpath) {
  if (read(snappath).str() == buf) {
    std::cout << "PASS: \"" << testname << testtype << "\"\n";
  } else {
    std::cout << "FAIL: \"" << testname << testtype << "\"\n";
  }

  if (write_output) {
    std::fstream out(outpath, std::ios::out);
    out << buf;
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

    GlobalContext gctx;
    Context ctx(gctx, entry.path().string(), in);

    {
      LoggingLexer lexer(ctx);
      Parser parser(lexer, ctx);
      parser.parse();

      auto &lexbuf = lexer.finish();
      compare(lexbuf.str(), testname, ".ll", write_output,
              with_ext(entry.path(), ".ll.snap"),
              with_ext(entry.path(), ".ll.out"));
    }

    {
      std::stringstream parsebuf;
      ctx.each_expr([&parsebuf](const ast::Expression &node) -> void {
        parsebuf << node << "\n";
      });
      parsebuf.flush();

      compare(parsebuf.str(), testname, ".pp", write_output,
              with_ext(entry.path(), ".pp.snap"),
              with_ext(entry.path(), ".pp.out"));
    }

    {
      std::stringstream cfgbuf;
      ctx.each_block([&cfgbuf](const cfg::BasicBlock &block) -> void {
        cfgbuf << block << "\n";
      });
      cfgbuf.flush();

      compare(cfgbuf.str(), testname, ".cfg", write_output,
              with_ext(entry.path(), ".cfg.snap"),
              with_ext(entry.path(), ".cfg.out"));
    }

    if (ctx.good()) {
      codegen::Codegen codegen(ctx);
      codegen.generate();

      std::string codestr;
      llvm::raw_string_ostream codebuf(codestr);
      codegen.module().print(codebuf, nullptr);
      compare(codebuf.str(), testname, ".cg", write_output,
              with_ext(entry.path(), ".cg.snap"),
              with_ext(entry.path(), ".cg.out"));
    }

    {
      std::stringstream errorbuf;
      ctx.each_error([&errorbuf](const err::Error &err) -> void {
        errorbuf << err << "\n";
      });
      errorbuf.flush();

      compare(errorbuf.str(), testname, ".err", write_output,
              with_ext(entry.path(), ".err.snap"),
              with_ext(entry.path(), ".err.out"));
    }
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
      ("w,write-output", "Write output files")
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
      auto write_output =
          result.count("write-output") && result["write-output"].as<bool>();
      for (const auto &test : tests) {
        lang::compiler::run_snapshots(test, write_output);
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
