CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11 -Iengine $(shell sdl2-config --cflags)
LDFLAGS = $(shell sdl2-config --libs) -lm

ENGINE_SRC = engine/main.c engine/game.c engine/raycast.c engine/render.c \
             engine/texture.c engine/map.c engine/enemy.c engine/weapon.c engine/input.c

TARGET = summer
ASSETS = assets
PYTHON = python3

.PHONY: all clean assets validate run

all: assets validate $(TARGET)

assets:
	$(PYTHON) tools/generate_textures.py
	$(PYTHON) tools/generate_assets.py

validate: assets
	cd rust && cargo run --quiet -- ../assets/levels

$(TARGET): $(ENGINE_SRC)
	$(CC) $(CFLAGS) -o $@ $(ENGINE_SRC) $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
	rm -rf assets/textures/*.ppm assets/sprites/*.ppm assets/levels/*.map
	cd rust && cargo clean 2>/dev/null || true
