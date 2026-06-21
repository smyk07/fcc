CXX      := clang++
CXXFLAGS := -Iinclude -O0 -std=c++23 -Wall -Wextra -g -fsanitize=address,undefined,leak

TARGET  := fcc
HEADERS := $(shell find include -name "*.hpp")
SRC     := $(shell find src -name "*.cpp")

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRC) $(HEADERS)
	$(CXX) -o $@ $(CXXFLAGS) $(SRC) -lclang

clean:
	rm -f $(TARGET)
