#pragma once

#include "passes/BaseFnPass.hpp"

#include "IR.hpp"

namespace fcc {

/*
 * Constant Folding
 */

struct ConstantFoldingFnPass : BaseFnPass {
  bool run(Function &) override;
};

} // namespace fcc
