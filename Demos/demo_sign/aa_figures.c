#include "aa_figures.h"
#include <math.h>

// ---------------- helpers ----------------
static inline int iroundf(float v){ return (int)lroundf(v); }

static void fill_cell(SDL_Renderer* r, int ox, int oy, int cs,
                      int cx, int cy, int cw, int ch, SDL_Color col){
    SDL_Rect rr = { ox + cx*cs, oy + cy*cs, cw*cs, ch*cs };
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, col.r, col.g, col.b, col.a);
    SDL_RenderFillRect(r, &rr);
}

// 5x5 block glyphs (more complex shapes) to fit inside a 7x7 inner field with 1-cell padding
static const unsigned char G5_X[5][5]={{1,0,1,0,1},{0,1,0,1,0},{0,0,1,0,0},{0,1,0,1,0},{1,0,1,0,1}};
static const unsigned char G5_Y[5][5]={{1,0,0,0,1},{0,1,0,1,0},{0,0,1,0,0},{0,0,1,0,0},{0,0,1,0,0}};
static const unsigned char G5_Z[5][5]={{1,1,1,1,1},{0,0,0,0,1},{0,0,1,1,1},{0,1,0,0,0},{1,1,1,1,1}}; // more "staircase" style

static const unsigned char (*glyph5_for(char L))[5]{
    switch(L){ case 'x': case 'X': return G5_X; case 'y': case 'Y': return G5_Y; default: return G5_Z; }
}

// ---------------- bodies ----------------
// More humanoid: head+neck, stepped shoulders, chest/waist separation, defined elbow/wrist on right arm.

static void body_wide(SDL_Renderer* r,int ox,int oy,int cs, SDL_Color fg){
    // head + neck
    fill_cell(r,ox,oy,cs, 4,2, 2,2, fg);
    fill_cell(r,ox,oy,cs, 5,4, 1,1, fg);
    // shoulder shelf (upper & lower) and trapezoid torso
    fill_cell(r,ox,oy,cs, 3,5, 4,1, fg);
    fill_cell(r,ox,oy,cs, 3,6, 5,1, fg);
    fill_cell(r,ox,oy,cs, 4,7, 3,3, fg);   // chest
    fill_cell(r,ox,oy,cs, 4,10,3,2, fg);   // waist
    // legs & feet
    fill_cell(r,ox,oy,cs, 4,12,1,6, fg);
    fill_cell(r,ox,oy,cs, 6,12,1,6, fg);
    fill_cell(r,ox,oy,cs, 4,18,2,1, fg);   // broader feet 2x1 for grounding
    // arms
    fill_cell(r,ox,oy,cs, 3,9,1,5, fg);    // left
    fill_cell(r,ox,oy,cs, 7,8,1,2, fg);    // right shoulder
    fill_cell(r,ox,oy,cs, 8,9,1,1, fg);    // elbow
    fill_cell(r,ox,oy,cs, 9,10,1,1, fg);   // wrist step
}

static void body_med(SDL_Renderer* r,int ox,int oy,int cs, SDL_Color fg){
    fill_cell(r,ox,oy,cs, 4,2,2,2, fg);  // head
    fill_cell(r,ox,oy,cs, 5,4,1,1, fg);  // neck
    fill_cell(r,ox,oy,cs, 3,5,3,1, fg);  // shoulders
    fill_cell(r,ox,oy,cs, 3,6,4,1, fg);
    fill_cell(r,ox,oy,cs, 4,7,2,5, fg);  // torso
    fill_cell(r,ox,oy,cs, 4,12,2,1, fg); // hips
    // legs (tiny gap) + subtle knee hint (1x1 offset)
    fill_cell(r,ox,oy,cs, 3,13,1,3, fg);
    fill_cell(r,ox,oy,cs, 3,16,1,2, fg);
    fill_cell(r,ox,oy,cs, 5,13,1,3, fg);
    fill_cell(r,ox,oy,cs, 5,16,1,2, fg);
    fill_cell(r,ox,oy,cs, 3,18,1,1, fg);
    fill_cell(r,ox,oy,cs, 5,18,1,1, fg);
    // arms
    fill_cell(r,ox,oy,cs, 2,9,1,4, fg);
    fill_cell(r,ox,oy,cs, 6,8,1,2, fg);
    fill_cell(r,ox,oy,cs, 7,9,1,1, fg);
    fill_cell(r,ox,oy,cs, 8,10,1,1, fg);
}

static void body_slim(SDL_Renderer* r,int ox,int oy,int cs, SDL_Color fg){
    fill_cell(r,ox,oy,cs, 4,2,2,2, fg);  // head
    fill_cell(r,ox,oy,cs, 5,4,1,1, fg);  // neck
    fill_cell(r,ox,oy,cs, 4,5,2,1, fg);  // narrow shoulders
    fill_cell(r,ox,oy,cs, 5,6,1,6, fg);  // thin torso
    fill_cell(r,ox,oy,cs, 5,12,1,1, fg); // tiny hip
    // long legs (1x7) with spacing
    fill_cell(r,ox,oy,cs, 4,13,1,7, fg);
    fill_cell(r,ox,oy,cs, 6,13,1,7, fg);
    fill_cell(r,ox,oy,cs, 4,20,1,1, fg);
    fill_cell(r,ox,oy,cs, 6,20,1,1, fg);
    // arms
    fill_cell(r,ox,oy,cs, 3,9,1,4, fg);
    fill_cell(r,ox,oy,cs, 7,8,1,2, fg);
    fill_cell(r,ox,oy,cs, 8,9,1,1, fg);
    fill_cell(r,ox,oy,cs, 9,10,1,1, fg);
}

// ---------------- sign (thick frame + terminal effect) ----------------
// Larger board: 9x9 cells total (thick frame = 1 cell; inner field = 7x7 cells).
// 5x5 glyph centered inside inner 7x7 → >=1-cell padding all around.
static void draw_sign(SDL_Renderer* r,int ox,int oy,int cs,SDL_Color fg,SDL_Color plate,char letter,float t){
    // Board anchor
    int bx = 12, by = 4; // top-left of the 9x9 board (relative to body origin)

    // Pole, aligned roughly under board center and attached to the bottom frame
    int pole_x = bx + 4; // center column of 9x9
    fill_cell(r,ox,oy,cs, pole_x, by+8, 1, 6, fg); // start on bottom frame row

    // Thick frame bands (1 cell)
    // Top & bottom (width 9)
    fill_cell(r,ox,oy,cs, bx,     by,     9,1, fg);
    fill_cell(r,ox,oy,cs, bx,     by+8,   9,1, fg);
    // Left & right (height 7 inside top/bottom)
    fill_cell(r,ox,oy,cs, bx,     by+1,   1,7, fg);
    fill_cell(r,ox,oy,cs, bx+8,   by+1,   1,7, fg);

    // Inner field 7x7
    fill_cell(r,ox,oy,cs, bx+1,   by+1,   7,7, plate);

    // 5x5 glyph (centered in 7x7 → start at bx+2,by+2)
    const unsigned char (*G)[5] = glyph5_for(letter);
    int gx = bx + 2, gy = by + 2;
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);

    // Blink + afterglow
    const float period=1.0f, duty=0.65f; float ph=fmodf(t,period); int on=(ph<duty);
    SDL_Color gcol = fg; gcol.a = on?255:70;
    for(int yy=0; yy<5; yy++){
        for(int xx=0; xx<5; xx++){
            if(G[yy][xx]) fill_cell(r,ox,oy,cs, gx+xx, gy+yy, 1,1, gcol);
        }
    }

    // Vertical CRT sweep inside 7x7 field (1px horizontal line)
    int inner_px_h = 7*cs;
    int scan_y = (int)fmodf(t*60.0f, (float)inner_px_h);
    int slx = ox + (bx+1)*cs + 1;  // +1px margins left/right
    int sly = oy + (by+1)*cs + scan_y;
    int slw = 7*cs - 2;
    if(sly >= oy + (by+1)*cs + 1 && sly < oy + (by+1)*cs + 7*cs - 1){
        SDL_SetRenderDrawColor(r, 255,255,255, 64);
        SDL_Rect sl={ slx, sly, slw, 1 };
        SDL_RenderFillRect(r,&sl);
    }
}

// ---------------- public API ----------------
void aa_draw_figure(SDL_Renderer* r, int x, int y, int cs,
                    SDL_Color fg, SDL_Color plate, char letter, float t, AA_FigType type){
    switch(type){
        case AA_FIG_WIDE:  body_wide (r,x,y,cs,fg); break;
        case AA_FIG_MED:   body_med  (r,x,y,cs,fg); break;
        default:           body_slim (r,x,y,cs,fg); break;
    }
    draw_sign(r,x,y,cs,fg,plate,letter,t); // only the SIGN has the thick frame
}
