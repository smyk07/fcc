#include "PassRunner.hpp"
#include "Utils.hpp"

#include <cstdint>
#include <print>

namespace fcc {

void PassRunner::run(std::vector<std::string> &passes) {
  std::uint64_t run_count = 0;

  for (auto &fn : mod.funcs) {
    for (auto pass : passes) {
      try {
        run_count += fn_pass_reg.get_pass(pass)()->run(*fn);
      }

      catch (const std::out_of_range &e) {
        throw_error(e.what());
      }
    }
  }

  FCC_DEBUG(
      std::println("{}/{} passes run", run_count, fn_pass_reg.entries.size()));
}

} // namespace fcc
