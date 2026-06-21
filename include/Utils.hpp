#pragma once

#include <clang-c/CXString.h>
#include <clang-c/Index.h>

#include <source_location>
#include <string>

namespace fcc {

extern bool debug_enabled;

#define FCC_DEBUG(stmt)                                                        \
  do {                                                                         \
    if (fcc::debug_enabled) {                                                  \
      stmt;                                                                    \
    }                                                                          \
  } while (0)

[[noreturn]]
void throw_error(std::string msg,
                 std::source_location loc = std::source_location::current());

std::string cxstring_to_string(CXString str);

std::string get_cursor_text(CXTranslationUnit tu, CXCursor cursor);

} // namespace fcc
