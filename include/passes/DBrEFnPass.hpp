/*
 * DBrE: Dead Branch Elimination
 *
 * Eliminate always-true or always-false conditional branches, replace them with
 * jumps
 */

#pragma once

#include "IR.hpp"
#include "passes/BaseFnPass.hpp"

namespace fcc {

struct DBrEFnPass : BaseFnPass {
  bool run(Function &) override;
};

} // namespace fcc
