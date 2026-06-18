#include "IR.hpp"

#include <print>

namespace fcc {

const char *opcode_to_string(OpCode op) {
  switch (op) {
  case OpCode::Const:
    return "const";
  case OpCode::Add:
    return "add";
  case OpCode::Sub:
    return "sub";
  case OpCode::Mul:
    return "mul";
  case OpCode::Div:
    return "div";
  case OpCode::Ret:
    return "ret";
  case OpCode::Jmp:
    return "jmp";
  case OpCode::CondBr:
    return "condbr";
  case OpCode::Phi:
    return "phi";
  }
}

void dump(fcc::Instruction &instr) {
  if (instr.has_result)
    std::print("%{} = ", instr.id);

  std::print("{}", opcode_to_string(instr.op));

  switch (instr.op) {
  case OpCode::Const: {
    auto data = std::get<ConstData>(instr.payload);
    std::print(" {}", data.bits);
    break;
  }

  case OpCode::Ret:
    if (!instr.operands.empty())
      std::print(" %{}", instr.operands[0]->id);
    break;

  default:
    break;
  }

  std::println();
}

void dump(fcc::BasicBlock &bb) {
  std::println("bb {}:", bb.id);

  for (auto &instr : bb.instrs)
    dump(*instr);
}

void dump(fcc::Function &fn) {
  std::println("fn {} {{", fn.name);

  for (auto &bb : fn.blcks)
    dump(*bb);

  std::println("}}");
}

void dump(fcc::Module &module) {
  for (auto &fn : module.funcs)
    dump(*fn);
}

} // namespace fcc
