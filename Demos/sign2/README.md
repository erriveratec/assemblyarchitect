# Unified Pixel Human API (SDL2)

A **single function** to draw any of the three pixel-human silhouettes at `(x, y)` with a given block size `cellsz`. The sign is large (9×9 board, 7×7 inner) with a 5×5 glyph and terminal-like effects (1 Hz blink + vertical CRT sweep). All code uses integer coordinates and `SDL_RenderFillRect` only.

## API
```c
typedef enum { AA_FIG_WIDE, AA_FIG_MED, AA_FIG_SLIM } AA_FigureType;

// Options
#define AA_OPT_NONE   0
#define AA_OPT_LIFT   1
#define AA_OPT_INVERT 2
#define AA_OPT_DIM    4

void aa_draw_human(SDL_Renderer* r,
                   AA_FigureType type,
                   int x, int y,
                   int cellsz,
                   SDL_Color fg,
                   SDL_Color plate,
                   char letter,
                   float t,
                   int opts);

SDL_Rect aa_sign_rect_px(int x, int y, int cellsz);
```

- `(x, y)`: BODY topleft (use the same `y` to align multiple figures).
- `cellsz`: block size (e.g., **12–16**). Larger values → chunkier pixels and thicker sign frame.
- `fg`: figure color (white); `plate`: near‑black for the sign (e.g., 16,16,16,255).
- `letter`: `'X'|'Y'|'Z'`.
- `t`: elapsed seconds to drive the sign animations.
- `opts`: OR-mask of flags (`LIFT` adds a 2‑px lift + halo; `INVERT` flashes on press; `DIM` pauses animations and dims colors).

## Build (macOS with SDL2.framework at /Library/Frameworks)
```
make
./figures_demo
```

## Notes
- The **outer sign** rectangle is always at `(x + 12*cellsz, y + 4*cellsz)` with size `(9*cellsz, 9*cellsz)`.
- To change CRT speed, edit `t*60.0f` in `aa_humans.c` (`draw_sign`).
- To tweak silhouettes further, adjust `body_wide/med/slim` (sizes are expressed in **cells**).
