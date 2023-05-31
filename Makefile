CC = clang
CFLAGS = -Wall -Werror
CFLAGS_DEBUG = -fsanitize=address,undefined -g3
LDFLAGS = -lm -lSDL2 -lSDL2_ttf -lSDL2_gfx

CFILES = main.c draw.c widgets.c utils.c
BIN = waves

all:	
	$(CC) $(CFLAGS) $(LDFLAGS) $(CFILES) -o $(BIN)

wasm:
	emcc $(CFILES) \
	-s WASM=1 \
	-s USE_SDL=2 \
	-s USE_SDL_TTF=2 \
	-s USE_SDL_GFX=2 \
	-sALLOW_MEMORY_GROWTH \
	--preload-file res \
	-o index.js


clean:
	rm -rf *.js *.wasm $(BIN)


.PHONY: all
