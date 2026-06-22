CXX      := clang++
CXXFLAGS := -Iinclude -O0 -std=c++23 -Wall -Wextra -g \
            -fsanitize=address,undefined,leak \
            -MMD -MP

TARGET    := fcc
BUILD_DIR := .build

SRC := $(shell find src -name "*.cpp")
OBJ := $(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(SRC))
DEP := $(OBJ:.o=.d)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ -lclang

$(BUILD_DIR)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

-include $(DEP)
