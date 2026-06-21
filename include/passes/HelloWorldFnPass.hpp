#pragma once

#include "IR.hpp"
#include "passes/BaseFnPass.hpp"

namespace fcc {

struct HelloWorldFnPass : BaseFnPass {
  bool run(Function &) override;
};

} // namespace fcc
