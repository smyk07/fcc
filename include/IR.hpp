#pragma once

#include <clang-c/Index.h>

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <variant>
#include <vector>

namespace fcc {

enum class OpCode {
  Const,

  Add,
  Sub,
  Mul,
  Div,
  Mod,

  Neg,

  Lt,
  Le,
  Gt,
  Ge,
  Eq,
  Ne,

  LNot,

  Call,
  Ret,

  Jmp,
  CondBr,

  Phi,
};

constexpr bool is_terminator(OpCode op) {
  switch (op) {
  case OpCode::Ret:
  case OpCode::Jmp:
  case OpCode::CondBr:
    return true;

  default:
    return false;
  }
}

constexpr bool is_unary(OpCode op) {
  switch (op) {
  case OpCode::Neg:
  case OpCode::LNot:
    return true;

  default:
    return false;
  }
}

enum class TypeKind {
  I8,
  I16,
  I32,
  I64,

  U8,
  U16,
  U32,
  U64,

  F32,
  F64,

  Void,
  Pointer,
  Array,
};

struct Type {
  TypeKind kind;

  // pointer
  Type *pointed_to = nullptr;

  // Array
  Type *arr_elem_type = nullptr;
  std::uint64_t arr_len = 0;

  Type(TypeKind kind) : kind{kind} {}
};

enum class ValueKind {
  Instr,
  Param,
};

struct Value {
  ValueKind kind;
  std::uint64_t id;
  Type *type = nullptr;

  Value() { kind = ValueKind::Param; }
};

struct ConstData {
  std::uint64_t bits = 0;
};

// fwd decl
struct BasicBlock;

struct PhiData {
  std::vector<std::pair<BasicBlock *, Value *>> incoming;
};

struct JmpData {
  BasicBlock *target = nullptr;
};

struct CondBrData {
  Value *cond = nullptr;
  BasicBlock *true_target = nullptr;
  BasicBlock *false_target = nullptr;
};

// fwd decl
struct Function;

struct CallData {
  Function *callee;
};

struct Instruction : Value {
  OpCode op;
  std::vector<Value *> operands;

  bool has_result;

  std::variant<std::monostate, ConstData, CallData, JmpData, CondBrData,
               PhiData>
      payload;

  Instruction(OpCode op);
  Instruction(OpCode op, std::uint64_t id);
};

struct BasicBlock {
  std::uint64_t id;

  std::vector<std::unique_ptr<Instruction>> instrs;

  bool sealed = false;
  std::vector<std::pair<CXCursor, Instruction *>> incomplete_phis;

  std::vector<BasicBlock *> preds; // cache

  std::vector<BasicBlock *> successors() const;
  std::vector<BasicBlock *> predecessors(Function *fn) const;
};

struct Function {
  std::string name;
  Type *ret_type = nullptr;
  std::vector<std::unique_ptr<Value>> params;
  std::vector<std::unique_ptr<BasicBlock>> blcks;

  void replace_all_uses(Value *oldv, Value *newv);
  void erase_instr(Instruction *dead);
};

struct Module {
  std::vector<std::unique_ptr<Function>> funcs;
  std::unordered_map<std::string, Function *> fn_map;
};

} // namespace fcc
