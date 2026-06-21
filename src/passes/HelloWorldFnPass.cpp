#include "passes/HelloWorldFnPass.hpp"
#include "IR.hpp"

#include <print>

namespace fcc {

bool HelloWorldFnPass::run(Function &fn) {
  std::println("Hello, {}", fn.name);
  return true;
}

} // namespace fcc
