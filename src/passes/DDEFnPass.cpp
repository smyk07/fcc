#include "passes/DDEFnPass.hpp"
#include "IR.hpp"

#include <unordered_set>

namespace fcc {

/*
 * DDE : Dead Definitions Elimination
 */

static bool is_critical(const Instruction &instr) {
  switch (instr.op) {
  case OpCode::Ret:
  case OpCode::CondBr:
  case OpCode::Jmp:
    return true;

  default:
    return false;
  }
}

bool DDEFnPass::run(Function &fn) {
  std::vector<Instruction *> worklist;
  std::unordered_set<Instruction *> live;

  auto mark_live = [&](Value *val) {
    if (val->kind != ValueKind::Instr)
      return;

    auto *def = static_cast<Instruction *>(val);

    if (!live.contains(def)) {
      worklist.push_back(def);
      live.insert(def);
    }
  };

  /*
   * instrs which can never be removed
   */
  for (auto &bb : fn.blcks) {
    for (auto &instr : bb->instrs) {
      if (is_critical(*instr)) {
        worklist.push_back(instr.get());
        live.insert(instr.get());
      }
    }
  }

  /*
   * pop one instr
   * -> visit every value
   * -> mark the instr defining each used value as live
   */
  while (!worklist.empty()) {
    auto *instr = worklist.back();
    worklist.pop_back();

    for (auto *operand : instr->operands)
      mark_live(operand);

    // special cases
    switch (instr->op) {
    case OpCode::CondBr: {
      auto &data = std::get<CondBrData>(instr->payload);
      mark_live(data.cond);
      break;
    }

    case OpCode::Phi: {
      auto &data = std::get<PhiData>(instr->payload);
      for (auto &[pred, val] : data.incoming)
        mark_live(val);
      break;
    }

    default:
      break;
    }
  }

  bool changed = false;

  // every instr not present in live is now dead, so remove it
  for (auto &bb : fn.blcks) {
    auto &instrs = bb->instrs;
    auto old_size = instrs.size();

    std::erase_if(instrs,
                  [&](auto &instr) { return !live.contains(instr.get()); });

    changed |= instrs.size() != old_size;
  }

  return changed;
}

} // namespace fcc
