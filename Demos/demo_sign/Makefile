# Assembly Architect – Pixel Humans (Large Sign, Complex Letters) – SDL2
# macOS (Frameworks in /Library/Frameworks; no Homebrew required)
# Usage:
#   make            # build
#   make run        # build + run
#   make clean      # clean artifacts

APP   := figures_demo
SRC   := main.c aa_figures.c
OBJ   := $(SRC:.c=.o)

CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Wno-unused-parameter
LDFLAGS ?=

UNAME_S := $(shell uname -s 2>/dev/null)

ifeq ($(UNAME_S),Darwin)
    # macOS using SDL2.framework
    CFLAGS  += -F/Library/Frameworks -I/Library/Frameworks/SDL2.framework/Headers
    # Embed RPATH so dyld can locate the framework at runtime
    LDFLAGS += -F/Library/Frameworks -framework SDL2 \
               -Wl,-rpath,/Library/Frameworks \
               -Wl,-rpath,@executable_path/../Frameworks
else
    # Linux/BSD: attempt sdl2-config, then pkg-config
    SDL2_CFLAGS := $(shell sdl2-config --cflags 2>/dev/null)
    SDL2_LIBS   := $(shell sdl2-config --libs   2>/dev/null)
    ifneq (,$(SDL2_CFLAGS))
        CFLAGS  += $(SDL2_CFLAGS)
        LDFLAGS += $(SDL2_LIBS)
    else
        CFLAGS  += $(shell pkg-config --cflags sdl2 2>/dev/null)
        LDFLAGS += $(shell pkg-config --libs   sdl2 2>/dev/null)
    endif
endif

all: $(APP)

$(APP): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(APP)
	./$(APP)

clean:
	rm -f $(APP) $(OBJ)
