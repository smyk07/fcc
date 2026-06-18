#pragma once

#include <clang-c/CXString.h>
#include <clang-c/Index.h>

#include <string>

namespace fcc {

std::string cxstring_to_string(CXString str);

std::string get_cursor_text(CXTranslationUnit tu, CXCursor cursor);

} // namespace fcc
