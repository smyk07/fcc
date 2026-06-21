#pragma once

#include "IR.hpp"

#include "passes/BaseFnPass.hpp"
#include "passes/DDEFnPass.hpp"
#include "passes/HelloWorldFnPass.hpp"

#include <array>
#include <format>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace fcc {

template <typename Pass> struct PassEntry {
  std::string_view name;
  std::unique_ptr<Pass> (*pass)();
};

template <typename Pass> std::unique_ptr<BaseFnPass> create_pass() {
  return std::make_unique<Pass>();
}

template <typename Pass, std::size_t N> struct PassReg {
  std::array<PassEntry<Pass>, N> entries;

  constexpr PassReg(std::array<PassEntry<Pass>, N> entries)
      : entries(entries) {}

  constexpr auto get_pass(std::string_view name) const
      -> std::unique_ptr<Pass> (*)() {
    for (const auto &entry : entries) {
      if (entry.name == name)
        return entry.pass;
    }
    throw std::out_of_range(std::format("pass {} not found", name));
  }
};

#define REGISTER_PASS(name, pass) {name, &create_pass<pass>}

struct PassRunner {
private:
  const Module &mod;

  static std::vector<std::unique_ptr<BaseFnPass>> fn_passes;
  static constexpr PassReg<BaseFnPass, 2> fn_pass_reg{{{
      REGISTER_PASS("HelloWorld", HelloWorldFnPass),
      REGISTER_PASS("DDE", DDEFnPass),
  }}};

public:
  PassRunner(const Module &mod) : mod{mod} {}

  void run(std::vector<std::string> &passes);
};

} // namespace fcc
