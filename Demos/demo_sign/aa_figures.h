#ifndef AA_FIGURES_H
#define AA_FIGURES_H

#include <SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

// Three static pixel-human silhouettes (blocky). Only the SIGN has a thick frame.
typedef enum {
    AA_FIG_WIDE = 0,   // broad chest/shoulders
    AA_FIG_MED,        // medium torso
    AA_FIG_SLIM        // thin torso, longer legs
} AA_FigType;

// Draw one pixel-human holding a sign with a thick frame and terminal-like effects.
// (x,y)  : topleft origin of the BODY grid (not the sign frame)
// cellsz : block size in pixels (e.g., 12–16)
// fg     : figure color (usually white)
// plate  : near-black board color for the sign (e.g., 16,16,16,255)
// letter : 'X', 'Y' or 'Z'
// t      : elapsed time in seconds (controls blink + CRT sweep)
// type   : AA_FIG_WIDE / AA_FIG_MED / AA_FIG_SLIM
void aa_draw_figure(SDL_Renderer* r, int x, int y, int cellsz,
                    SDL_Color fg, SDL_Color plate, char letter, float t, AA_FigType type);

#ifdef __cplusplus
}
#endif
#endif
