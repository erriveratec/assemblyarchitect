# Assembly Architect – Pixel Humans (Larger Sign + Complex Letters) – SDL2

**What changed in this drop**
- **Bigger sign**: 9×9-cell board with a 1-cell thick frame and a 7×7 inner field.
- **More complex letters**: 5×5 glyphs (X/Y/Z) centered in the 7×7 inner area → at least **1 cell of empty space** around the glyph.
- **Silhouette polish**: clearer head/neck, stepped shoulders, chest vs. waist, elbow/wrist blocks on the right arm, broader feet on WIDE, subtle knee hints on MED.
- Terminal effect remains: **1 Hz blink** (65% ON), **afterglow**, and a **vertical CRT sweep** (1‑px line).

## API
```c
void aa_draw_figure(SDL_Renderer* r, int x, int y, int cellsz,
                    SDL_Color fg, SDL_Color plate, char letter, float t, AA_FigType type);
```
- `(x, y)`: topleft of the body grid (use the same `y` to align figures).
- `cellsz`: block size (e.g., 12–16); all strokes scale with it.
- `fg`: figure color (white); `plate`: near‑black (e.g., 16,16,16,255).
- `letter`: 'X', 'Y', 'Z'; `t`: elapsed time (sec) for blink/sweep.

## Build (macOS with SDL2.framework at /Library/Frameworks)
```
make
./figures_demo
```

## Notes
- Drawn exclusively with `SDL_RenderFillRect`, integer coordinates only → sharp pixels.
- To tweak pose proportions, edit `body_wide/med/slim` in `aa_figures.c`.
- To adjust sweep speed, change `t*60.0f` in `draw_sign()`.
