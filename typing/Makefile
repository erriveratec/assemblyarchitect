
# Makefile for Assembly Architect Title Screen
# Supports macOS with SDL frameworks in /Library/Frameworks (no Homebrew),
# and falls back to sdl2-config on Linux/MSYS2.
#
# Targets:
#   make            # build
#   make run        # run
#   make clean      # clean

APP := title_screen
SRC := src/title_screen.c
CC  ?= clang
CFLAGS ?=

UNAME_S := $(shell uname -s)

# ---------------------- macOS (Frameworks in /Library) ----------------------
ifeq ($(UNAME_S),Darwin)
    # Framework paths
    FW_DIR    := /Library/Frameworks
    FW_FLAGS  := -F$(FW_DIR) \
                 -framework SDL2 \
                 -framework SDL2_ttf \
                 -framework SDL2_mixer

    # Header paths inside frameworks
    INC_FLAGS := -I$(FW_DIR)/SDL2.framework/Headers \
                 -I$(FW_DIR)/SDL2_ttf.framework/Headers \
                 -I$(FW_DIR)/SDL2_mixer.framework/Headers

    # Ensure threadsafety flags on macOS build
    CFLAGS   += -D_THREAD_SAFE -D_REENTRANT $(INC_FLAGS)

    # Ensure the binary finds frameworks at runtime (no need to copy them next to the app)
    LDFLAGS  += -Wl,-rpath,$(FW_DIR)

    LIBS     := $(FW_FLAGS) -lm

    $(APP): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)
else
# ---------------------- Linux / MSYS2 fallback ----------------------
    SDL2_CFLAGS := $(shell sdl2-config --cflags)
    SDL2_LIBS   := $(shell sdl2-config --libs)
    LIBS := $(SDL2_LIBS) -lSDL2_ttf -lSDL2_mixer -lm

    $(APP): $(SRC)
	$(CC) $(CFLAGS) $(SDL2_CFLAGS) -o $@ $^ $(LIBS)
endif

run: $(APP)
	./$(APP)

clean:
	rm -f $(APP)

.PHONY: all run clean
