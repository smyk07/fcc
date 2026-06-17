CXX      := clang++
CXXFLAGS := -O0 -std=c++23 -Wall -Wextra -g -fsanitize=address,undefined,leak

TARGET  := fcc
HEADERS := SourceFile.hpp
SRC     := fcc.cpp

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRC) $(HEADERS)
	$(CXX) $(CXXFLAGS) -lclang $< -o $@

clean:
	rm -f $(TARGET)
