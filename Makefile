CXX = g++
CXXFLAGS = -std=c++17 -Isrc -Wall -Wextra
LDFLAGS = -pthread

BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj

LIB_SRCS = src/json_logic.cpp \
           src/video_logic.cpp \
           src/config.cpp \
           src/analytics.cpp \
           src/exporters.cpp \
           src/ai.cpp \
           src/cloud.cpp

LIB_OBJS = $(LIB_SRCS:src/%.cpp=$(OBJ_DIR)/src/%.o)

MAIN_SRC = src/main.cpp
MAIN_OBJ = $(OBJ_DIR)/src/main.o

TARGET = $(BUILD_DIR)/QuantaCerebra

TEST_SRCS = $(wildcard tests/*.cpp)
TEST_BINS = $(TEST_SRCS:tests/%.cpp=$(BUILD_DIR)/%)
TEST_OBJS = $(TEST_SRCS:tests/%.cpp=$(OBJ_DIR)/tests/%.o)

.PHONY: all clean run test help directories

all: directories $(TARGET)

directories:
	@mkdir -p $(OBJ_DIR)/src
	@mkdir -p $(OBJ_DIR)/tests

$(TARGET): $(MAIN_OBJ) $(LIB_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/tests/%.o: tests/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%: $(OBJ_DIR)/tests/%.o $(LIB_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

test: directories $(TEST_BINS)
	@for test in $(TEST_BINS); do \
		echo "----------------------------------------"; \
		echo "Running $$test..."; \
		./$$test || exit 1; \
	done

run: all
	./$(TARGET)

help:
	@echo "Available targets:"
	@echo "  all     - Build the QuantaCerebra application (default)"
	@echo "  run     - Build and run the QuantaCerebra application"
	@echo "  test    - Build and run all tests"
	@echo "  clean   - Remove all built artifacts"
	@echo "  help    - Show this help message"

clean:
	rm -rf $(BUILD_DIR)
