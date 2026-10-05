############################
# Project configuration
############################

APP_NAME = AssemblyArchitect
EXEC     = assemblyArchitect

S_DIR = source
B_DIR = .

DATA_DIRS = sound fonts data img
APP_ICON = icon.icns

S_FILES := $(shell find $(S_DIR) -type f -name '*.c')
TEST_EXEC = tutorial_text_test
TEST_SOURCE = tests/tutorial_text_test.c
TEST_SOURCES := $(filter-out $(S_DIR)/main.c,$(S_FILES))
OPERAND_TEST_EXEC = operand_highlights_test
OPERAND_TEST_SOURCE = tests/operand_highlights_test.c
LEVEL_CONFIG_HOVER_TEST_EXEC = level_config_hover_test
LEVEL_CONFIG_HOVER_TEST_SOURCE = tests/level_config_hover_test.c
PROGRAM_CHARACTERIZATION_TEST_EXEC = program_characterization_test
PROGRAM_CHARACTERIZATION_TEST_SOURCE = tests/program_characterization_test.c
PROGRAM_DOMAIN_TEST_EXEC = program_domain_test
PROGRAM_DOMAIN_TEST_SOURCE = tests/program_domain_test.c
PROGRAM_ADAPTER_TEST_EXEC = program_adapter_test
PROGRAM_ADAPTER_TEST_SOURCE = tests/program_adapter_test.c
PROGRAM_EDITOR_SYNC_TEST_EXEC = program_editor_sync_test
PROGRAM_EDITOR_SYNC_TEST_SOURCE = tests/program_editor_sync_test.c
LEVEL1_APPEND_TEST_EXEC = level1_append_test
LEVEL1_APPEND_TEST_SOURCE = tests/level1_append_test.c

 
############################
# Compiler
############################

CC = clang
CFLAGS = -g -I$(S_DIR)

############################
# SDL Frameworks
############################

FRAMEWORK_DIR = /Library/Frameworks

SDL_FRAMEWORKS = \
    SDL2.framework \
    SDL2_image.framework \
    SDL2_ttf.framework \
    SDL2_mixer.framework

SDL_INCLUDES = \
    -I$(FRAMEWORK_DIR)/SDL2.framework/Headers \
    -I$(FRAMEWORK_DIR)/SDL2_image.framework/Headers \
    -I$(FRAMEWORK_DIR)/SDL2_ttf.framework/Headers \
    -I$(FRAMEWORK_DIR)/SDL2_mixer.framework/Headers

SDL_LINK = \
    -F$(FRAMEWORK_DIR) \
    -framework SDL2 \
    -framework SDL2_image \
    -framework SDL2_ttf \
    -framework SDL2_mixer

############################
# App bundle paths
############################

APP_BUNDLE = $(APP_NAME).app
APP_CONTENTS = $(APP_BUNDLE)/Contents
APP_MACOS = $(APP_CONTENTS)/MacOS
APP_FRAMEWORKS = $(APP_CONTENTS)/Frameworks
APP_RESOURCES = $(APP_CONTENTS)/Resources

############################
# Default target
############################

all: build

build:
	$(CC) $(CFLAGS) $(S_FILES) -o $(EXEC) \
    $(SDL_INCLUDES) $(SDL_LINK) \
	-Wl,-rpath,/Library/Frameworks

run: build
	./$(EXEC)

test: test-level-config-hover test-operand-highlights test-program-characterization test-program-domain test-level1-append
	$(CC) $(CFLAGS) $(TEST_SOURCE) $(TEST_SOURCES) -o $(TEST_EXEC) \
	    $(SDL_INCLUDES) $(SDL_LINK) -Wl,-rpath,/Library/Frameworks
	./$(TEST_EXEC)

test-operand-highlights:
	$(CC) $(CFLAGS) $(OPERAND_TEST_SOURCE) $(TEST_SOURCES) -o $(OPERAND_TEST_EXEC) \
	    $(SDL_INCLUDES) $(SDL_LINK) -Wl,-rpath,/Library/Frameworks
	./$(OPERAND_TEST_EXEC)

test-level-config-hover:
	$(CC) $(CFLAGS) $(LEVEL_CONFIG_HOVER_TEST_SOURCE) $(TEST_SOURCES) -o $(LEVEL_CONFIG_HOVER_TEST_EXEC) \
	    $(SDL_INCLUDES) $(SDL_LINK) -Wl,-rpath,/Library/Frameworks
	./$(LEVEL_CONFIG_HOVER_TEST_EXEC)

test-program-characterization:
	$(CC) $(CFLAGS) $(PROGRAM_CHARACTERIZATION_TEST_SOURCE) $(TEST_SOURCES) -o $(PROGRAM_CHARACTERIZATION_TEST_EXEC) \
	    $(SDL_INCLUDES) $(SDL_LINK) -Wl,-rpath,/Library/Frameworks
	./$(PROGRAM_CHARACTERIZATION_TEST_EXEC)

test-program-domain:
	$(CC) $(CFLAGS) $(PROGRAM_DOMAIN_TEST_SOURCE) source/domain/program.c -o $(PROGRAM_DOMAIN_TEST_EXEC)
	./$(PROGRAM_DOMAIN_TEST_EXEC)

test-program-adapter:
	$(CC) $(CFLAGS) -Wall -Wextra -Werror $(PROGRAM_ADAPTER_TEST_SOURCE) \
	    source/migration/legacy_program_adapter.c source/domain/program.c \
	    -o $(PROGRAM_ADAPTER_TEST_EXEC)
	./$(PROGRAM_ADAPTER_TEST_EXEC)

test-program-editor-sync:
	$(CC) -std=c11 -Wall -Wextra -Werror -Isource -fsyntax-only \
	    source/domain/program.c source/migration/legacy_program_adapter.c
	$(CC) $(CFLAGS) -DCW_REPAIR_TESTING $(PROGRAM_EDITOR_SYNC_TEST_SOURCE) \
	    $(TEST_SOURCES) -o $(PROGRAM_EDITOR_SYNC_TEST_EXEC) \
	    $(SDL_INCLUDES) $(SDL_LINK) -Wl,-rpath,/Library/Frameworks
	./$(PROGRAM_EDITOR_SYNC_TEST_EXEC)

test-level1-append:
	$(CC) $(CFLAGS) -DSTAGES_EDIT_TESTING $(LEVEL1_APPEND_TEST_SOURCE) $(TEST_SOURCES) \
	    -o $(LEVEL1_APPEND_TEST_EXEC) $(SDL_INCLUDES) $(SDL_LINK) \
	    -Wl,-rpath,/Library/Frameworks
	@save_before=$$(shasum -a 256 "$(CURDIR)"/data/saves/player_*.sav) && \
	    test_root=$$(mktemp -d /tmp/assemblygame-level1.XXXXXX) && \
	    cp -R data img fonts "$$test_root/" && \
	    cp $(abspath $(LEVEL1_APPEND_TEST_EXEC)) "$$test_root/level1_append_test" && \
	    cd "$$test_root" && { \
	        ./level1_append_test; exit_code=$$?; \
	        save_after=$$(shasum -a 256 "$(CURDIR)"/data/saves/player_*.sav); \
	        test "$$save_before" = "$$save_after" || { \
	            printf '%s\n' 'Player saves outside the fixture changed'; exit 1; \
	        }; \
	        exit $$exit_code; \
	    }

############################
# App bundle target
############################

app: build
	@echo "Creating app bundle..."
	mkdir -p $(APP_MACOS) $(APP_FRAMEWORKS) $(APP_RESOURCES)
	
	@echo "Copying app icon..."
	cp $(APP_ICON) $(APP_RESOURCES)/

	@echo "Copying resource directories..."
	for dir in $(DATA_DIRS); do \
	if [ -d $$dir ]; then \
 	cp -R $$dir $(APP_RESOURCES)/ ; \
 	fi \
	done

	@echo "Copying executable..."
	cp $(EXEC) $(APP_MACOS)/$(APP_NAME)
	
	@echo "Copying SDL frameworks..."
	for fw in $(SDL_FRAMEWORKS); do \
	    cp -R $(FRAMEWORK_DIR)/$$fw $(APP_FRAMEWORKS)/ ; \
	done
	
	@echo "Fixing framework paths..."
	install_name_tool -add_rpath "@executable_path/../Frameworks" \
	    $(APP_MACOS)/$(APP_NAME)
	
	install_name_tool -change \
	$(FRAMEWORK_DIR)/SDL2.framework/SDL2 \
	@executable_path/../Frameworks/SDL2.framework/SDL2 \
	$(APP_MACOS)/$(APP_NAME)
	
	install_name_tool -change \
	$(FRAMEWORK_DIR)/SDL2_image.framework/SDL2_image \
	@executable_path/../Frameworks/SDL2_image.framework/SDL2_image \
	$(APP_MACOS)/$(APP_NAME)
	
	install_name_tool -change \
	$(FRAMEWORK_DIR)/SDL2_ttf.framework/SDL2_ttf \
	@executable_path/../Frameworks/SDL2_ttf.framework/SDL2_ttf \
	$(APP_MACOS)/$(APP_NAME)
	
	install_name_tool -change \
	$(FRAMEWORK_DIR)/SDL2_mixer.framework/SDL2_mixer \
	@executable_path/../Frameworks/SDL2_mixer.framework/SDL2_mixer \
	$(APP_MACOS)/$(APP_NAME)
	
	@echo "Writing Info.plist..."
	printf '%s\n' \
	'<?xml version="1.0" encoding="UTF-8"?>' \
	'<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN"' \
	'"http://www.apple.com/DTDs/PropertyList-1.0.dtd">' \
	'<plist version="1.0">' \
	'<dict>' \
	'  <key>CFBundleExecutable</key>' \
	'  <string>$(APP_NAME)</string>' \
	'  <key>CFBundleIdentifier</key>' \
	'  <string>edu.yourname.$(APP_NAME)</string>' \
	'  <key>CFBundleName</key>' \
	'  <string>$(APP_NAME)</string>' \
	'  <key>CFBundleIconFile</key>' \
	'  <string>icon</string>' \
	'  <key>CFBundlePackageType</key>' \
	'  <string>APPL</string>' \
	'</dict>' \
	'</plist>' \
	> $(APP_CONTENTS)/Info.plist
	
	@echo "App bundle created: $(APP_BUNDLE)"

	@echo "Signing app bundle..."
	codesign --deep --force --sign - $(APP_BUNDLE)

	@echo "Removing quarantine attribute..."
	xattr -dr com.apple.quarantine $(APP_BUNDLE)

############################
# Clean
############################

clean:
	rm -rf $(EXEC) $(APP_BUNDLE) $(TEST_EXEC) $(LEVEL_CONFIG_HOVER_TEST_EXEC) $(PROGRAM_CHARACTERIZATION_TEST_EXEC) $(PROGRAM_DOMAIN_TEST_EXEC) $(PROGRAM_ADAPTER_TEST_EXEC) $(PROGRAM_EDITOR_SYNC_TEST_EXEC) $(LEVEL1_APPEND_TEST_EXEC)
	find . -type d -name '*.dSYM' -prune -exec rm -rf {} +
	rm -rf assemblyArchitect*
	rm -f source/.*.swp
	rm -f data/.*.swp
