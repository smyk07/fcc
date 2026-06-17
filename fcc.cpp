#include "SourceFile.hpp"

#include <exception>
#include <print>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::println("usage: {} [FILE]", argv[0]);
    return 0;
  }

  try {
    fcc::SourceFile root{argv[1]};
    root.dump_ast();
  } catch (const std::exception &e) {
    std::println(stderr, "{}", e.what());
    return 1;
  }

  return 0;
}
