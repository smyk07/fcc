#include "LoweringPass.hpp"
#include "IR.hpp"
#include "SourceFile.hpp"
#include "Utils.hpp"

#include <clang-c/Index.h>

#include <memory>
#include <string>
#include <utility>

namespace fcc {

Value *LoweringPass::lower_expr(CXCursor expr, Function *fn, BasicBlock *bb) {
  switch (clang_getCursorKind(expr)) {

  case CXCursor_IntegerLiteral: {
    auto instr = std::make_unique<Instruction>();

    instr->op = OpCode::Const;
    instr->type = std::make_unique<Type>(TypeKind::I32);
    instr->id = next_value_id++;

    auto literal = get_cursor_text(source.tu, expr);
    auto value = std::stoull(literal, nullptr, 0);
    instr->payload = ConstData{.bits = value};

    auto *result = instr.get();
    bb->instrs.push_back(std::move(instr));

    return result;
  }

  default:
    throw_error("unsupported expression");
  }
}

void LoweringPass::lower_stmt(CXCursor stmt, Function *fn, BasicBlock *bb) {
  struct StmtCtx {
    LoweringPass *self;
    Function *fn;
    BasicBlock *bb;
  } ctx{this, fn, bb};

  switch (clang_getCursorKind(stmt)) {
  case CXCursor_CompoundStmt:
    clang_visitChildren(
        stmt,
        [](CXCursor child, CXCursor, CXClientData data) {
          auto *ctx = static_cast<StmtCtx *>(data);
          ctx->self->lower_stmt(child, ctx->fn, ctx->bb);
          return CXChildVisit_Continue;
        },
        &ctx);
    break;

  case CXCursor_ReturnStmt: {
    CXCursor expr = clang_getNullCursor();

    clang_visitChildren(
        stmt,
        [](CXCursor child, CXCursor, CXClientData data) {
          *static_cast<CXCursor *>(data) = child;
          return CXChildVisit_Break;
        },
        &expr);

    Value *ret_val = lower_expr(expr, fn, bb);

    auto ret = std::make_unique<Instruction>();
    ret->has_result = false;
    ret->op = OpCode::Ret;
    ret->operands.push_back(ret_val);

    bb->instrs.push_back(std::move(ret));
    break;
  }

  default:
    throw_error("unsupported statement");
  }
}

void LoweringPass::lower_function(const CXCursor fn_decl, Module *mod) {
  auto fn = std::make_unique<Function>();
  auto *fn_ptr = fn.get();

  fn_ptr->name = cxstring_to_string(clang_getCursorSpelling(fn_decl));
  fn_ptr->ret_type =
      std::make_unique<Type>(clang_getResultType(clang_getCursorType(fn_decl)));

  CXCursor body_cursor = clang_getNullCursor();

  struct Ctx {
    LoweringPass *self;
    Function *fn;
    CXCursor *body_out;
  } ctx{this, fn_ptr, &body_cursor};

  clang_visitChildren(
      fn_decl,
      [](CXCursor cursor, CXCursor /*parent*/, CXClientData client_data) {
        auto *ctx = static_cast<Ctx *>(client_data);

        switch (clang_getCursorKind(cursor)) {
        case CXCursor_ParmDecl: {
          auto param = std::make_unique<Value>();
          param->type = std::make_unique<Type>(clang_getCursorType(cursor));
          ctx->fn->params.push_back(std::move(param));
          break;
        }
        case CXCursor_CompoundStmt:
          *ctx->body_out = cursor;
          break;
        default:
          break;
        }

        return CXChildVisit_Continue;
      },
      &ctx);

  auto entry = std::make_unique<BasicBlock>();
  auto *entry_ptr = entry.get();
  entry_ptr->id = next_bb_id++;

  fn_ptr->blcks.push_back(std::move(entry));

  if (!clang_Cursor_isNull(body_cursor)) {
    lower_stmt(body_cursor, fn_ptr, entry_ptr);
  }

  mod->funcs.push_back(std::move(fn));
}

Module LoweringPass::run() {
  Module mod;

  struct VisitorCtx {
    LoweringPass *self;
    Module *mod;
  } ctx{this, &mod};

  clang_visitChildren(
      source.root,
      [](CXCursor cursor, CXCursor, CXClientData client_data) {
        auto *ctx = static_cast<VisitorCtx *>(client_data);

        if (clang_getCursorKind(cursor) == CXCursor_FunctionDecl &&
            clang_isCursorDefinition(cursor)) {
          ctx->self->lower_function(cursor, ctx->mod);
        }

        return CXChildVisit_Continue;
      },
      &ctx);

  return mod;
}

} // namespace fcc
