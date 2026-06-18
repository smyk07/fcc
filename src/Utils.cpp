#include "Utils.hpp"

#include <clang-c/Index.h>
#include <string>

namespace fcc {

std::string cxstring_to_string(CXString str) {
  const char *cstr = clang_getCString(str);
  std::string result = cstr ? cstr : "";
  clang_disposeString(str);
  return result;
}

std::string get_cursor_text(CXTranslationUnit tu, CXCursor cursor) {
  auto range = clang_getCursorExtent(cursor);
  CXToken *tokens = nullptr;
  unsigned num_tokens = 0;

  clang_tokenize(tu, range, &tokens, &num_tokens);

  if (num_tokens == 0)
    return "";

  std::string text;

  for (unsigned i = 0; i < num_tokens; i++) {
    auto spelling = clang_getTokenSpelling(tu, tokens[i]);

    text += clang_getCString(spelling);

    clang_disposeString(spelling);
  }

  clang_disposeTokens(tu, tokens, num_tokens);

  return text;
}

} // namespace fcc
