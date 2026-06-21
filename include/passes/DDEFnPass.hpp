#pragma once

#include "IR.hpp"
#include "passes/BaseFnPass.hpp"

namespace fcc {

/*
 * DDE: Dead Definitions Elimination
 */

struct DDEFnPass : BaseFnPass {
  bool run(Function &) override;
};

} // namespace fcc
