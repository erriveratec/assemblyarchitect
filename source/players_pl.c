#include "players_pl.h"
#include <math.h>

// ============================================================================
// Internal helpers (pixel-safe drawing)
// ============================================================================
static inline int iroundf(float v){ return (int)lroundf(v); }
static inline void setc(SDL_Renderer* r, SDL_Color c){
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
}
static void fill_cell(SDL_Renderer* r, int ox, int oy, int cs,
                      int cx, int cy, int cw, int ch, SDL_Color col){
    SDL_Rect rr = { ox + cx*cs, oy + cy*cs, cw*cs, ch*cs };
    setc(r, col); SDL_RenderFillRect(r, &rr);
}

// ============================================================================
// Glyphs (5x5) — complex but blocky, for 7x7 sign inner field with padding
// ============================================================================
static const unsigned char G5_X[5][5]={{1,0,1,0,1},{0,1,0,1,0},{0,0,1,0,0},{0,1,0,1,0},{1,0,1,0,1}};
static const unsigned char G5_Y[5][5]={{1,0,0,0,1},{0,1,0,1,0},{0,0,1,0,0},{0,0,1,0,0},{0,0,1,0,0}};
static const unsigned char G5_Z[5][5]={{1,1,1,1,1},{0,0,0,0,1},{0,0,1,1,1},{0,1,0,0,0},{1,1,1,1,1}};
static const unsigned char (*glyph5_for(char L))[5]{
    switch(L){ case 'x': case 'X': return G5_X; case 'y': case 'Y': return G5_Y; default: return G5_Z; }
}

// ============================================================================
// Bodies (block coordinates) — tuned to read clearly humanoid
// ============================================================================
static void body_wide(SDL_Renderer* r,int ox,int oy,int cs, SDL_Color fg){
    // head + neck
    fill_cell(r,ox,oy,cs, 4,2, 2,2, fg);  // 2x2 head
    fill_cell(r,ox,oy,cs, 5,4, 1,1, fg);  // 1x1 neck
    // shoulders shelf
    fill_cell(r,ox,oy,cs, 3,5, 4,1, fg);
    fill_cell(r,ox,oy,cs, 3,6, 5,1, fg);
    // chest + waist
    fill_cell(r,ox,oy,cs, 4,7, 3,3, fg);  // 3x3 chest
    fill_cell(r,ox,oy,cs, 4,10,3,2, fg);  // 3x2 waist
    // legs + feet
    fill_cell(r,ox,oy,cs, 4,12,1,6, fg);
    fill_cell(r,ox,oy,cs, 6,12,1,6, fg);
    fill_cell(r,ox,oy,cs, 4,18,2,1, fg);  // 2x1 feet (grounded)
    // arms (left down; right stepped toward pole)
    fill_cell(r,ox,oy,cs, 3,9,1,5, fg);
    fill_cell(r,ox,oy,cs, 7,8,1,2, fg);
    fill_cell(r,ox,oy,cs, 8,9,1,1, fg);
    fill_cell(r,ox,oy,cs, 9,10,1,1, fg);
}

static void body_med(SDL_Renderer* r,int ox,int oy,int cs, SDL_Color fg){
    fill_cell(r,ox,oy,cs, 4,2,2,2, fg); fill_cell(r,ox,oy,cs, 5,4,1,1, fg);
    fill_cell(r,ox,oy,cs, 3,5,3,1, fg); fill_cell(r,ox,oy,cs, 3,6,4,1, fg);
    fill_cell(r,ox,oy,cs, 4,7,2,5, fg); fill_cell(r,ox,oy,cs, 4,12,2,1, fg);
    // legs with tiny gap + knee hint
    fill_cell(r,ox,oy,cs, 3,13,1,3, fg); fill_cell(r,ox,oy,cs, 3,16,1,2, fg);
    fill_cell(r,ox,oy,cs, 5,13,1,3, fg); fill_cell(r,ox,oy,cs, 5,16,1,2, fg);
    fill_cell(r,ox,oy,cs, 3,18,1,1, fg); fill_cell(r,ox,oy,cs, 5,18,1,1, fg);
    // arms
    fill_cell(r,ox,oy,cs, 2,9,1,4, fg);
    fill_cell(r,ox,oy,cs, 6,8,1,2, fg); fill_cell(r,ox,oy,cs, 7,9,1,1, fg); fill_cell(r,ox,oy,cs, 8,10,1,1, fg);
}

static void body_slim(SDL_Renderer* r,int ox,int oy,int cs, SDL_Color fg){
    fill_cell(r,ox,oy,cs, 4,2,2,2, fg); fill_cell(r,ox,oy,cs, 5,4,1,1, fg);
    fill_cell(r,ox,oy,cs, 4,5,2,1, fg); fill_cell(r,ox,oy,cs, 5,6,1,6, fg);
    fill_cell(r,ox,oy,cs, 5,12,1,1, fg);
    fill_cell(r,ox,oy,cs, 4,13,1,7, fg); fill_cell(r,ox,oy,cs, 6,13,1,7, fg);
    fill_cell(r,ox,oy,cs, 4,20,1,1, fg); fill_cell(r,ox,oy,cs, 6,20,1,1, fg);
    // arms
    fill_cell(r,ox,oy,cs, 3,9,1,4, fg);
    fill_cell(r,ox,oy,cs, 7,8,1,2, fg); fill_cell(r,ox,oy,cs, 8,9,1,1, fg); fill_cell(r,ox,oy,cs, 9,10,1,1, fg);
}

// ============================================================================
// Sign (9x9 board, 7x7 inner field, 5x5 glyph) + terminal effects
// ============================================================================
static void draw_sign(SDL_Renderer* r,int ox,int oy,int cs,
                      SDL_Color fg, SDL_Color plate, char letter, float t, int opts){
    if(opts & AA_OPT_LIFT) oy -= 2; // pixel-safe lift for entire figure

    // Board anchor relative to body origin
    const int bx = 12, by = 4; // 9x9 outer board starts here

    // Pole (attached to bottom frame)
    SDL_Color poleC = fg; if(opts & AA_OPT_DIM) poleC.a = 140;
    fill_cell(r,ox,oy,cs, bx+4, by+8, 1, 6, poleC);

    // Thick frame (1 cell)
    SDL_Color frameC = fg; if(opts & AA_OPT_DIM) frameC.a = 140;
    fill_cell(r,ox,oy,cs, bx,   by,   9,1, frameC); // top
    fill_cell(r,ox,oy,cs, bx,   by+8, 9,1, frameC); // bottom
    fill_cell(r,ox,oy,cs, bx,   by+1, 1,7, frameC); // left
    fill_cell(r,ox,oy,cs, bx+8, by+1, 1,7, frameC); // right

    // Inner field (7x7)
    SDL_Color fieldC = (opts & AA_OPT_INVERT)? fg : plate;
    fill_cell(r,ox,oy,cs, bx+1, by+1, 7,7, fieldC);

    // 5x5 glyph centered in 7x7 → starts at (bx+2, by+2). Blink + afterglow unless DIM.
    const unsigned char (*G)[5] = glyph5_for(letter);
    int on = 1; if(!(opts & AA_OPT_DIM)){ float ph = fmodf(t, 1.0f); on = (ph < 0.65f); }
    SDL_Color gcol = (opts & AA_OPT_INVERT) ? (SDL_Color){ plate.r, plate.g, plate.b, plate.a }
                                            : fg;
    gcol.a = (opts & AA_OPT_DIM) ? 120 : (on ? 255 : 70);
    for(int yy=0; yy<5; yy++)
        for(int xx=0; xx<5; xx++)
            if(G[yy][xx]) fill_cell(r,ox,oy,cs, bx+2+xx, by+2+yy, 1,1, gcol);

    // Vertical CRT sweep (1px horizontal line across inner field)
    if(!(opts & AA_OPT_DIM)){
        int inner_px_h = 7*cs; int scan_y = (int)fmodf(t*60.0f, (float)inner_px_h);
        int slx = ox + (bx+1)*cs + 1; int sly = oy + (by+1)*cs + scan_y; int slw = 7*cs - 2;
        if(sly >= oy + (by+1)*cs + 1 && sly < oy + (by+1)*cs + 7*cs - 1){
            SDL_SetRenderDrawColor(r, 255,255,255, 64); SDL_Rect sl = { slx, sly, slw, 1 }; SDL_RenderFillRect(r, &sl);
        }
    }

    // Hover halo (around outer board) + tiny shadow when lifted
    if((opts & AA_OPT_LIFT) && !(opts & AA_OPT_DIM)){
        SDL_Rect outer = (SDL_Rect){ ox + bx*cs, oy + by*cs, 9*cs, 9*cs };
        SDL_Rect halo  = { outer.x-2, outer.y-2, outer.w+4, outer.h+4 };
        SDL_SetRenderDrawColor(r, 255,255,255, 50); SDL_RenderDrawRect(r, &halo);
        SDL_Rect sh = { outer.x, outer.y + outer.h, outer.w, 2 }; // pixel shadow 2px
        SDL_SetRenderDrawColor(r, 0,0,0,160); SDL_RenderFillRect(r, &sh);
    }
}

// ============================================================================
// Public API
// ============================================================================
void aa_draw_human(SDL_Renderer* r,
                   AA_FigureType type,
                   int x, int y,
                   int cs,
                   SDL_Color fg,
                   SDL_Color plate,
                   char letter,
                   float t,
                   int opts){
    // Apply integer lift to the whole figure if requested
    int oy = y + ((opts & AA_OPT_LIFT) ? -2 : 0);

    switch(type){
        case AA_FIG_WIDE: body_wide(r, x, oy, cs, fg); break;
        case AA_FIG_MED:  body_med (r, x, oy, cs, fg); break;
        default:          body_slim(r, x, oy, cs, fg); break;
    }
    draw_sign(r, x, oy, cs, fg, plate, letter, t, opts);
}

SDL_Rect aa_sign_rect_px(int x, int y, int cs){
    // Matches draw_sign(): outer board at (bx=12, by=4), size 9x9 cells
    return (SDL_Rect){ x + 12*cs, y + 4*cs, 9*cs, 9*cs };
}
