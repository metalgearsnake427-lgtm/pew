CC = g++
CFLAGS = -O2 -Wall -Wextra -std=c++11 -DUSE_READLINE
LDFLAGS = -lm -lreadline
VERSION = 0.3
BIN = pew

all: build

build:
	$(CC) $(CFLAGS) "pew.C" -o $(BIN) $(LDFLAGS)

install: build
	install -Dm755 $(BIN) $(HOME)/.local/bin/$(BIN)

tarball: build
	mkdir -p dist
	tar -czf dist/pew-$(VERSION).tar.gz pew.C README.md install_pew.sh Makefile website

clean:
	rm -f $(BIN)

.PHONY: all build install tarball clean
