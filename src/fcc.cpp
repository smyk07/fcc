#include "IR.hpp"
#include "IRPrinter.hpp"
#include "LoweringPass.hpp"
#include "SourceFile.hpp"
#include "Utils.hpp"

#include <exception>
#include <print>

namespace fcc {
bool debug_enabled = false;
}

int main(int argc, char *argv[]) {

  if (argc < 2) {
    std::println("usage: {} [FILE] [--debug]", argv[0]);
    return 0;
  }

  for (int i = 2; i < argc; i++) {
    if (std::string_view(argv[i]) == "--debug" ||
        std::string_view(argv[i]) == "-d")
      fcc::debug_enabled = true;
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

  } catch (const std::exception &e) {
    std::println(stderr, "{}", e.what());
    return 1;
  }

  return 0;
}
