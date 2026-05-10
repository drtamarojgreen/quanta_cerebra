CXX = g++
CXXFLAGS = -std=c++17 -Isrc -Wall -Wextra
LDFLAGS = -pthread

BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj

LIB_SRCS = $(wildcard src/*/*.cpp)
LIB_OBJS = $(LIB_SRCS:src/%.cpp=$(OBJ_DIR)/src/%.o)

MAIN_SRC = src/main.cpp
MAIN_OBJ = $(OBJ_DIR)/src/main.o

TARGET = $(BUILD_DIR)/QuantaCerebra

# Unit Tests
UNIT_SRCS = $(wildcard tests/unit/*/*.cpp)
UNIT_BINS = $(UNIT_SRCS:tests/unit/%.cpp=$(BUILD_DIR)/unit_%)
UNIT_OBJS = $(UNIT_SRCS:tests/unit/%.cpp=$(OBJ_DIR)/tests/unit/%.o)

# SDD Tests
SDD_SRCS = $(wildcard tests/sdd/cards/*.cpp)
SDD_BINS = $(SDD_SRCS:tests/sdd/cards/%.cpp=$(BUILD_DIR)/sdd_%)
SDD_OBJS = $(SDD_SRCS:tests/sdd/cards/%.cpp=$(OBJ_DIR)/tests/sdd/%.o)

.PHONY: all clean run test sdd-test help directories

all: directories $(TARGET)

directories:
	@mkdir -p $(dir $(MAIN_OBJ))
	@mkdir -p $(dir $(LIB_OBJS))
	@mkdir -p $(dir $(UNIT_OBJS))
	@mkdir -p $(dir $(SDD_OBJS))

$(TARGET): $(MAIN_OBJ) $(LIB_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/src/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/tests/unit/%.o: tests/unit/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/tests/sdd/%.o: tests/sdd/cards/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/unit_%: $(OBJ_DIR)/tests/unit/%.o $(LIB_OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/sdd_%: $(OBJ_DIR)/tests/sdd/%.o
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

test: directories $(UNIT_BINS)
	@for test in $(UNIT_BINS); do \
		echo "----------------------------------------"; \
		echo "Running $$test..."; \
		./$$test || exit 1; \
	done

sdd-test: directories $(SDD_BINS)
	@for test in $(SDD_BINS); do \
		echo "----------------------------------------"; \
		echo "Running $$test..."; \
		./$$test || exit 1; \
	done

run: all
	./$(TARGET)

clean:
	@echo "Cleaning up build artifacts and temporary files..."
	rm -rf $(BUILD_DIR)
	rm -rf cloud/
	rm -rf config_presets/
	rm -f *.mid *.png *.bmp *.gif *.bin debug.log tj.json
	@echo "Clean complete."

help:
	@echo "================================================="
	@echo " QuantaCerebra Build System"
	@echo "================================================="
	@echo "  all      : Build the application"
	@echo "  run      : Build and run the CLI"
	@echo "  test     : Run functional unit tests"
	@echo "  sdd-test : Run empirical SDD audits"
	@echo "  clean    : Purge all binaries and temp data"
	@echo "================================================="
