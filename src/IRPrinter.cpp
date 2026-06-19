#include "IR.hpp"

#include <format>
#include <inttypes.h>
#include <print>

namespace fcc {

const std::string opcode_to_string(const OpCode op) {
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

const std::string type_to_str(const Type &ty) {
  switch (ty.kind) {
  case TypeKind::I8:
    return "i8";
  case TypeKind::I16:
    return "i16";
  case TypeKind::I32:
    return "i32";
  case TypeKind::I64:
    return "i64";

  case TypeKind::U8:
    return "u8";
  case TypeKind::U16:
    return "u16";
  case TypeKind::U32:
    return "u32";
  case TypeKind::U64:
    return "u64";

  case TypeKind::F32:
    return "f32";
  case TypeKind::F64:
    return "f64";

  case TypeKind::Void:
    return "void";
  case TypeKind::Pointer:
    return (std::string{"ptr "} + type_to_str(*ty.pointed_to));
  case TypeKind::Array:
    return std::format("%s[%" PRIu64 "]", type_to_str(*ty.arr_elem_type),
                       ty.arr_len);
    break;
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

  case OpCode::Add:
  case OpCode::Sub:
  case OpCode::Mul:
  case OpCode::Div:
    std::print(" %{} %{}", instr.operands[0]->id, instr.operands[1]->id);

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
  std::println("bb_{}:", bb.id);

  for (auto &instr : bb.instrs)
    dump(*instr);
}

void dump(fcc::Function &fn) {
  std::print("fn {}(", fn.name);
  for (auto &pv : fn.params) {
    if (pv == fn.params.back())
      std::print("{}", type_to_str(*pv->type));
    else
      std::print("{}, ", type_to_str(*pv->type));
  }
  std::println(") -> {} {{", type_to_str(*fn.ret_type));

  for (auto &bb : fn.blcks)
    dump(*bb);

  std::println("}}");
}

void dump(fcc::Module &module) {
  for (auto &fn : module.funcs)
    dump(*fn);
}

} // namespace fcc
