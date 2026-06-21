#pragma once

#include "IR.hpp"

namespace fcc {

struct BaseFnPass {
public:
  virtual ~BaseFnPass() = default;

  virtual bool run(Function &fn) = 0;
};

} // namespace fcc
