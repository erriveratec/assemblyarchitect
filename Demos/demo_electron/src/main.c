#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include "electron_fx.h"

int main(){
  const int W=1280, H=720;
  if(SDL_Init(SDL_INIT_VIDEO) < 0){ SDL_Log("SDL_Init: %s", SDL_GetError()); return 1; }
  int imgFlags=IMG_INIT_PNG; if((IMG_Init(imgFlags)&imgFlags)!=imgFlags){ SDL_Log("IMG_Init PNG failed: %s", IMG_GetError()); }

  SDL_Window* win = SDL_CreateWindow("AA Electron FX — EXACT", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, W, H, SDL_WINDOW_SHOWN);
  if(!win){ SDL_Log("CreateWindow: %s", SDL_GetError()); return 1; }
  SDL_Renderer* r = SDL_CreateRenderer(win,-1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if(!r){ SDL_Log("CreateRenderer: %s", SDL_GetError()); return 1; }

  // Pass NULL so it uses the same radial fallback as your original code if PNG is absent
  fx_electron_t* fx = fx_electron_create(r, W, H, NULL);

  int running=1; Uint32 prev=SDL_GetTicks();
  while(running){
    SDL_Event ev; while(SDL_PollEvent(&ev)){
      if(ev.type==SDL_QUIT) running=0; if(ev.type==SDL_KEYDOWN && (ev.key.keysym.sym==SDLK_ESCAPE || ev.key.keysym.sym==SDLK_SPACE || ev.key.keysym.sym==SDLK_RETURN)) running=0;
    }
    Uint32 now=SDL_GetTicks(); float dt=(now - prev)/1000.0f; prev=now;
    fx_electron_update(fx, dt);
    fx_electron_render(fx, r);
    SDL_RenderPresent(r);
  }

  aa_electron_fx_destroy(fx);
  SDL_DestroyRenderer(r); SDL_DestroyWindow(win);
  IMG_Quit(); SDL_Quit();
  return 0;
}
