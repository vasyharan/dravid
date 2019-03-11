#define DOCTEST_CONFIG_IMPLEMENT
#include "compiler/lexer.h"
#include "config.h"
#include "cxxopts.hpp"
#include "filesystem.h"
#include <fstream>
#include <iostream>
#include <sstream>

void run_snapshots(const std::string &dir, const bool write_output = false) {
  lang::fs::path dirpath(dir);
  for (auto &entry : lang::fs::recursive_directory_iterator(dir)) {
    auto inpath = entry.path();

    auto extension = inpath.extension();
    if (lang::fs::is_directory(entry.path()) ||
        extension == lang::fs::path(".snap") ||
        extension == lang::fs::path(".out")) {
      continue;
    }

    auto snappath = lang::fs::path(inpath).remove_filename();
    snappath /= inpath.stem();
    snappath += lang::fs::path(".lex.snap");

    std::fstream in(inpath.string(), std::ios::in);
    lang::compiler::Lexer lexer(inpath.string(), in);

    std::stringstream outbuf;
    auto token = lexer.lex();
    while (!token.invalid() && !token.eof()) {
      outbuf << token.string() << "\n";
      token = lexer.lex();
    }
    outbuf << token.string() << "\n";

    std::fstream snap(snappath.string(), std::ios::in);
    std::stringstream snapbuf;
    snapbuf << snap.rdbuf();

    auto relpath = lang::fs::relative(inpath.string(), dirpath);
    if (snapbuf.str() == outbuf.str()) {
      std::cout << "PASS: " << relpath.string() << "\n";
    } else {
      std::cout << "FAIL: " << lang::fs::relative(inpath.string(), dirpath)
                << "\n";
    }

    if (write_output) {
      auto outpath = lang::fs::path(inpath).remove_filename();
      outpath /= inpath.stem();
      outpath += lang::fs::path(".lex.out");

      std::fstream out(outpath.string(), std::ios::out);
      out << outbuf.rdbuf();
    }
  }
}

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
        run_snapshots(test, true);
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
