#pragma once
#include <SDL.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct fx_electron_t fx_electron_t;

fx_electron_t* fx_electron_create(SDL_Renderer* r, 
									 int window_w, 
									 int window_h, 
									 const char* electron_png_or_null);
void           fx_electron_update(fx_electron_t* fx, float dt);
void           fx_electron_render(fx_electron_t* fx, SDL_Renderer* r);
void           aa_electron_fx_destroy(fx_electron_t* fx);

#ifdef __cplusplus
}
#endif
