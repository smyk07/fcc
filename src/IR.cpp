#include "IR.hpp"
#include "Utils.hpp"

#include <algorithm>
#include <clang-c/Index.h>

#include <cassert>

namespace fcc {

std::vector<BasicBlock *> BasicBlock::successors() const {
  if (instrs.empty()) {
    return {};
  }

  auto *term = instrs.back().get();

  switch (term->op) {
  case OpCode::Jmp: {
    auto &data = std::get<JmpData>(term->payload);
    return {data.target};
  }

  case OpCode::CondBr: {
    auto &data = std::get<CondBrData>(term->payload);
    return {data.true_target, data.false_target};
  }

  case OpCode::Ret:
    return {};

  default:
    throw_error("BasicBlock does not end in a terminator");
    return {};
  }
}

std::vector<BasicBlock *> BasicBlock::predecessors(Function *fn) const {
  std::vector<BasicBlock *> preds;

  for (auto &b : fn->blcks) {
    auto succs = b.get()->successors();

    for (auto *s : succs) {
      if (s == this) {
        preds.push_back(b.get());
        break;
      }
    }
  }

  return preds;
}

Instruction::Instruction(OpCode op) : op{op} {
  kind = ValueKind::Instr;

  switch (op) {
  case OpCode::Const:
  case OpCode::Add:
  case OpCode::Sub:
  case OpCode::Mul:
  case OpCode::Div:
  case OpCode::Mod:
  case OpCode::Neg:
  case OpCode::Lt:
  case OpCode::Le:
  case OpCode::Gt:
  case OpCode::Ge:
  case OpCode::Eq:
  case OpCode::Ne:
  case OpCode::LNot:
  case OpCode::Call:
  case OpCode::Phi:
    has_result = true;
    break;
  case OpCode::Ret:
  case OpCode::Jmp:
  case OpCode::CondBr:
    has_result = false;
    break;
  }
}

Instruction::Instruction(OpCode op, std::uint64_t id) : Instruction(op) {
  Value::id = id;
}

void Function::replace_all_uses(Value *oldv, Value *newv) {
  for (auto &bb : blcks) {
    for (auto &instr : bb->instrs) {
      for (auto &val : instr->operands) {
        if (val == oldv)
          val = newv;
      }

      std::visit(
          [&](auto &payload) {
            using T = std::decay_t<decltype(payload)>;

            if constexpr (std::is_same_v<T, PhiData>) {
              for (auto &[block, val] : payload.incoming) {
                if (val == oldv)
                  val = newv;
              }
            } else if constexpr (std::is_same_v<T, CondBrData>) {
              if (payload.cond == oldv)
                payload.cond = newv;
            }
          },
          instr->payload);
    }
  }
}

void Function::erase_instr(Instruction *dead) {
  for (auto &bb : blcks) {
    auto &instrs = bb->instrs;

    auto it = std::find_if(instrs.begin(), instrs.end(),
                           [&](auto &ptr) { return ptr.get() == dead; });

    if (it != instrs.end()) {
      instrs.erase(it);
      break;
    }
  }
}

} // namespace fcc
