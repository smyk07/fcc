#include "SourceFile.hpp"
#include "Utils.hpp"

#include <clang-c/Index.h>

#include <format>
#include <print>

namespace fcc {

SourceFile::SourceFile(const char *filepath) {
  index = clang_createIndex(0, 0);

  tu = clang_parseTranslationUnit(index, filepath, clang_args, 2, nullptr, 0,
                                  CXTranslationUnit_None);

  if (!tu) {
    clang_disposeIndex(index);
    throw_error(std::format("Failed to parse {}", filepath));
  }

  bool has_errors = false;
  unsigned num_diags = clang_getNumDiagnostics(tu);

  for (unsigned i = 0; i < num_diags; i++) {
    auto diag = clang_getDiagnostic(tu, i);
    auto severity = clang_getDiagnosticSeverity(diag);

    if (severity == CXDiagnostic_Error || severity == CXDiagnostic_Fatal) {
      has_errors = true;
    }

    auto msg =
        clang_formatDiagnostic(diag, clang_defaultDiagnosticDisplayOptions());

    std::println("{}", clang_getCString(msg));

    clang_disposeString(msg);
    clang_disposeDiagnostic(diag);
  }

  if (has_errors) {
    clang_disposeTranslationUnit(tu);
    clang_disposeIndex(index);

    throw_error(std::format("Failed to parse {}", filepath));
  }

  root = clang_getTranslationUnitCursor(tu);
}

SourceFile::~SourceFile() {
  clang_disposeTranslationUnit(tu);
  clang_disposeIndex(index);
}

static void dump_cursor(CXCursor cursor, int depth) {
  for (auto i = 0; i < depth; ++i)
    std::print("  ");

  auto kind = cxstring_to_string(
      clang_getCursorKindSpelling(clang_getCursorKind(cursor)));

  auto spelling = cxstring_to_string(clang_getCursorSpelling(cursor));

  if (spelling.empty())
    std::println("{}", kind);
  else
    std::println("{} {}", kind, spelling);

  int next_depth = depth + 1;
  clang_visitChildren(
      cursor,
      [](CXCursor child, CXCursor, CXClientData client_data) {
        auto *current_depth = static_cast<int *>(client_data);
        dump_cursor(child, *current_depth);
        return CXChildVisit_Continue;
      },
      &next_depth);
}

void dump(SourceFile &source) { dump_cursor(source.root, 0); }

} // namespace fcc
