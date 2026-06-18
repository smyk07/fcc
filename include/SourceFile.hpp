#pragma once

#include <clang-c/Index.h>

namespace fcc {

struct SourceFile {
public:
  CXCursor root{};
  CXTranslationUnit tu{};

private:
  CXIndex index{};

  static constexpr const char *clang_args[] = {"-xc", "-std=c99"};

public:
  explicit SourceFile(const char *filepath);

  SourceFile(const SourceFile &) = delete;
  SourceFile &operator=(const SourceFile &) = delete;
  SourceFile(SourceFile &&) = delete;
  SourceFile &operator=(SourceFile &&) = delete;

  ~SourceFile();
};

void dump(SourceFile &source);

} // namespace fcc
