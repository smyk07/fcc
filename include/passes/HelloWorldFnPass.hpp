#pragma once

#include "BaseFnPass.hpp"
#include "IR.hpp"

namespace fcc {

struct HelloWorldFnPass : BaseFnPass {
  bool run(Function &) override;
};

} // namespace fcc
