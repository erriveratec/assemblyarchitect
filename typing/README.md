
# Assembly Architect — Title Screen Module

SDL2 title screen with:
- Typewriter animation for `ASSEMBLY ARCHITECT`
- Typing click SFX per character
- Optional "ready" beep when typing finishes
- Center chip image fade-in with subtle pulse
- Blinking `PRESS SPACE`

## Folder Layout
```
assembly-architect-title-screen/
├── Makefile
├── src/
│   └── title_screen.c
└── assets/
    ├── fonts/        # put your TTF here (e.g., PressStart2P.ttf)
    ├── audio/        # type.wav, ready.wav (optional)
    └── ui/           # chip.bmp (or use PNG if you switch to SDL_image)
```

## Required Assets (you add these):
- `assets/fonts/PressStart2P.ttf` (or any pixel/mono font)
- `assets/audio/type.wav` (short key click, 20–40 ms)
- `assets/audio/ready.wav` (optional short beep)
- `assets/ui/chip.bmp` (chip icon; if using BMP with transparency, use a color key)

> You can change the exact paths in `src/title_screen.c`.

## Build
### Linux / macOS
Make sure you have SDL2, SDL2_ttf, and SDL2_mixer installed.

- **macOS (Homebrew):**
  ```bash
  brew install sdl2 sdl2_ttf sdl2_mixer
  ```
- **Ubuntu/Debian:**
  ```bash
  sudo apt-get install libsdl2-dev libsdl2-ttf-dev libsdl2-mixer-dev
  ```

Then:
```bash
make
make run
```

### Windows (MinGW / MSYS2)
Install `mingw-w64` toolchain and SDL2, SDL2_ttf, SDL2_mixer packages in MSYS2. The provided Makefile uses `sdl2-config`, which is available in MSYS2 environments. Otherwise, link libraries manually.

## Notes
- If running in a headless/CI environment without an audio device, set:
  ```bash
  export SDL_AUDIODRIVER=dummy
  ```
  to avoid audio initialization failures.
- Edit `FONT_SIZE_TITLE` / `FONT_SIZE_UI` and positioning inside `title_screen.c` to fit your look.
- Replace the `running=false` on SPACE/ENTER with your scene switch.
