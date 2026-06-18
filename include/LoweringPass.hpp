#include "IR.hpp"
#include "SourceFile.hpp"

namespace fcc {

struct LoweringPass {
  const SourceFile &source;

public:
  explicit LoweringPass(const SourceFile &source) : source(source) {};

  Module run();

private:
  std::uint64_t next_bb_id = 0;
  std::uint64_t next_value_id = 0;

  Value *lower_expr(CXCursor expr, Function *fn, BasicBlock *bb);

  void lower_stmt(CXCursor stmt, Function *fn, BasicBlock *bb);

  void lower_function(const CXCursor fn_decl, Module *mod);
};

} // namespace fcc
