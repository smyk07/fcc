#include "IR.hpp"

#include <format>
#include <inttypes.h>
#include <print>

namespace fcc {

namespace {

constexpr const char *opcode_to_string(const OpCode op) {
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
  case OpCode::Mod:
    return "mod";

  case OpCode::Neg:
    return "neg";

  case OpCode::Lt:
    return "lt";
  case OpCode::Le:
    return "le";
  case OpCode::Gt:
    return "gt";
  case OpCode::Ge:
    return "ge";
  case OpCode::Eq:
    return "eq";
  case OpCode::Ne:
    return "ne";

  case OpCode::LNot:
    return "lnot";

  case OpCode::Call:
    return "call";
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

std::string type_kind_to_str(const Type &ty) {
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
    return (std::string{"ptr "} + type_kind_to_str(*ty.pointed_to));
  case TypeKind::Array:
    return std::format("%s[%" PRIu64 "]", type_kind_to_str(*ty.arr_elem_type),
                       ty.arr_len);
    break;
  }
}

void dump(fcc::Instruction &instr) {
  if (instr.has_result)
    std::print("%{}: {} = ", instr.id, type_kind_to_str(instr.type->kind));

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
  case OpCode::Mod:
  case OpCode::Lt:
  case OpCode::Le:
  case OpCode::Gt:
  case OpCode::Ge:
  case OpCode::Eq:
  case OpCode::Ne:
    std::print(" %{} %{}", instr.operands[0]->id, instr.operands[1]->id);
    break;

  case OpCode::Neg:
  case OpCode::LNot:
    std::print(" %{}", instr.operands[0]->id);
    break;

  case OpCode::Call: {
    auto data = std::get<CallData>(instr.payload);
    std::print(" {}(", data.callee->name);
    if (!instr.operands.empty()) {
      for (auto &ai : instr.operands) {
        if (ai == instr.operands.back())
          std::print("%{}", ai->id);
        else
          std::print("%{}, ", ai->id);
      }
    }
    std::print(")", data.callee->name);
    break;
  }

  case OpCode::Ret:
    if (!instr.operands.empty())
      std::print(" %{}", instr.operands[0]->id);
    break;

  case OpCode::Jmp: {
    auto &data = std::get<JmpData>(instr.payload);
    std::print(" bb_{}", data.target->id);
    break;
  }

  case OpCode::CondBr: {
    auto &data = std::get<CondBrData>(instr.payload);

    std::print(" %{} bb_{} bb_{}", data.cond->id, data.true_target->id,
               data.false_target->id);
    break;
  }

  case OpCode::Phi: {
    auto &data = std::get<PhiData>(instr.payload);

    std::print(" ");

    bool first = true;
    for (auto &[bb, val] : data.incoming) {
      if (!first)
        std::print(", ");

      std::print("[ bb_{}, %{} ]", bb->id, val->id);
      first = false;
    }

    break;
  }
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
      std::print("%{}: {}", pv->id, type_kind_to_str(*pv->type));
    else
      std::print("%{}: {}, ", pv->id, type_kind_to_str(*pv->type));
  }
  std::println(") -> {} {{", type_kind_to_str(*fn.ret_type));

  for (auto &bb : fn.blcks)
    dump(*bb);

  std::println("}}");
}

} // namespace

void dump(fcc::Module &module) {
  for (auto &fn : module.funcs)
    dump(*fn);
}

} // namespace fcc
