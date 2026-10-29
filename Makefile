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
S_HEADERS := $(shell find $(S_DIR) -type f -name '*.h')
# Tests: tests/<feature>_test.c, executable build/tests/<feature>_test, target test-<feature-with-hyphens>.
TEST_BUILD_DIR := build/tests
TUTORIAL_TEXT_TEST_EXEC = $(TEST_BUILD_DIR)/tutorial_text_test
TUTORIAL_TEXT_TEST_SOURCE = tests/tutorial_text_test.c
TEST_SOURCES := $(filter-out $(S_DIR)/main.c,$(S_FILES))
OPERAND_HIGHLIGHTS_TEST_EXEC = $(TEST_BUILD_DIR)/operand_highlights_test
OPERAND_HIGHLIGHTS_TEST_SOURCE = tests/operand_highlights_test.c
LEVEL_CONFIG_HOVER_TEST_EXEC = $(TEST_BUILD_DIR)/level_config_hover_test
LEVEL_CONFIG_HOVER_TEST_SOURCE = tests/level_config_hover_test.c
OPERAND_CHANGE_SELECTION_TEST_EXEC = $(TEST_BUILD_DIR)/operand_change_selection_test
OPERAND_CHANGE_SELECTION_TEST_SOURCE = tests/operand_change_selection_test.c
SECTOR_COMPLETION_NAVIGATION_TEST_EXEC = $(TEST_BUILD_DIR)/sector_completion_navigation_test
SECTOR_COMPLETION_NAVIGATION_TEST_SOURCE = tests/sector_completion_navigation_test.c
BUFFER_LABEL_LAYOUT_TEST_EXEC = $(TEST_BUILD_DIR)/buffer_label_layout_test
BUFFER_LABEL_LAYOUT_TEST_SOURCE = tests/buffer_label_layout_test.c
EXECUTION_ARROW_GEOMETRY_TEST_EXEC = $(TEST_BUILD_DIR)/execution_arrow_geometry_test
EXECUTION_ARROW_GEOMETRY_TEST_SOURCE = tests/execution_arrow_geometry_test.c
IO_RAIL_GEOMETRY_TEST_EXEC = $(TEST_BUILD_DIR)/io_rail_geometry_test
IO_RAIL_GEOMETRY_TEST_SOURCE = tests/io_rail_geometry_test.c
TEXT_BOX_LAYOUT_TEST_EXEC = $(TEST_BUILD_DIR)/text_box_layout_test
TEXT_BOX_LAYOUT_TEST_SOURCE = tests/text_box_layout_test.c
TUTORIAL_ARROW_ANIMATION_TEST_EXEC = $(TEST_BUILD_DIR)/tutorial_arrow_animation_test
TUTORIAL_ARROW_ANIMATION_TEST_SOURCE = tests/tutorial_arrow_animation_test.c
HIGHLIGHT_ANIMATION_SYNC_TEST_EXEC = $(TEST_BUILD_DIR)/highlight_animation_sync_test
HIGHLIGHT_ANIMATION_SYNC_TEST_SOURCE = tests/highlight_animation_sync_test.c
MODAL_POINTER_CAPTURE_TEST_EXEC = $(TEST_BUILD_DIR)/modal_pointer_capture_test
MODAL_POINTER_CAPTURE_TEST_SOURCE = tests/modal_pointer_capture_test.c
RUN_RESULT_RENDER_ORDER_TEST_EXEC = $(TEST_BUILD_DIR)/run_result_render_order_test
RUN_RESULT_RENDER_ORDER_TEST_SOURCE = tests/run_result_render_order_test.c
VALUE_BOX_TRANSPORT_TEST_EXEC = $(TEST_BUILD_DIR)/value_box_transport_test
VALUE_BOX_TRANSPORT_TEST_SOURCE = tests/value_box_transport_test.c
REGISTERS_HIGHLIGHT_GEOMETRY_TEST_EXEC = $(TEST_BUILD_DIR)/registers_highlight_geometry_test
REGISTERS_HIGHLIGHT_GEOMETRY_TEST_SOURCE = tests/registers_highlight_geometry_test.c
TEST_EXECS := \
	$(TUTORIAL_TEXT_TEST_EXEC) \
	$(OPERAND_HIGHLIGHTS_TEST_EXEC) \
	$(LEVEL_CONFIG_HOVER_TEST_EXEC) \
	$(OPERAND_CHANGE_SELECTION_TEST_EXEC) \
	$(SECTOR_COMPLETION_NAVIGATION_TEST_EXEC) \
	$(BUFFER_LABEL_LAYOUT_TEST_EXEC) \
	$(EXECUTION_ARROW_GEOMETRY_TEST_EXEC) \
	$(IO_RAIL_GEOMETRY_TEST_EXEC) \
	$(TEXT_BOX_LAYOUT_TEST_EXEC) \
	$(TUTORIAL_ARROW_ANIMATION_TEST_EXEC) \
	$(HIGHLIGHT_ANIMATION_SYNC_TEST_EXEC) \
	$(MODAL_POINTER_CAPTURE_TEST_EXEC) \
	$(RUN_RESULT_RENDER_ORDER_TEST_EXEC) \
	$(VALUE_BOX_TRANSPORT_TEST_EXEC) \
	$(REGISTERS_HIGHLIGHT_GEOMETRY_TEST_EXEC)
TEST_TARGETS := $(addprefix test-,$(subst _,-,$(patsubst %_test,%,$(notdir $(TEST_EXECS)))))
LEGACY_ROOT_TEST_EXECS := \
	$(notdir $(TEST_EXECS)) \
	highligh_animation_sync_test

 
############################
# Compiler
############################

CC = clang
CFLAGS = -g -I$(S_DIR)
override CFLAGS += -Wall -Wextra -Wpedantic -Werror

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
	-isystem $(FRAMEWORK_DIR)/SDL2.framework/Headers \
	-isystem $(FRAMEWORK_DIR)/SDL2_image.framework/Headers \
	-isystem $(FRAMEWORK_DIR)/SDL2_ttf.framework/Headers \
	-isystem $(FRAMEWORK_DIR)/SDL2_mixer.framework/Headers

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

build: $(EXEC)

$(EXEC): $(S_FILES) $(S_HEADERS) Makefile
	$(CC) $(CFLAGS) $(S_FILES) -o $@ \
    $(SDL_INCLUDES) $(SDL_LINK) \
	-Wl,-rpath,/Library/Frameworks

run: build
	./$(EXEC)

test: $(TEST_TARGETS)

.PHONY: all build run app clean test test-level-config-hover test-operand-highlights test-operand-change-selection \
	test-tutorial-text test-sector-completion-navigation test-buffer-label-layout \
	test-execution-arrow-geometry test-text-box-layout test-tutorial-arrow-animation \
	test-highlight-animation-sync test-modal-pointer-capture test-run-result-render-order \
	test-value-box-transport test-io-rail-geometry test-registers-highlight-geometry

$(TEST_BUILD_DIR):
	mkdir -p $@
	for dir in $(DATA_DIRS); do ln -s ../../$$dir $@/$$dir; done

test-modal-pointer-capture: | $(TEST_BUILD_DIR)
	$(CC) $(CFLAGS) $(MODAL_POINTER_CAPTURE_TEST_SOURCE) $(TEST_SOURCES) -o $(MODAL_POINTER_CAPTURE_TEST_EXEC) \
	    $(SDL_INCLUDES) $(SDL_LINK) -Wl,-rpath,/Library/Frameworks
	./$(MODAL_POINTER_CAPTURE_TEST_EXEC)

test-run-result-render-order: | $(TEST_BUILD_DIR)
	$(CC) $(CFLAGS) $(RUN_RESULT_RENDER_ORDER_TEST_SOURCE) \
	    $(filter-out $(S_DIR)/stages.c $(S_DIR)/levels_lv.c $(S_DIR)/ui/run_result_rr.c,$(TEST_SOURCES)) -o $(RUN_RESULT_RENDER_ORDER_TEST_EXEC) \
	    $(SDL_INCLUDES) $(SDL_LINK) -Wl,-rpath,/Library/Frameworks
	./$(RUN_RESULT_RENDER_ORDER_TEST_EXEC)

test-value-box-transport: | $(TEST_BUILD_DIR)
	$(CC) $(CFLAGS) $(VALUE_BOX_TRANSPORT_TEST_SOURCE) \
	    $(S_DIR)/ui/value_box_vb.c $(S_DIR)/dimensions_dm.c -o $(VALUE_BOX_TRANSPORT_TEST_EXEC) \
	    $(SDL_INCLUDES) $(SDL_LINK) -Wl,-rpath,/Library/Frameworks
	./$(VALUE_BOX_TRANSPORT_TEST_EXEC)

test-registers-highlight-geometry: | $(TEST_BUILD_DIR)
	$(CC) $(CFLAGS) $(REGISTERS_HIGHLIGHT_GEOMETRY_TEST_SOURCE) \
	    $(filter-out $(S_DIR)/registers_rg.c,$(TEST_SOURCES)) -o $(REGISTERS_HIGHLIGHT_GEOMETRY_TEST_EXEC) \
	    $(SDL_INCLUDES) $(SDL_LINK) -Wl,-rpath,/Library/Frameworks
	./$(REGISTERS_HIGHLIGHT_GEOMETRY_TEST_EXEC)

test-highlight-animation-sync: | $(TEST_BUILD_DIR)
	$(CC) $(CFLAGS) -DTR_TUTORIAL_TESTING $(HIGHLIGHT_ANIMATION_SYNC_TEST_SOURCE) $(TEST_SOURCES) -o $(HIGHLIGHT_ANIMATION_SYNC_TEST_EXEC) \
	    $(SDL_INCLUDES) $(SDL_LINK) -Wl,-rpath,/Library/Frameworks
	./$(HIGHLIGHT_ANIMATION_SYNC_TEST_EXEC)

test-tutorial-arrow-animation: | $(TEST_BUILD_DIR)
	$(CC) $(CFLAGS) -DTR_TUTORIAL_TESTING $(TUTORIAL_ARROW_ANIMATION_TEST_SOURCE) \
	    $(filter-out $(S_DIR)/arrow_ar.c,$(TEST_SOURCES)) -o $(TUTORIAL_ARROW_ANIMATION_TEST_EXEC) \
	    $(SDL_INCLUDES) $(SDL_LINK) -Wl,-rpath,/Library/Frameworks
	./$(TUTORIAL_ARROW_ANIMATION_TEST_EXEC)

test-text-box-layout: | $(TEST_BUILD_DIR)
	$(CC) $(CFLAGS) $(TEXT_BOX_LAYOUT_TEST_SOURCE) \
	    $(filter-out $(S_DIR)/text_tx.c $(S_DIR)/ui/run_result_rr.c,$(TEST_SOURCES)) -o $(TEXT_BOX_LAYOUT_TEST_EXEC) \
	    $(SDL_INCLUDES) $(SDL_LINK) -Wl,-rpath,/Library/Frameworks
	./$(TEXT_BOX_LAYOUT_TEST_EXEC)

test-io-rail-geometry: | $(TEST_BUILD_DIR)
	$(CC) $(CFLAGS) $(IO_RAIL_GEOMETRY_TEST_SOURCE) \
	    $(filter-out $(S_DIR)/game_mechanics_mc.c,$(TEST_SOURCES)) -o $(IO_RAIL_GEOMETRY_TEST_EXEC) \
	    $(SDL_INCLUDES) $(SDL_LINK) -Wl,-rpath,/Library/Frameworks
	./$(IO_RAIL_GEOMETRY_TEST_EXEC)

test-execution-arrow-geometry: | $(TEST_BUILD_DIR)
	$(CC) $(CFLAGS) $(EXECUTION_ARROW_GEOMETRY_TEST_SOURCE) \
	    $(filter-out $(S_DIR)/arrow_ar.c,$(TEST_SOURCES)) -o $(EXECUTION_ARROW_GEOMETRY_TEST_EXEC) \
	    $(SDL_INCLUDES) $(SDL_LINK) -Wl,-rpath,/Library/Frameworks
	./$(EXECUTION_ARROW_GEOMETRY_TEST_EXEC)

test-buffer-label-layout: | $(TEST_BUILD_DIR)
	$(CC) $(CFLAGS) $(BUFFER_LABEL_LAYOUT_TEST_SOURCE) \
	    $(filter-out $(S_DIR)/main.c $(S_DIR)/buffers_bf.c,$(S_FILES)) -o $(BUFFER_LABEL_LAYOUT_TEST_EXEC) \
	    $(SDL_INCLUDES) $(SDL_LINK) -Wl,-rpath,/Library/Frameworks
	./$(BUFFER_LABEL_LAYOUT_TEST_EXEC)

test-sector-completion-navigation: | $(TEST_BUILD_DIR)
	$(CC) $(CFLAGS) $(SECTOR_COMPLETION_NAVIGATION_TEST_SOURCE) \
	    $(filter-out $(S_DIR)/stages.c,$(TEST_SOURCES)) -o $(SECTOR_COMPLETION_NAVIGATION_TEST_EXEC) \
	    $(SDL_INCLUDES) $(SDL_LINK) -Wl,-rpath,/Library/Frameworks
	./$(SECTOR_COMPLETION_NAVIGATION_TEST_EXEC)

test-tutorial-text: | $(TEST_BUILD_DIR)
	# Test-only row source enables content assertions without production metadata.
	$(CC) $(CFLAGS) \
	    -DTX_STYLED_TEXT_TESTING \
	    -DCW_CHALLENGE_TEXT_TESTING \
	    $(TUTORIAL_TEXT_TEST_SOURCE) $(TEST_SOURCES) -o $(TUTORIAL_TEXT_TEST_EXEC) \
	    $(SDL_INCLUDES) $(SDL_LINK) -Wl,-rpath,/Library/Frameworks
	./$(TUTORIAL_TEXT_TEST_EXEC)

test-operand-change-selection: | $(TEST_BUILD_DIR)
	$(CC) $(CFLAGS) $(OPERAND_CHANGE_SELECTION_TEST_SOURCE) \
	    $(filter-out $(S_DIR)/stages.c,$(TEST_SOURCES)) -o $(OPERAND_CHANGE_SELECTION_TEST_EXEC) \
	    $(SDL_INCLUDES) $(SDL_LINK) -Wl,-rpath,/Library/Frameworks
	./$(OPERAND_CHANGE_SELECTION_TEST_EXEC)

test-operand-highlights: | $(TEST_BUILD_DIR)
	$(CC) $(CFLAGS) -DTR_TUTORIAL_TESTING $(OPERAND_HIGHLIGHTS_TEST_SOURCE) $(TEST_SOURCES) -o $(OPERAND_HIGHLIGHTS_TEST_EXEC) \
	    $(SDL_INCLUDES) $(SDL_LINK) -Wl,-rpath,/Library/Frameworks
	./$(OPERAND_HIGHLIGHTS_TEST_EXEC)

test-level-config-hover: | $(TEST_BUILD_DIR)
	$(CC) $(CFLAGS) $(LEVEL_CONFIG_HOVER_TEST_SOURCE) $(TEST_SOURCES) -o $(LEVEL_CONFIG_HOVER_TEST_EXEC) \
	    $(SDL_INCLUDES) $(SDL_LINK) -Wl,-rpath,/Library/Frameworks
	./$(LEVEL_CONFIG_HOVER_TEST_EXEC)

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
	rm -rf $(EXEC) $(APP_BUNDLE) $(TEST_BUILD_DIR)
	rm -f $(LEGACY_ROOT_TEST_EXECS)
	find . -type d -name '*.dSYM' -prune -exec rm -rf {} +
	rm -rf assemblyArchitect*
	rm -f source/.*.swp
	rm -f data/.*.swp
