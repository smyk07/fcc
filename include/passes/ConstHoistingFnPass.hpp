#pragma once

#include "IR.hpp"
#include "passes/BaseFnPass.hpp"

namespace fcc {

struct ConstHoistingFnPass : BaseFnPass {
  bool run(Function &) override;
};

} // namespace fcc
