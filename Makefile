# QuantaCerebra Makefile
# Cross-platform build system for C++11/17

CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2
TARGET = QuantaCerebra
TEST_UNIT = test_unit
TEST_BDD = test_bdd

# Source files
SOURCES = main.cpp json_logic.cpp video_logic.cpp
TEST_SOURCES = json_logic.cpp video_logic.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Default target
all: $(TARGET)

# Main application
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Unit tests
$(TEST_UNIT): test_unit.cpp $(TEST_SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $^

# BDD tests
$(TEST_BDD): test_bdd.cpp $(TEST_SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Object file compilation
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Test targets
test: $(TEST_UNIT) $(TEST_BDD)
	@echo "Running unit tests..."
	./$(TEST_UNIT)
	@echo "Running BDD tests..."
	./$(TEST_BDD)

# Run the application
run: $(TARGET)
	./$(TARGET)

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(TARGET) $(TEST_UNIT) $(TEST_BDD)
	rm -f *.exe *.o build_errors.txt

# Windows-specific targets
windows: CXXFLAGS += -static-libgcc -static-libstdc++
windows: TARGET := $(TARGET).exe
windows: TEST_UNIT := $(TEST_UNIT).exe
windows: TEST_BDD := $(TEST_BDD).exe
windows: $(TARGET)

# Help target
help:
	@echo "Available targets:"
	@echo "  all      - Build main application (default)"
	@echo "  test     - Build and run all tests"
	@echo "  run      - Build and run main application"
	@echo "  windows  - Build for Windows with static linking"
	@echo "  clean    - Remove all build artifacts"
	@echo "  help     - Show this help message"

.PHONY: all test run clean windows help
