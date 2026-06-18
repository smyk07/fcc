#include "IR.hpp"
#include "Utils.hpp"

#include <clang-c/Index.h>

#include <cassert>
#include <format>
#include <stdexcept>

namespace fcc {

Type::Type(CXType cxtype) {
  switch (cxtype.kind) {
  case CXType_SChar:
  case CXType_Char_S:
    kind = TypeKind::I8;
    break;

  case CXType_UChar:
  case CXType_Char_U:
    kind = TypeKind::U8;
    break;

  case CXType_Short:
    kind = TypeKind::I16;
    break;

  case CXType_UShort:
    kind = TypeKind::U16;
    break;

  case CXType_Int:
    kind = TypeKind::I32;
    break;

  case CXType_UInt:
    kind = TypeKind::U32;
    break;

  case CXType_LongLong:
    kind = TypeKind::I64;
    break;

  case CXType_ULongLong:
    kind = TypeKind::U64;
    break;

  case CXType_Float:
    kind = TypeKind::F32;
    break;

  case CXType_Double:
    kind = TypeKind::F64;
    break;

  case CXType_Void:
    kind = TypeKind::Void;
    break;

  case CXType_Pointer: {
    auto *pointee = new Type(clang_getPointeeType(cxtype));

    kind = TypeKind::Pointer;
    pointed_to = pointee;
    break;
  }

  case CXType_ConstantArray: {
    auto *elem = new Type(clang_getArrayElementType(cxtype));

    auto len = clang_getArraySize(cxtype);

    kind = TypeKind::Array;
    arr_elem_type = elem;
    arr_len = static_cast<uint64_t>(len);
    break;
  }

  default:
    throw std::runtime_error(
        std::format("unsupported type {}",
                    cxstring_to_string(clang_getTypeSpelling(cxtype))));
    break;
  }
}

std::vector<BasicBlock *> BasicBlock::successors() const {
  assert(!instrs.empty() && "BasicBlock has no instructions");

  auto *term = instrs.back().get();

  switch (term->op) {
  case OpCode::Jmp: {
    auto &data = std::get<JmpData>(term->payload);
    return {data.target};
  }

  case OpCode::CondBr: {
    auto &data = std::get<CondBrData>(term->payload);
    return {data.true_target, data.false_target};
  }

  case OpCode::Ret:
    return {};

  default: {
    assert(false && "BasicBlock does not end in a terminator");
    return {};
  }
  }
}

std::vector<BasicBlock *> BasicBlock::predecessors(Function *fn) const {
  std::vector<BasicBlock *> preds;

  for (auto &b : fn->blcks) {
    auto succs = b.get()->successors();

    for (auto *s : succs) {
      if (s == this) {
        preds.push_back(b.get());
        break;
      }
    }
  }

  return preds;
}

} // namespace fcc
