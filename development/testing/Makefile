# Makefile for Fixed-Point Optimization Testing
# Tests correctness and performance of modulus elimination

CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99
INCLUDES = -I../../bfin_lib/src -I../../common
TARGET = fix_test
SOURCES = fix_optimization_test.c

# Platform detection
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    CFLAGS += -DMACOS
endif

# Build test executable
$(TARGET): $(SOURCES)
	@echo "Building fixed-point optimization test..."
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) $(SOURCES)
	@echo "Test executable ready: ./$(TARGET)"

# Run all tests
test: $(TARGET)
	@echo "Running fixed-point optimization validation..."
	./$(TARGET)

# Run only correctness tests (fast)
test-correctness: $(TARGET)
	@echo "Running correctness tests only..."
	./$(TARGET) --correctness-only

# Run performance benchmark
benchmark: $(TARGET)
	@echo "Running performance benchmark..."
	./$(TARGET) --benchmark-only

# Clean build artifacts
clean:
	rm -f $(TARGET) *.o
	@echo "Cleaned build artifacts"

# Create test report
report: $(TARGET)
	@echo "Generating optimization test report..."
	./$(TARGET) > fix_optimization_test_report.txt
	@echo "Report saved to: fix_optimization_test_report.txt"

# Quick validation (minimal test set)
quick-test: $(TARGET)
	@echo "Running quick validation..."
	./$(TARGET) --quick

.PHONY: test test-correctness benchmark clean report quick-test

# Help
help:
	@echo "Fixed-Point Optimization Testing"
	@echo "================================"
	@echo ""
	@echo "Available targets:"
	@echo "  test           - Run all tests (correctness + performance)"
	@echo "  test-correctness - Run only correctness validation"
	@echo "  benchmark      - Run only performance benchmarks"
	@echo "  quick-test     - Fast validation with minimal test set"
	@echo "  report         - Generate detailed test report"
	@echo "  clean          - Remove build artifacts"
	@echo "  help           - Show this help message"
	@echo ""
	@echo "Expected results:"
	@echo "  ✅ All correctness tests should pass"
	@echo "  🚀 Performance should show 1.5-2x improvement"