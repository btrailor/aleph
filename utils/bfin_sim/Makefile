# utils/bfin_sim/Makefile
# Builds the bfin_sim JACK/OSC audio module simulator for macOS
# Dependencies: jack, liblo (install via: brew install jack liblo)

TARGET = bfin_sim

SRC_DIR = src
LIB_DIR = $(SRC_DIR)/libfixmath

# Source files
SRC = main.c \
      fract_math.c \
      fract2float_conv.c \
      $(SRC_DIR)/fix.c

OBJ = $(SRC:.c=.o)

# Include paths
COMMON = ../../common
INC = -I. \
      -I$(SRC_DIR) \
      -I$(LIB_DIR) \
      -I$(COMMON)

# Compiler flags
CC = gcc
CFLAGS = -O2 -Wall -g $(INC) -DARCH_BFIN=1

# Dependency flags — use pkg-config with Homebrew fallback
JACK_CFLAGS  := $(shell pkg-config --cflags jack 2>/dev/null || echo "-I/opt/homebrew/include")
JACK_LIBS    := $(shell pkg-config --libs   jack 2>/dev/null || echo "-L/opt/homebrew/lib -ljack")

LIBLO_CFLAGS := $(shell pkg-config --cflags liblo 2>/dev/null || echo "-I/opt/homebrew/include")
LIBLO_LIBS   := $(shell pkg-config --libs   liblo 2>/dev/null || echo "-L/opt/homebrew/lib -llo")

CFLAGS  += $(JACK_CFLAGS) $(LIBLO_CFLAGS)
LDFLAGS  = $(JACK_LIBS) $(LIBLO_LIBS) -lm

# libfixmath static library
FIXMATH_LIB = $(LIB_DIR)/libfixmath.a

.PHONY: all clean libfixmath check-deps

all: check-deps libfixmath $(TARGET)

check-deps:
	@command -v pkg-config >/dev/null 2>&1 || (echo "ERROR: pkg-config not found — install via: brew install pkg-config" && exit 1)
	@(pkg-config --exists jack 2>/dev/null || test -f /opt/homebrew/lib/libjack.dylib) || \
		(echo "ERROR: JACK not found — install via: brew install jack" && exit 1)
	@(pkg-config --exists liblo 2>/dev/null || test -f /opt/homebrew/lib/liblo.dylib) || \
		(echo "ERROR: liblo not found — install via: brew install liblo" && exit 1)

libfixmath:
	$(MAKE) -C $(LIB_DIR)

$(TARGET): $(OBJ) $(FIXMATH_LIB)
	$(CC) $(OBJ) $(FIXMATH_LIB) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
	$(MAKE) -C $(LIB_DIR) clean
