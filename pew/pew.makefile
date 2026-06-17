CC ?= gcc

CFLAGS  := -Wall -Wextra -std=c99
LDFLAGS := -lm

SRC := pew.c
OBJ := $(SRC:.c=.o)

UNAME_S := $(shell uname -s 2>/dev/null)

# --------------------------------------------------
# Platform Detection
# --------------------------------------------------

ifeq ($(UNAME_S),Linux)
    CFLAGS += -DPEW_HAS_POSIX
    LDFLAGS += -ldl
endif

ifeq ($(UNAME_S),Darwin)
    CFLAGS += -DPEW_HAS_POSIX
endif

ifneq (,$(findstring MINGW,$(UNAME_S)))
    CFLAGS += -DPEW_HAS_WIN32
endif

ifneq (,$(findstring MSYS,$(UNAME_S)))
    CFLAGS += -DPEW_HAS_WIN32
endif

# --------------------------------------------------
# Build Modes
# --------------------------------------------------

DEBUG_FLAGS   := -O0 -g3 -DDEBUG
RELEASE_FLAGS := -O3 -DNDEBUG

# --------------------------------------------------
# Default Target
# --------------------------------------------------

all: release

# --------------------------------------------------
# Release
# --------------------------------------------------

release: CFLAGS += $(RELEASE_FLAGS)
release: pew

# --------------------------------------------------
# Debug
# --------------------------------------------------

debug: CFLAGS += $(DEBUG_FLAGS)
debug: pew

# --------------------------------------------------
# Link
# --------------------------------------------------

pew: $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

# --------------------------------------------------
# Compile
# --------------------------------------------------

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# --------------------------------------------------
# WebAssembly
# --------------------------------------------------

wasm:
	emcc pew.c \
		-O3 \
		-s WASM=1 \
		-s EXIT_RUNTIME=1 \
		-DPEW_PLATFORM_WASM \
		-o pew.html

# --------------------------------------------------
# Static Analysis
# --------------------------------------------------

analyze:
	cppcheck --enable=all --inconclusive .

# --------------------------------------------------
# Memory Check
# --------------------------------------------------

valgrind:
	valgrind --leak-check=full ./pew

# --------------------------------------------------
# Clean
# --------------------------------------------------

clean:
	rm -f *.o pew pew.exe pew.html pew.js pew.wasm

.PHONY: all release debug wasm clean analyze valgrind
