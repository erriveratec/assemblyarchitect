#pragma once
#include <SDL.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct AA_ElectronFX AA_ElectronFX;

AA_ElectronFX* aa_electron_fx_create(SDL_Renderer* r, 
									 int window_w, 
									 int window_h, 
									 const char* electron_png_or_null);
void           aa_electron_fx_update(AA_ElectronFX* fx, float dt);
void           aa_electron_fx_render(AA_ElectronFX* fx, SDL_Renderer* r);
void           aa_electron_fx_destroy(AA_ElectronFX* fx);

#ifdef __cplusplus
}
#endif
