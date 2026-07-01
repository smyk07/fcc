#pragma once

#include "passes/BaseFnPass.hpp"

#include "IR.hpp"

namespace fcc {

/*
 * Constant Folding
 */

struct ConstFold : BaseFnPass {
  bool run(Function &) override;
};

} // namespace fcc
