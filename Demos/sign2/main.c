#include <SDL.h>
#include "aa_humans.h"

// Minimal demo using the unified API: aa_draw_human(...)
int main(){
    if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER)!=0){ SDL_Log("SDL_Init failed: %s", SDL_GetError()); return 1; }
    SDL_Window* w = SDL_CreateWindow("Unified Pixel Human API", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_SHOWN);
    SDL_Renderer* r = SDL_CreateRenderer(w,-1,SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);

    const int cs = 12; // block size / scale
    const SDL_Color WHITE = {255,255,255,255};
    const SDL_Color PLATE = {16,16,16,255};

    float t=0.f; Uint64 f=SDL_GetPerformanceFrequency(), prev=SDL_GetPerformanceCounter();
    int running=1; SDL_Event e; int mx=0,my=0; int hoverMask=0; int pressedMask=0;

    struct Item { AA_FigureType type; int x,y; char L; } A[3] = {
        {AA_FIG_WIDE, 260,240,'X'},
        {AA_FIG_MED,  610,240,'Y'},
        {AA_FIG_SLIM, 960,240,'Z'}
    };

    while(running){
        while(SDL_PollEvent(&e)){
            if(e.type==SDL_QUIT) running=0;
            else if(e.type==SDL_MOUSEMOTION){ mx=e.motion.x; my=e.motion.y; }
            else if(e.type==SDL_MOUSEBUTTONDOWN && e.button.button==SDL_BUTTON_LEFT){
                for(int i=0;i<3;i++){ SDL_Rect s=aa_sign_rect_px(A[i].x,A[i].y,cs); if(mx>=s.x&&mx<s.x+s.w&&my>=s.y&&my<s.y+s.h) pressedMask|=(1<<i); }
            }
            else if(e.type==SDL_MOUSEBUTTONUP && e.button.button==SDL_BUTTON_LEFT){ pressedMask=0; }
            else if(e.type==SDL_KEYDOWN && e.key.keysym.sym==SDLK_ESCAPE){ running=0; }
        }

        Uint64 now=SDL_GetPerformanceCounter(); float dt=(float)((double)(now-prev)/(double)f); prev=now; t+=dt;

        // Hover mask per sign
        hoverMask=0; for(int i=0;i<3;i++){ SDL_Rect s=aa_sign_rect_px(A[i].x,A[i].y,cs); if(mx>=s.x&&mx<s.x+s.w&&my>=s.y&&my<s.y+s.h) hoverMask|=(1<<i); }

        SDL_SetRenderDrawColor(r,0,0,0,255); SDL_RenderClear(r);
        for(int i=0;i<3;i++){
            int opts = AA_OPT_NONE;
            if(hoverMask & (1<<i))  opts |= AA_OPT_LIFT;   // lift + halo on hover
            if(pressedMask & (1<<i)) opts |= AA_OPT_INVERT;// flash invert while mouse down
            aa_draw_human(r, A[i].type, A[i].x, A[i].y, cs, WHITE, PLATE, A[i].L, t, opts);
        }
        SDL_RenderPresent(r);
    }

    SDL_DestroyRenderer(r); SDL_DestroyWindow(w); SDL_Quit();
    return 0;
}
