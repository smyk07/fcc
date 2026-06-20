#pragma once

#include <clang-c/Index.h>

#include <cstdint>
#include <memory>
#include <variant>
#include <vector>

namespace fcc {

enum class OpCode {
  Const,

  Add,
  Sub,
  Mul,
  Div,

  Ret,

  Jmp,
  CondBr,

  Phi,
};

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

struct Value {
  std::uint64_t id;
  Type *type = nullptr;
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

struct Instruction : Value {
  bool has_result = true;

  OpCode op;
  std::vector<Value *> operands;

  std::variant<std::monostate, ConstData, PhiData, JmpData, CondBrData> payload;
};

// fwd decl
struct Function;

struct BasicBlock {
  std::uint64_t id;

  std::vector<std::unique_ptr<Instruction>> instrs;

  bool sealed = false;
  std::vector<std::pair<CXCursor, Instruction *>> incomplete_phis;

  std::vector<BasicBlock *> successors() const;
  std::vector<BasicBlock *> predecessors(Function *fn) const;
};

struct Function {
  std::string name;
  Type *ret_type = nullptr;
  std::vector<std::unique_ptr<Value>> params;
  std::vector<std::unique_ptr<BasicBlock>> blcks;

  void replace_all_uses(Value *oldv, Value *newv);
};

struct Module {
  std::vector<std::unique_ptr<Function>> funcs;
};

} // namespace fcc
