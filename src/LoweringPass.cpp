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

std::uint64_t LoweringPass::next_bb_id = 0;
std::uint64_t LoweringPass::next_value_id = 0;

Instruction *LoweringPass::create_phi(CXCursor var, BasicBlock *bb) {
  auto phi = std::make_unique<Instruction>(OpCode::Phi, next_value_id++);
  phi->type = lower_type(clang_getCursorType(var));
  phi->payload = PhiData{};

  auto *ptr = phi.get();
  bb->instrs.insert(bb->instrs.begin(), std::move(phi));

  return ptr;
}

BasicBlock *LoweringPass::create_block(Function *fn) {
  auto bb = std::make_unique<BasicBlock>();

  bb->id = next_bb_id++;
  bb->sealed = false;

  BasicBlock *ptr = bb.get();
  fn->blcks.push_back(std::move(bb));

  return ptr;
}

void LoweringPass::seal_block(Function *fn, BasicBlock *bb,
                              const std::vector<BasicBlock *> &preds) {
  bb->sealed = true;
  bb->preds = preds;

  for (auto &[var, phi] : bb->incomplete_phis) {
    PhiData data;
    for (auto *pred : preds) {
      Value *incoming = read_variable(var, fn, pred);
      data.incoming.push_back({pred, incoming});
    }
    phi->payload = std::move(data);
    try_remove_trivial_phi(fn, phi);
  }

  bb->incomplete_phis.clear();
}

void LoweringPass::write_variable(CXCursor var, BasicBlock *bb, Value *val) {
  defs[bb][var] = val;
}

Value *LoweringPass::read_variable(CXCursor var, Function *fn, BasicBlock *bb) {
  auto &bb_defs = defs[bb];
  auto it = bb_defs.find(var);

  if (it != bb_defs.end())
    return it->second;

  return read_variable_recursive(var, fn, bb);
}

Value *LoweringPass::read_variable_recursive(CXCursor var, Function *fn,
                                             BasicBlock *bb) {
  if (!bb->sealed) {
    auto phi = create_phi(var, bb);

    bb->incomplete_phis.emplace_back(var, phi);

    write_variable(var, bb, phi);
    return phi;
  }

  if (bb->preds.size() == 1) {
    Value *val = read_variable(var, fn, bb->preds[0]);
    write_variable(var, bb, val);
    return val;
  }

  auto *phi = create_phi(var, bb);
  write_variable(var, bb, phi);

  return add_phi_operands(var, fn, phi, bb->preds);
}

Value *LoweringPass::add_phi_operands(CXCursor var, Function *fn,
                                      Instruction *phi,
                                      const std::vector<BasicBlock *> &preds) {
  PhiData data;

  for (auto *pred : preds) {
    Value *incoming = read_variable(var, fn, pred);
    data.incoming.push_back({pred, incoming});
  }

  phi->payload = std::move(data);

  return try_remove_trivial_phi(fn, phi);
}

Value *LoweringPass::try_remove_trivial_phi(Function *fn, Instruction *phi) {
  auto &data = std::get<PhiData>(phi->payload);
  Value *same = nullptr;

  for (auto &[pred, val] : data.incoming) {
    if (val == phi)
      continue;

    if (!same) {
      same = val;
      continue;
    }

    if (val != same)
      return phi;
  }

  if (!same)
    return phi;

  for (auto &[bb, map] : defs) {
    for (auto &[var, val] : map) {
      if (val == phi)
        val = same;
    }
  }

  fn->replace_all_uses(phi, same);
  fn->erase_instr(phi);
  return same;
}

Value *LoweringPass::lower_expr(CXCursor expr, Module *mod, Function *fn,
                                BasicBlock *&bb) {
  switch (clang_getCursorKind(expr)) {

  case CXCursor_IntegerLiteral: {
    auto instr = std::make_unique<Instruction>(OpCode::Const, next_value_id++);
    instr->type = type_ctx.get(TypeKind::I32);

    auto literal = get_cursor_text(source.tu, expr);
    auto bits = std::stoull(literal, nullptr, 0);
    instr->payload = ConstData{.bits = bits};

    auto *result = instr.get();
    bb->instrs.push_back(std::move(instr));

    return result;
  }

  case CXCursor_UnaryOperator: {
    auto clang_op = clang_getCursorUnaryOperatorKind(expr);

    CXCursor child = clang_getNullCursor();
    clang_visitChildren(
        expr,
        [](CXCursor child, CXCursor /* parent */, CXClientData data) {
          *static_cast<CXCursor *>(data) = child;
          return CXChildVisit_Continue;
        },
        &child);

    OpCode op;
    switch (clang_op) {
    case CXUnaryOperator_Plus:
      return lower_expr(child, mod, fn, bb);

    case CXUnaryOperator_Minus:
      op = OpCode::Neg;
      break;

    case CXUnaryOperator_LNot:
      op = OpCode::LNot;
      break;

    default:
      throw_error(std::format(
          "unsupported unary type {}",
          cxstring_to_string(clang_getUnaryOperatorKindSpelling(clang_op))));
    }

    Value *operand = lower_expr(child, mod, fn, bb);

    auto instr = std::make_unique<Instruction>(op, next_value_id++);
    instr->type = operand->type;
    instr->operands.push_back(operand);

    Value *result = instr.get();
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

    OpCode op;
    switch (clang_op) {
    case CXBinaryOperator_Assign: {
      std::vector<CXCursor> children;

      clang_visitChildren(
          expr,
          [](CXCursor child, CXCursor, CXClientData data) {
            auto *children = static_cast<std::vector<CXCursor> *>(data);
            children->push_back(child);
            return CXChildVisit_Continue;
          },
          &children);

      Value *val = lower_expr(children[1], mod, fn, bb);
      CXCursor var = clang_getCursorReferenced(children[0]);
      write_variable(var, bb, val);
      return val;
    }

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
    case CXBinaryOperator_Rem:
      op = OpCode::Mod;
      break;

    case CXBinaryOperator_LT:
      op = OpCode::Lt;
      break;
    case CXBinaryOperator_LE:
      op = OpCode::Le;
      break;
    case CXBinaryOperator_GT:
      op = OpCode::Gt;
      break;
    case CXBinaryOperator_GE:
      op = OpCode::Ge;
      break;
    case CXBinaryOperator_EQ:
      op = OpCode::Eq;
      break;
    case CXBinaryOperator_NE:
      op = OpCode::Ne;
      break;

    case CXBinaryOperator_LAnd:
    case CXBinaryOperator_LOr: {
      Value *lhs = lower_expr(children[0], mod, fn, bb);

      auto rhs_bb = create_block(fn);
      auto true_bb = create_block(fn);
      auto false_bb = create_block(fn);
      auto merge_bb = create_block(fn);

      auto lhs_br = std::make_unique<Instruction>(OpCode::CondBr);
      lhs_br->payload = CondBrData{
          .cond = lhs,
          .true_target = (clang_op == CXBinaryOperator_LAnd) ? rhs_bb : true_bb,
          .false_target =
              (clang_op == CXBinaryOperator_LAnd) ? false_bb : rhs_bb,
      };
      bb->instrs.push_back(std::move(lhs_br));

      seal_block(fn, rhs_bb, {bb});

      Value *rhs = lower_expr(children[1], mod, fn, rhs_bb);

      auto rhs_br = std::make_unique<Instruction>(OpCode::CondBr);
      rhs_br->payload = CondBrData{
          .cond = rhs,
          .true_target = true_bb,
          .false_target = false_bb,
      };
      rhs_bb->instrs.push_back(std::move(rhs_br));

      seal_block(fn, true_bb, {rhs_bb});
      seal_block(fn, false_bb, {bb, rhs_bb});

      auto true_jmp = std::make_unique<Instruction>(OpCode::Jmp);
      true_jmp->payload = JmpData{.target = merge_bb};
      true_bb->instrs.push_back(std::move(true_jmp));

      auto false_jmp = std::make_unique<Instruction>(OpCode::Jmp);
      false_jmp->payload = JmpData{.target = merge_bb};
      false_bb->instrs.push_back(std::move(false_jmp));

      seal_block(fn, merge_bb, {true_bb, false_bb});

      auto one = std::make_unique<Instruction>(OpCode::Const, next_value_id++);
      one->type = type_ctx.get(TypeKind::I32);
      one->payload = ConstData{.bits = 1};
      auto *one_ptr = one.get();
      merge_bb->instrs.push_back(std::move(one));

      auto zero = std::make_unique<Instruction>(OpCode::Const, next_value_id++);
      zero->type = type_ctx.get(TypeKind::I32);
      zero->payload = ConstData{.bits = 0};
      auto *zero_ptr = zero.get();
      merge_bb->instrs.push_back(std::move(zero));

      auto phi = std::make_unique<Instruction>(OpCode::Phi, next_value_id++);
      phi->type = type_ctx.get(TypeKind::I32);

      PhiData data;
      data.incoming.push_back({true_bb, one_ptr});
      data.incoming.push_back({false_bb, zero_ptr});
      phi->payload = std::move(data);

      Value *result = phi.get();
      merge_bb->instrs.push_back(std::move(phi));
      bb = merge_bb;
      return result;
    }

    default:
      throw_error(std::format(
          "unsupported binary type {}",
          cxstring_to_string(clang_getBinaryOperatorKindSpelling(clang_op))));
    }

    Value *lhs = lower_expr(children[0], mod, fn, bb);
    Value *rhs = lower_expr(children[1], mod, fn, bb);

    auto instr = std::make_unique<Instruction>(op, next_value_id++);
    instr->type = lhs->type;
    instr->operands.push_back(lhs);
    instr->operands.push_back(rhs);

    Value *result = instr.get();
    bb->instrs.push_back(std::move(instr));
    return result;
  }

  case CXCursor_CompoundAssignOperator: {
    std::vector<CXCursor> children;
    clang_visitChildren(
        expr,
        [](CXCursor child, CXCursor, CXClientData data) {
          static_cast<std::vector<CXCursor> *>(data)->push_back(child);
          return CXChildVisit_Continue;
        },
        &children);

    CXCursor var = clang_getCursorReferenced(children[0]);

    auto lhs = read_variable(var, fn, bb);
    auto rhs = lower_expr(children[1], mod, fn, bb);
    OpCode op;

    auto expr_text = get_cursor_text(source.tu, expr);
    if (expr_text.contains("+="))
      op = OpCode::Add;
    else if (expr_text.contains("-="))
      op = OpCode::Sub;
    else if (expr_text.contains("*="))
      op = OpCode::Mul;
    else if (expr_text.contains("/="))
      op = OpCode::Div;
    else if (expr_text.contains("%="))
      op = OpCode::Mod;
    else
      throw_error("unsupported assignment operator");

    auto instr = std::make_unique<Instruction>(op, next_value_id++);
    instr->type = lhs->type;
    instr->operands.push_back(lhs);
    instr->operands.push_back(rhs);

    Value *result = instr.get();
    bb->instrs.push_back(std::move(instr));
    write_variable(var, bb, result);
    return result;
  }

  case CXCursor_UnexposedExpr:
  case CXCursor_ParenExpr: {
    CXCursor child = clang_getNullCursor();

    clang_visitChildren(
        expr,
        [](CXCursor cursor, CXCursor, CXClientData data) {
          *static_cast<CXCursor *>(data) = cursor;
          return CXChildVisit_Break;
        },
        &child);

    return lower_expr(child, mod, fn, bb);
  }

  case CXCursor_DeclRefExpr: {
    CXCursor decl = clang_getCursorReferenced(expr);
    return read_variable(decl, fn, bb);
  }

  case CXCursor_CallExpr: {
    std::vector<CXCursor> children;

    clang_visitChildren(
        expr,
        [](CXCursor child, CXCursor, CXClientData data) {
          auto *children = static_cast<std::vector<CXCursor> *>(data);
          children->push_back(child);
          return CXChildVisit_Continue;
        },
        &children);

    CXCursor callee_expr = children[0];
    CXCursor callee_decl = clang_getCursorReferenced(callee_expr);
    auto callee_name = cxstring_to_string(clang_getCursorSpelling(callee_decl));
    Function *callee = mod->fn_map.at(callee_name);

    auto call_instr =
        std::make_unique<Instruction>(OpCode::Call, next_value_id++);
    call_instr->type = callee->ret_type;
    call_instr->payload = CallData{.callee = callee};

    for (auto arg = children.begin() + 1; arg != children.end(); arg++) {
      auto *arg_val = lower_expr(*arg, mod, fn, bb);
      call_instr->operands.push_back(arg_val);
    }

    auto *result = call_instr.get();
    bb->instrs.push_back(std::move(call_instr));

    return result;
  }

  default:
    throw_error("unsupported expression");
  }
}

BasicBlock *LoweringPass::lower_stmt(CXCursor stmt, Module *mod, Function *fn,
                                     BasicBlock *bb) {
  struct StmtCtx {
    LoweringPass *self;
    Module *mod;
    Function *fn;
    BasicBlock *bb;
  } ctx{this, mod, fn, bb};

  switch (clang_getCursorKind(stmt)) {
  case CXCursor_CompoundStmt:
    clang_visitChildren(
        stmt,
        [](CXCursor child, CXCursor, CXClientData data) {
          auto *ctx = static_cast<StmtCtx *>(data);
          ctx->bb = ctx->self->lower_stmt(child, ctx->mod, ctx->fn, ctx->bb);
          return CXChildVisit_Continue;
        },
        &ctx);
    return ctx.bb;

  case CXCursor_DeclStmt: {
    clang_visitChildren(
        stmt,
        [](CXCursor child, CXCursor, CXClientData data) {
          auto *ctx = static_cast<StmtCtx *>(data);
          ctx->bb = ctx->self->lower_stmt(child, ctx->mod, ctx->fn, ctx->bb);
          return CXChildVisit_Continue;
        },
        &ctx);
    return ctx.bb;
  }

  case CXCursor_VarDecl: {
    CXCursor init = clang_getNullCursor();

    clang_visitChildren(
        stmt,
        [](CXCursor child, CXCursor, CXClientData data) {
          *static_cast<CXCursor *>(data) = child;
          return CXChildVisit_Break;
        },
        &init);

    Value *init_val = lower_expr(init, mod, fn, bb);

    write_variable(stmt, bb, init_val);
    return bb;
  }

  case CXCursor_CompoundAssignOperator:
  case CXCursor_BinaryOperator:
    lower_expr(stmt, mod, fn, bb);
    return bb;

  case CXCursor_IfStmt: {
    std::vector<CXCursor> children;
    clang_visitChildren(
        stmt,
        [](CXCursor child, CXCursor, CXClientData data) {
          static_cast<std::vector<CXCursor> *>(data)->push_back(child);
          return CXChildVisit_Continue;
        },
        &children);

    CXCursor cond_cursor = children[0];
    CXCursor then_cursor = children[1];
    bool has_else = children.size() > 2;
    CXCursor else_cursor = has_else ? children[2] : clang_getNullCursor();

    Value *cond_val = lower_expr(cond_cursor, mod, fn, bb);

    auto then_bb = create_block(fn);
    auto else_bb = has_else ? create_block(fn) : nullptr;
    auto merge_bb = create_block(fn);

    auto br = std::make_unique<Instruction>(OpCode::CondBr);
    br->payload = CondBrData{.cond = cond_val,
                             .true_target = then_bb,
                             .false_target = has_else ? else_bb : merge_bb};

    bb->instrs.push_back(std::move(br));

    seal_block(fn, then_bb, {bb});
    if (has_else) {
      seal_block(fn, else_bb, {bb});
    }

    BasicBlock *then_exit = lower_stmt(then_cursor, mod, fn, then_bb);

    if (then_exit->instrs.empty() ||
        !is_terminator(then_exit->instrs.back()->op)) {
      auto jmp = std::make_unique<Instruction>(OpCode::Jmp);
      jmp->payload = JmpData{.target = merge_bb};

      then_exit->instrs.push_back(std::move(jmp));
    }

    BasicBlock *else_exit = nullptr;

    if (has_else) {
      else_exit = lower_stmt(else_cursor, mod, fn, else_bb);
      if (else_exit->instrs.empty() ||
          !is_terminator(else_exit->instrs.back()->op)) {
        auto jmpelse = std::make_unique<Instruction>(OpCode::Jmp);
        jmpelse->payload = JmpData{.target = merge_bb};
        else_exit->instrs.push_back(std::move(jmpelse));
      }
    }

    std::vector<BasicBlock *> merge_preds = {then_exit};
    merge_preds.push_back(has_else ? else_exit : bb);

    seal_block(fn, merge_bb, merge_preds);
    return merge_bb;
  }

  case CXCursor_WhileStmt: {
    std::vector<CXCursor> children;
    clang_visitChildren(
        stmt,
        [](CXCursor child, CXCursor, CXClientData data) {
          static_cast<std::vector<CXCursor> *>(data)->push_back(child);
          return CXChildVisit_Continue;
        },
        &children);

    CXCursor cond_cursor = children[0];
    CXCursor body_cursor = children[1];

    BasicBlock *header_bb = create_block(fn);

    auto entry_jmp = std::make_unique<Instruction>(OpCode::Jmp);
    entry_jmp->payload = JmpData{.target = header_bb};
    bb->instrs.push_back(std::move(entry_jmp));

    auto body_bb = create_block(fn);
    auto exit_bb = create_block(fn);

    loop_stack.push_back({header_bb, exit_bb});

    Value *cond_val = lower_expr(cond_cursor, mod, fn, header_bb);

    auto br = std::make_unique<Instruction>(OpCode::CondBr);
    br->payload = CondBrData{
        .cond = cond_val, .true_target = body_bb, .false_target = exit_bb};
    header_bb->instrs.push_back(std::move(br));

    seal_block(fn, body_bb, {header_bb});

    auto body_exit = lower_stmt(body_cursor, mod, fn, body_bb);

    if (body_exit->instrs.empty() ||
        !is_terminator(body_exit->instrs.back()->op)) {
      auto loop_jmp = std::make_unique<Instruction>(OpCode::Jmp);
      loop_jmp->payload = JmpData{.target = header_bb};
      body_exit->instrs.push_back(std::move(loop_jmp));
    }

    seal_block(fn, header_bb, {bb, body_exit});
    seal_block(fn, exit_bb, {header_bb});

    loop_stack.pop_back();

    return exit_bb;
  }

  case CXCursor_ForStmt: {
    std::vector<CXCursor> children;
    clang_visitChildren(
        stmt,
        [](CXCursor child, CXCursor, CXClientData data) {
          static_cast<std::vector<CXCursor> *>(data)->push_back(child);
          return CXChildVisit_Continue;
        },
        &children);

    auto init_cursor = children[0];
    auto cond_cursor = children[1];
    auto incr_cursor = children[2];
    auto body_cursor = children[3];

    lower_stmt(init_cursor, mod, fn, bb);

    auto header_bb = create_block(fn);
    auto body_bb = create_block(fn);
    auto incr_bb = create_block(fn);
    auto exit_bb = create_block(fn);

    auto entry_jmp = std::make_unique<Instruction>(OpCode::Jmp);
    entry_jmp->payload = JmpData{.target = header_bb};
    bb->instrs.push_back(std::move(entry_jmp));

    loop_stack.push_back({incr_bb, exit_bb});

    seal_block(fn, body_bb, {header_bb});
    BasicBlock *body_exit = lower_stmt(body_cursor, mod, fn, body_bb);

    if (body_exit && (body_exit->instrs.empty() ||
                      !is_terminator(body_exit->instrs.back()->op))) {
      auto jmp = std::make_unique<Instruction>(OpCode::Jmp);
      jmp->payload = JmpData{.target = incr_bb};
      body_exit->instrs.push_back(std::move(jmp));
    }

    seal_block(fn, incr_bb, {body_exit});
    BasicBlock *incr_exit = lower_stmt(incr_cursor, mod, fn, incr_bb);

    if (incr_exit && (incr_exit->instrs.empty() ||
                      !is_terminator(incr_exit->instrs.back()->op))) {
      auto jmp = std::make_unique<Instruction>(OpCode::Jmp);
      jmp->payload = JmpData{.target = header_bb};
      incr_exit->instrs.push_back(std::move(jmp));
    }

    seal_block(fn, header_bb, {bb, incr_exit});

    Value *cond_val = lower_expr(cond_cursor, mod, fn, header_bb);

    auto br = std::make_unique<Instruction>(OpCode::CondBr);
    br->payload = CondBrData{
        .cond = cond_val, .true_target = body_bb, .false_target = exit_bb};
    header_bb->instrs.push_back(std::move(br));

    seal_block(fn, exit_bb, {header_bb});

    loop_stack.pop_back();

    return exit_bb;
  }

  case CXCursor_BreakStmt: {
    auto exit_bb = loop_stack.back().second;

    auto break_jmp = std::make_unique<Instruction>(OpCode::Jmp);
    break_jmp->payload = JmpData{.target = exit_bb};
    bb->instrs.push_back(std::move(break_jmp));

    return bb;
  }

  case CXCursor_ContinueStmt: {
    auto continue_bb = loop_stack.back().first;

    auto cont_jmp = std::make_unique<Instruction>(OpCode::Jmp);
    cont_jmp->payload = JmpData{.target = continue_bb};
    bb->instrs.push_back(std::move(cont_jmp));

    return bb;
  }

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

    auto ret = std::make_unique<Instruction>(OpCode::Ret);

    if (found) {
      Value *ret_val = lower_expr(expr, mod, fn, bb);
      ret->operands.push_back(ret_val);
    }

    bb->instrs.push_back(std::move(ret));

    return bb;
  }

  case CXCursor_CallExpr:
    lower_expr(stmt, mod, fn, bb);
    return bb;

  default:
    throw_error("unsupported statement");
  }
}

Function *LoweringPass::declare_function(CXCursor fn_decl, Module *mod) {
  auto fn = std::make_unique<Function>();
  auto *fn_ptr = fn.get();

  fn_ptr->name = cxstring_to_string(clang_getCursorSpelling(fn_decl));
  fn_ptr->ret_type =
      lower_type(clang_getResultType(clang_getCursorType(fn_decl)));

  mod->fn_map[fn_ptr->name] = fn_ptr;
  mod->funcs.push_back(std::move(fn));

  return fn_ptr;
}

void LoweringPass::lower_function_body(CXCursor fn_decl, Module *mod,
                                       Function *fn) {
  CXCursor body_cursor = clang_getNullCursor();
  std::vector<std::pair<CXCursor, Value *>> param_decls;

  struct Ctx {
    LoweringPass *self;
    Function *fn;
    CXCursor *body_out;
    std::vector<std::pair<CXCursor, Value *>> *param_decls;
  } ctx{this, fn, &body_cursor, &param_decls};

  clang_visitChildren(
      fn_decl,
      [](CXCursor cursor, CXCursor, CXClientData client_data) {
        auto *ctx = static_cast<Ctx *>(client_data);

        switch (clang_getCursorKind(cursor)) {
        case CXCursor_ParmDecl: {
          auto param = std::make_unique<Value>();
          param->id = ctx->self->next_value_id++;
          param->type = ctx->self->lower_type(clang_getCursorType(cursor));

          Value *param_ptr = param.get();
          ctx->fn->params.push_back(std::move(param));
          ctx->param_decls->push_back({cursor, param_ptr});
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
  entry_ptr->sealed = true;

  fn->blcks.push_back(std::move(entry));

  for (auto &[param_cursor, param_val] : param_decls) {
    write_variable(param_cursor, entry_ptr, param_val);
  }

  BasicBlock *exit_bb = entry_ptr;

  if (!clang_Cursor_isNull(body_cursor)) {
    exit_bb = lower_stmt(body_cursor, mod, fn, entry_ptr);
  }

  bool terminated =
      !exit_bb->instrs.empty() && is_terminator(exit_bb->instrs.back()->op);

  if (!terminated && fn->ret_type->kind == TypeKind::Void) {
    auto ret = std::make_unique<Instruction>(OpCode::Ret);
    exit_bb->instrs.push_back(std::move(ret));
  }
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
          ctx->self->declare_function(cursor, ctx->mod);
        }

        return CXChildVisit_Continue;
      },
      &ctx);

  clang_visitChildren(
      source.root,
      [](CXCursor cursor, CXCursor, CXClientData client_data) {
        auto *ctx = static_cast<VisitorCtx *>(client_data);

        if (clang_getCursorKind(cursor) == CXCursor_FunctionDecl &&
            clang_isCursorDefinition(cursor)) {
          auto name = cxstring_to_string(clang_getCursorSpelling(cursor));
          Function *fn = ctx->mod->fn_map.at(name);
          ctx->self->lower_function_body(cursor, ctx->mod, fn);
        }

        return CXChildVisit_Continue;
      },
      &ctx);

  return mod;
}

} // namespace fcc
