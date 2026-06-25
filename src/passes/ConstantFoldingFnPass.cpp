#include "passes/ConstantFoldingFnPass.hpp"

#include "IR.hpp"

#include <cstdint>
#include <memory>

namespace fcc {

/*
 * Constant Folding
 *
 * evaluating constant expressions such as:
 *
 * %0: i32 = const 6
 * %1: i32 = const 7
 * %2: i32 = add %1 %0
 *
 * becomes
 *
 * %0: i32 = const 13
 */

static inline bool is_foldable(OpCode op) {
  switch (op) {
  case OpCode::Add:
  case OpCode::Sub:
  case OpCode::Mul:
  case OpCode::Div:
  case OpCode::Lt:
  case OpCode::Le:
  case OpCode::Gt:
  case OpCode::Ge:
  case OpCode::Eq:
  case OpCode::Ne:
    return true;

  default:
    return false;
  }
}

static inline Instruction *as_const_instr(Value *v) {
  if (v->kind != ValueKind::Instr)
    return nullptr;

  auto *instr = static_cast<Instruction *>(v);

  if (instr->op != OpCode::Const)
    return nullptr;

  return instr;
}

static inline bool try_fold(Instruction *instr) {
  auto *lhs = as_const_instr(instr->operands[0]);
  auto *rhs = as_const_instr(instr->operands[1]);

  if (!lhs || !rhs)
    return false;

  auto lhs_bits = std::get<ConstData>(lhs->payload).bits;
  auto rhs_bits = std::get<ConstData>(rhs->payload).bits;

  uint64_t result;

  switch (instr->op) {
  case OpCode::Add:
    result = lhs_bits + rhs_bits;
    break;
  case OpCode::Sub:
    result = lhs_bits - rhs_bits;
    break;
  case OpCode::Mul:
    result = lhs_bits * rhs_bits;
    break;
  case OpCode::Div:
    if (rhs_bits == 0)
      return false;
    result = lhs_bits / rhs_bits;
    break;

  case OpCode::Lt:
    result = lhs_bits < rhs_bits;
    break;
  case OpCode::Le:
    result = lhs_bits <= rhs_bits;
    break;
  case OpCode::Gt:
    result = lhs_bits > rhs_bits;
    break;
  case OpCode::Ge:
    result = lhs_bits >= rhs_bits;
    break;
  case OpCode::Eq:
    result = lhs_bits == rhs_bits;
    break;
  case OpCode::Ne:
    result = lhs_bits != rhs_bits;
    break;

  default:
    return false;
  }

  instr->op = OpCode::Const;
  instr->operands.clear();
  instr->payload = ConstData{.bits = result};

  return true;
}

bool ConstantFoldingFnPass::run(Function &fn) {
  bool changed = false;
  bool effective_iteration;

  do {
    effective_iteration = false;

    for (auto &bb : fn.blcks) {
      for (auto &instr_ptr : bb->instrs) {
        auto &instr = *instr_ptr;

        if (!is_foldable(instr.op))
          continue;

        if (try_fold(&instr)) {
          effective_iteration = true;
          changed = true;
        }
      }
    }
  } while (effective_iteration);

  return changed;
}

} // namespace fcc
