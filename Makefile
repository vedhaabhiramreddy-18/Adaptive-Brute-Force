# ─────────────────────────────────────────────────────────────────────────────
#  Makefile – Adaptive Brute-Force Attack Mitigation Simulator
#  Usage:
#    make          – build optimised binary  (./abfams)
#    make debug    – build debug binary
#    make run      – build & run with defaults
#    make clean    – remove build artefacts
# ─────────────────────────────────────────────────────────────────────────────

CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -Wshadow -Iinclude
RELEASE  := -O2 -DNDEBUG
DEBUG    := -g3 -O0 -DDEBUG

TARGET   := abfams
SRCS     := src/main.cpp src/Account.cpp src/AttackLog.cpp \
            src/DefenseManager.cpp src/Simulator.cpp

BUILD_DIR := build

# ─────────────────────────────────────────────────────────────────────────────
.PHONY: all debug run clean

all: $(TARGET)

$(TARGET): $(SRCS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(RELEASE) -o $@ $^
	@echo "✔ Built $(TARGET) (release)"

debug: $(SRCS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(DEBUG) -o $(TARGET)_debug $^
	@echo "✔ Built $(TARGET)_debug"

run: all
	@echo ""
	./$(TARGET)

clean:
	rm -f $(TARGET) $(TARGET)_debug
	rm -rf $(BUILD_DIR)
	@echo "✔ Cleaned"
