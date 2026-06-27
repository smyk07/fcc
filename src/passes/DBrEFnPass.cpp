/*
 * DBrE: Dead Branch Elimination
 *
 * Eliminate always-true or always-false conditional branches, replace them with
 * jumps
 */

#include "passes/DBrEFnPass.hpp"
#include "IR.hpp"
#include <vector>

namespace fcc {

namespace {

Instruction *as_const_instr(Value *v) {
  if (v->kind != ValueKind::Instr)
    return nullptr;

  auto *instr = static_cast<Instruction *>(v);

  if (instr->op != OpCode::Const)
    return nullptr;

  return instr;
}

} // namespace

bool DBrEFnPass::run(Function &fn) {
  bool changed = false;

  for (auto &bb : fn.blcks) {
    if (bb->instrs.empty())
      continue;

    if (bb->instrs.back()->op != OpCode::CondBr)
      continue;

    auto &br = bb->instrs.back();
    auto &br_data = std::get<CondBrData>(br->payload);

    auto cond_instr = as_const_instr(br_data.cond);
    if (!cond_instr)
      continue;

    auto bits = std::get<ConstData>(cond_instr->payload).bits;

    BasicBlock *jmp_target = nullptr;
    BasicBlock *live_target = nullptr;
    BasicBlock *dead_target = nullptr;

    if (bits > 0) {
      jmp_target = live_target = br_data.true_target;
      dead_target = br_data.false_target;
    } else {
      jmp_target = live_target = br_data.false_target;
      dead_target = br_data.true_target;
    }

    br->op = OpCode::Jmp;
    br->operands.clear();
    br->payload = JmpData{.target = jmp_target};

    std::erase(dead_target->preds, bb.get());

    for (auto &di : dead_target->instrs) {
      if (di->op != OpCode::Phi)
        continue;

      auto incoming = std::get<PhiData>(di->payload).incoming;

      std::erase_if(incoming,
                    [&bb](auto pair) { return pair.first == bb.get(); });
    }

    changed = true;
  }

  return changed;
}

} // namespace fcc
