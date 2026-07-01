#include "passes/ConstHoistingFnPass.hpp"
#include "IR.hpp"
#include "LoweringPass.hpp"

#include <ranges>
#include <unordered_map>
#include <vector>

namespace fcc {

namespace {

struct ConstKey {
  Type *type;
  std::uint64_t bits;
};

struct ConstKeyHash {
  size_t operator()(const ConstKey &k) const {
    size_t seed = std::hash<Type *>{}(k.type);
    seed ^= std::hash<std::uint64_t>{}(k.bits) + 0x9e3779b97f4a7c15llu +
            (seed << 6) + (seed >> 2); // yes i kinda ripped this off from boost
    return seed;
  }
};

struct ConstKeyEq {
  bool operator()(const ConstKey &a, const ConstKey &b) const {
    return a.type == b.type && a.bits == b.bits;
  }
};

} // namespace

bool ConstHoistingFnPass::run(Function &fn) {
  std::unordered_map<ConstKey, Instruction *, ConstKeyHash, ConstKeyEq>
      const_pool;

  auto &entry_bb = fn.blcks[0];

  for (auto &bb : fn.blcks) {
    for (auto &instr : bb->instrs) {
      if (instr->op != OpCode::Const)
        continue;

      auto idata = std::get<ConstData>(instr->payload);
      auto key = ConstKey{instr->type, idata.bits};

      if (!const_pool.contains(key)) {
        if (&bb == &entry_bb) {
          const_pool[key] = instr.get();
        } else {
          auto const_instr = std::make_unique<Instruction>(
              OpCode::Const, LoweringPass::next_value_id++);
          const_instr->type = instr->type;
          const_instr->payload =
              ConstData{.bits = std::get<ConstData>(instr->payload).bits};
          auto const_instr_ptr = const_instr.get();
          entry_bb->instrs.insert(entry_bb->instrs.begin(),
                                  std::move(const_instr));
          fn.replace_all_uses(instr.get(), const_instr_ptr);
          const_pool[key] = const_instr_ptr;
        }
      }
    }
  }

  bool changed = false;

  for (auto cconst : std::views::values(const_pool)) {
    auto key =
        ConstKey{cconst->type, std::get<ConstData>(cconst->payload).bits};
    auto entry_cinstr = const_pool[key];

    if (cconst != entry_cinstr) {
      fn.replace_all_uses(cconst, entry_cinstr);
      changed = true;
    }
  }

  return changed;
}

} // namespace fcc
