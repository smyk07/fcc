#pragma once

#include <clang-c/CXString.h>
#include <clang-c/Index.h>

#include <source_location>
#include <string>

namespace fcc {

[[noreturn]]
void throw_error(std::string msg,
                 std::source_location loc = std::source_location::current());

std::string cxstring_to_string(CXString str);

std::string get_cursor_text(CXTranslationUnit tu, CXCursor cursor);

} // namespace fcc
