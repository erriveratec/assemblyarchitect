# Makefile — Electron Effect (EXACT original visuals) using macOS Frameworks
APP := demo_effect
SRC := src/main.c src/electron_fx.c
CC  ?= clang
CFLAGS ?=
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
  FW_DIR    := /Library/Frameworks
  FW_FLAGS  := -F$(FW_DIR) \
               -framework SDL2 \
               -framework SDL2_ttf \
               -framework SDL2_mixer \
               -framework SDL2_image
  INC_FLAGS := -I$(FW_DIR)/SDL2.framework/Headers \
               -I$(FW_DIR)/SDL2_ttf.framework/Headers \
               -I$(FW_DIR)/SDL2_mixer.framework/Headers \
               -I$(FW_DIR)/SDL2_image.framework/Headers
  CFLAGS   += -O2 -D_THREAD_SAFE -D_REENTRANT $(INC_FLAGS)
  LDFLAGS  += -Wl,-rpath,$(FW_DIR)
  LIBS     := $(FW_FLAGS) -lm

  $(APP): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)
else
  SDL2_CFLAGS := $(shell pkg-config --cflags sdl2 SDL2_ttf SDL2_mixer SDL2_image)
  SDL2_LIBS   := $(shell pkg-config --libs   sdl2 SDL2_ttf SDL2_mixer SDL2_image)
  LIBS := $(SDL2_LIBS) -lm

  $(APP): $(SRC)
	$(CC) $(CFLAGS) $(SDL2_CFLAGS) -O2 -o $@ $^ $(LIBS)
endif

run: $(APP)
	./$(APP)

clean:
	rm -f $(APP)

.PHONY: all run clean
