# Directories
S_DIR=source
B_DIR=.

# Files
S_FILES=$(S_DIR)/*.c

# Output
EXEC=$(B_DIR)/assemblygame

# Build settings
CC=gcc
# SDL options
CC_SDL=-I/Library/Frameworks/SDL2.framework/Headers -I/Library/Frameworks/SDL2_image.framework/Headers -I/Library/Frameworks/SDL2_ttf.framework/Headers  -F/Library/Frameworks -framework SDL2 -framework SDL2_image -framework SDL2_ttf

# Added for the new problem
LDFLAGS = -rpath /Library/Frameworks/

all:Build

Build:
	$(CC) -g $(S_FILES) -o $(EXEC) $(CC_SDL) $(LDFLAGS)

build_run:Build
	$(EXEC)

clean:
	rm -rf assemblygame*
	rm -f source/.*.swp
	rm -f data/.*.swp
