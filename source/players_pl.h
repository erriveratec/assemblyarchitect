#ifndef PLAYERS_PL_H
#define PLAYERS_PL_H

#include <SDL.h>


// -----------------------------------------------------------------------------
// Unified, easy-to-call API
// -----------------------------------------------------------------------------
// Draws one static pixel human (WIDE / MED / SLIM) holding a large sign.
// Everything is block-based and scales with 'cellsz' (integer pixels only).
// Only the SIGN has a thick frame + terminal effect (1 Hz blink + vertical CRT sweep).
//
// Parameters:
//   r        : SDL renderer
//   type     : which silhouette to draw
//   x, y     : topleft origin of the BODY grid (not the sign frame)
//   cellsz   : block size in pixels (e.g., 12–16)
//   fg       : figure color (usually white)
//   plate    : near-black board color for the sign (e.g., 16,16,16,255)
//   letter   : glyph inside the sign ('X','Y','Z')
//   t        : elapsed time in seconds (drives blink/sweep)
//   opts     : OR-mask of AA_OPT_* flags for simple interactivity visuals
// -----------------------------------------------------------------------------

typedef enum {
    AA_FIG_WIDE = 0,
    AA_FIG_MED,
    AA_FIG_SLIM
} AA_FigureType;

// Options (bitmask)
#define AA_OPT_NONE   0
#define AA_OPT_LIFT   1  // hover: lifts figure by 2 px and draws a 1px halo around the sign
#define AA_OPT_INVERT 2  // press: inverts the inner sign field (flash)
#define AA_OPT_DIM    4  // disabled look: dims colors and pauses animations

void aa_draw_human(SDL_Renderer* r,
                   AA_FigureType type,
                   int x, int y,
                   int cellsz,
                   SDL_Color fg,
                   SDL_Color plate,
                   char letter,
                   float t,
                   int opts);

int pl_get_sign_y_ofs();
int pl_get_fig_block_w();
int pl_get_fig_block_h();

// Outer sign rectangle (9x9 board) in PIXELS — useful for hit-testing/hover.
SDL_Rect aa_sign_rect_px(int x, int y, int cellsz);

#endif // AA_HUMANS_H
