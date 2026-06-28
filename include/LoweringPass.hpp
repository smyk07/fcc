#pragma once

#include "IR.hpp"
#include "SourceFile.hpp"

#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

namespace fcc {

struct CursorHash {
  size_t operator()(CXCursor c) const { return clang_hashCursor(c); }
};

struct CursorEq {
  bool operator()(CXCursor a, CXCursor b) const {
    return clang_equalCursors(a, b);
  }
};

struct TypeCtx {
private:
  std::unordered_map<TypeKind, std::unique_ptr<Type>> primitives;
  std::vector<std::unique_ptr<Type>> compounds;

public:
  Type *get(TypeKind kind);

  Type *get_pointer_to(Type *pointee);

  Type *get_array_of(Type *elem, uint64_t len);
};

struct LoweringPass {
private:
  const SourceFile &source;

public:
  explicit LoweringPass(const SourceFile &source) : source{source} {};

  Module run();

private:
  std::uint64_t next_bb_id = 0;
  std::uint64_t next_value_id = 0;

  static TypeCtx type_ctx;

  std::unordered_map<
      BasicBlock *, std::unordered_map<CXCursor, Value *, CursorHash, CursorEq>>
      defs;

  std::vector<std::pair<BasicBlock *, BasicBlock *>>
      loop_stack; // {continue_bb, exit_bb}

  static Type *lower_type(CXType cxtype);

  BasicBlock *create_block(Function *fn);
  void seal_block(Function *fn, BasicBlock *bb,
                  const std::vector<BasicBlock *> &preds);

  void write_variable(CXCursor var, BasicBlock *bb, Value *val);
  Value *read_variable(CXCursor var, Function *fn, BasicBlock *bb);
  Value *read_variable_recursive(CXCursor var, Function *fn, BasicBlock *bb);

  Instruction *create_phi(CXCursor var, BasicBlock *bb);
  Value *add_phi_operands(CXCursor var, Function *fn, Instruction *phi,
                          const std::vector<BasicBlock *> &preds);
  Value *try_remove_trivial_phi(Function *fn, Instruction *phi);

  Value *lower_expr(CXCursor expr, Module *mod, Function *fn, BasicBlock *&bb);
  BasicBlock *lower_stmt(CXCursor stmt, Module *mod, Function *fn,
                         BasicBlock *bb);

  Function *declare_function(CXCursor fn_decl, Module *mod);
  void lower_function_body(CXCursor fn_decl, Module *mod, Function *fn);
};

} // namespace fcc
