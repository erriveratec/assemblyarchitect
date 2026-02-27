#ifndef INTRO_SCREEN_H
#define INTRO_SCREEN_H

#include <SDL.h>
#include <SDL_mixer.h>


extern texture_t *g_logo;
extern Mix_Chunk *g_studio_sfx;


int stage_studio(Uint64 start_time, Uint64 cur_time, bool key_pressed);
#endif 

