CXX      := clang++
CXXFLAGS := -Iinclude -O0 -std=c++23 -Wall -Wextra -g -fsanitize=address,undefined,leak

TARGET  := fcc
HEADERS := include/Utils.hpp include/SourceFile.hpp include/IR.hpp include/IRPrinter.hpp include/LoweringPass.hpp
SRC     := src/fcc.cpp src/Utils.cpp src/SourceFile.cpp src/IR.cpp src/IRPrinter.cpp src/LoweringPass.cpp

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRC) $(HEADERS)
	$(CXX) -o $@ $(CXXFLAGS) $(SRC) -lclang

clean:
	rm -f $(TARGET)
