#include "IR.hpp"
#include "SourceFile.hpp"
#include <memory>
#include <unordered_map>
#include <vector>

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
  const SourceFile &source;

public:
  explicit LoweringPass(const SourceFile &source) : source(source) {};

  Module run();

private:
  std::uint64_t next_bb_id = 0;
  std::uint64_t next_value_id = 0;

  static TypeCtx type_ctx;

  static Type *lower_type(CXType cxtype);

  Value *lower_expr(CXCursor expr, Function *fn, BasicBlock *bb);

  void lower_stmt(CXCursor stmt, Function *fn, BasicBlock *bb);

  void lower_function(const CXCursor fn_decl, Module *mod);
};

} // namespace fcc
