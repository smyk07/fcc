#include "IR.hpp"
#include "IRPrinter.hpp"
#include "LoweringPass.hpp"
#include "PassRunner.hpp"
#include "SourceFile.hpp"
#include "Utils.hpp"

#include <exception>
#include <print>
#include <vector>

namespace fcc {
bool debug_enabled = false;
} // namespace fcc

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::println("usage: {} [FILE] [--debug|-d] [passes...]", argv[0]);
    return 0;
  }

  std::vector<std::string> passes;

  for (int i = 2; i < argc; i++) {
    std::string_view arg{argv[i]};

    if (arg == "--debug" || arg == "-d") {
      fcc::debug_enabled = true;
    } else {
      passes.emplace_back(arg);
    }
  }

  try {
    fcc::SourceFile file{argv[1]};

    FCC_DEBUG(std::println("Clang AST:"));
    FCC_DEBUG(fcc::dump(file));
    FCC_DEBUG(std::println());

    fcc::LoweringPass lp{file};
    fcc::Module lowered_module = lp.run();

    FCC_DEBUG(std::println("fcc IR:"));
    fcc::dump(lowered_module);

    fcc::PassRunner pr{lowered_module};
    pr.run(passes);
  }

  catch (const std::exception &e) {
    std::println(stderr, "{}", e.what());
    return 1;
  }

  return 0;
}
