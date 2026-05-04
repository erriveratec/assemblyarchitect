############################
# Project configuration
############################

APP_NAME = AssemblyGame
EXEC     = assemblygame

S_DIR = source
B_DIR = .

DATA_DIRS = sound fonts data img
APP_ICON = icon.icns

S_FILES = $(S_DIR)/*.c

############################
# Compiler
############################

CC = clang
CFLAGS = -g

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

############################
# Clean
############################

clean:
	rm -rf $(EXEC) $(APP_BUNDLE)
	rm -rf assemblygame*
	rm -f source/.*.swp
	rm -f data/.*.swp
