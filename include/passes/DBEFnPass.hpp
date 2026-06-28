/*
 * DBE: Dead Block Elimination
 *
 * Eliminates unreachable blocks
 */

#pragma once

#include "IR.hpp"
#include "passes/BaseFnPass.hpp"

namespace fcc {

struct DBEFnPass : BaseFnPass {
  bool run(Function &) override;
};

} // namespace fcc
