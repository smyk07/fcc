#include "LoweringPass.hpp"
#include "IR.hpp"
#include "SourceFile.hpp"
#include "Utils.hpp"

#include <clang-c/Index.h>

#include <format>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace fcc {

Type *TypeCtx::get(TypeKind kind) {
  auto it = primitives.find(kind);
  if (it != primitives.end())
    return it->second.get();

  auto t = std::make_unique<Type>(kind);
  auto *ptr = t.get();
  primitives[kind] = std::move(t);

  return ptr;
}

Type *TypeCtx::get_pointer_to(Type *pointee) {
  for (auto &t : compounds) {
    if (t->kind == TypeKind::Pointer && t->pointed_to == pointee)
      return t.get();
  }

  auto t = std::make_unique<Type>(TypeKind::Pointer);
  t->pointed_to = pointee;
  auto *ptr = t.get();
  compounds.push_back(std::move(t));

  return ptr;
}

Type *TypeCtx::get_array_of(Type *elem, uint64_t len) {
  for (auto &t : compounds) {
    if (t->kind == TypeKind::Array && t->arr_elem_type == elem &&
        t->arr_len == len)
      return t.get();
  }

  auto t = std::make_unique<Type>(TypeKind::Array);
  t->arr_elem_type = elem;
  t->arr_len = len;
  auto *ptr = t.get();
  compounds.push_back(std::move(t));

  return ptr;
}

TypeCtx LoweringPass::type_ctx{};

Type *LoweringPass::lower_type(CXType cxtype) {
  switch (cxtype.kind) {
  case CXType_SChar:
  case CXType_Char_S:
    return type_ctx.get(TypeKind::I8);

  case CXType_UChar:
  case CXType_Char_U:
    return type_ctx.get(TypeKind::U8);

  case CXType_Short:
    return type_ctx.get(TypeKind::I16);

  case CXType_UShort:
    return type_ctx.get(TypeKind::U16);

  case CXType_Int:
    return type_ctx.get(TypeKind::I32);

  case CXType_UInt:
    return type_ctx.get(TypeKind::U32);

  case CXType_LongLong:
    return type_ctx.get(TypeKind::I64);

  case CXType_ULongLong:
    return type_ctx.get(TypeKind::U64);

  case CXType_Float:
    return type_ctx.get(TypeKind::F32);

  case CXType_Double:
    return type_ctx.get(TypeKind::F64);

  case CXType_Void:
    return type_ctx.get(TypeKind::Void);

  case CXType_Pointer: {
    Type *pointee = lower_type(clang_getPointeeType(cxtype));

    return type_ctx.get_pointer_to(pointee);
  }

  case CXType_ConstantArray: {
    Type *elem = lower_type(clang_getArrayElementType(cxtype));

    auto len = clang_getArraySize(cxtype);

    return type_ctx.get_array_of(elem, len);
  }

  default:
    throw_error(std::format("unsupported type {}",
                            cxstring_to_string(clang_getTypeSpelling(cxtype))));
  }
}

Value *LoweringPass::lower_expr(CXCursor expr, Function *fn, BasicBlock *bb) {
  switch (clang_getCursorKind(expr)) {

  case CXCursor_IntegerLiteral: {
    auto instr = std::make_unique<Instruction>();

    instr->op = OpCode::Const;
    instr->type = type_ctx.get(TypeKind::I32);
    instr->id = next_value_id++;

    auto literal = get_cursor_text(source.tu, expr);
    auto value = std::stoull(literal, nullptr, 0);
    instr->payload = ConstData{.bits = value};

    auto *result = instr.get();
    bb->instrs.push_back(std::move(instr));

    return result;
  }

  case CXCursor_BinaryOperator: {
    auto clang_op = clang_getCursorBinaryOperatorKind(expr);

    std::vector<CXCursor> children;
    clang_visitChildren(
        expr,
        [](CXCursor child, CXCursor /* parent */, CXClientData data) {
          static_cast<std::vector<CXCursor> *>(data)->push_back(child);
          return CXChildVisit_Continue;
        },
        &children);

    Value *lhs = lower_expr(children[0], fn, bb);
    Value *rhs = lower_expr(children[1], fn, bb);

    OpCode op;
    switch (clang_op) {
    case CXBinaryOperator_Add:
      op = OpCode::Add;
      break;
    case CXBinaryOperator_Sub:
      op = OpCode::Sub;
      break;
    case CXBinaryOperator_Mul:
      op = OpCode::Mul;
      break;
    case CXBinaryOperator_Div:
      op = OpCode::Div;
      break;
    default:
      throw_error(std::format(
          "unsupported binary type {}",
          cxstring_to_string(clang_getBinaryOperatorKindSpelling(clang_op))));
    }

    auto instr = std::make_unique<Instruction>();
    instr->id = next_value_id++;
    instr->op = op;
    instr->type = lhs->type;
    instr->operands.push_back(lhs);
    instr->operands.push_back(rhs);

    Value *result = instr.get();
    bb->instrs.push_back(std::move(instr));
    return result;

    break;
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
    bool found = false;

    struct Ctx {
      CXCursor *out;
      bool *found;
    };
    Ctx ctx{&expr, &found};

    clang_visitChildren(
        stmt,
        [](CXCursor child, CXCursor /* parent */, CXClientData data) {
          auto *ctx = static_cast<Ctx *>(data);
          *ctx->out = child;
          *ctx->found = true;
          return CXChildVisit_Break;
        },
        &ctx);

    auto ret = std::make_unique<Instruction>();
    ret->has_result = false;
    ret->op = OpCode::Ret;

    if (found) {
      Value *ret_val = lower_expr(expr, fn, bb);
      ret->operands.push_back(ret_val);
    }

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
      lower_type(clang_getResultType(clang_getCursorType(fn_decl)));

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
          param->type = lower_type(clang_getCursorType(cursor));
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
      [](CXCursor cursor, CXCursor /* parent */, CXClientData client_data) {
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
