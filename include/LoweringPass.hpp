#include "IR.hpp"
#include "SourceFile.hpp"

#include <memory>
#include <unordered_map>
#include <vector>

struct CursorHash {
  size_t operator()(CXCursor c) const { return clang_hashCursor(c); }
};

struct CursorEq {
  bool operator()(CXCursor a, CXCursor b) const {
    return clang_equalCursors(a, b);
  }
};

namespace fcc {

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

  std::unordered_map<
      BasicBlock *, std::unordered_map<CXCursor, Value *, CursorHash, CursorEq>>
      defs;

  static TypeCtx type_ctx;

  static Type *lower_type(CXType cxtype);

  Instruction *create_phi(CXCursor var, BasicBlock *bb);

  BasicBlock *create_block(Function *fn);

  void seal_block(BasicBlock *bb, const std::vector<BasicBlock *> &preds);

  void write_variable(CXCursor var, BasicBlock *bb, Value *val);
  Value *read_variable(CXCursor var, BasicBlock *bb);
  Value *read_variable_recursive(CXCursor var, BasicBlock *bb);
  Value *add_phi_operands(CXCursor var, Instruction *phi,
                          const std::vector<BasicBlock *> &preds);
  Value *try_remove_trivial_phi(Instruction *phi);

  Value *lower_expr(CXCursor expr, Function *fn, BasicBlock *bb);
  BasicBlock *lower_stmt(CXCursor stmt, Function *fn, BasicBlock *bb);
  void lower_function(CXCursor fn_decl, Module *mod);
};

} // namespace fcc
