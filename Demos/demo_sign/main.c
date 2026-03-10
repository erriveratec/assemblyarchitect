#include <SDL.h>
#include "aa_figures.h"

// Demo: three pixel humans (WIDE / MED / SLIM) aligned to the same baseline.
// Larger sign (9x9 board, 7x7 inner, 5x5 glyph) – only the sign has a thick frame.

int main(){
    if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER)!=0){ SDL_Log("SDL_Init failed: %s", SDL_GetError()); return 1; }
    SDL_Window* w = SDL_CreateWindow("Pixel Humans – Larger Sign + Better Silhouettes", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_SHOWN);
    if(!w){ SDL_Log("CreateWindow failed: %s", SDL_GetError()); return 1; }
    SDL_Renderer* r = SDL_CreateRenderer(w,-1,SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
    if(!r){ SDL_Log("CreateRenderer failed: %s", SDL_GetError()); return 1; }

    const int cs = 12; // block size
    const SDL_Color WHITE = {255,255,255,255};
    const SDL_Color PLATE = {16,16,16,255};

    float t=0.f; Uint64 f=SDL_GetPerformanceFrequency(); Uint64 prev=SDL_GetPerformanceCounter();
    int run=1; SDL_Event e;

    // Same Y for alignment
    int Y = 240;

    while(run){
        while(SDL_PollEvent(&e)){
            if(e.type==SDL_QUIT) run=0; if(e.type==SDL_KEYDOWN && e.key.keysym.sym==SDLK_ESCAPE) run=0;
        }
        Uint64 now=SDL_GetPerformanceCounter(); float dt=(float)((double)(now-prev)/(double)f); prev=now; t+=dt;

        SDL_SetRenderDrawColor(r,0,0,0,255); SDL_RenderClear(r);

        aa_draw_figure(r, 260, Y, cs, WHITE, PLATE, 'X', t, AA_FIG_WIDE);
        aa_draw_figure(r, 610, Y, cs, WHITE, PLATE, 'Y', t, AA_FIG_MED);
        aa_draw_figure(r, 960, Y, cs, WHITE, PLATE, 'Z', t, AA_FIG_SLIM);

        SDL_RenderPresent(r);
    }

    SDL_DestroyRenderer(r); SDL_DestroyWindow(w); SDL_Quit();
    return 0;
}
