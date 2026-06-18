#include "IR.hpp"
#include "IRPrinter.hpp"
#include "LoweringPass.hpp"
#include "SourceFile.hpp"

#include <exception>
#include <print>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::println("usage: {} [FILE]", argv[0]);
    return 0;
  }

  try {
    fcc::SourceFile file{argv[1]};

    std::println("Clang AST:");
    fcc::dump(file);
    std::println();

    fcc::LoweringPass lp{file};
    fcc::Module lowered_module = lp.run();

    std::println("fcc IR:");
    fcc::dump(lowered_module);

  } catch (const std::exception &e) {
    std::println(stderr, "{}", e.what());
    return 1;
  }

  return 0;
}
