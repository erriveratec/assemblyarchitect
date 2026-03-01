#ifndef INTRO_SCREEN_H
#define INTRO_SCREEN_H

#include <SDL.h>
#include <SDL_mixer.h>

extern char *GAME_TITLE;
extern char *PRESS_SPACE;

extern texture_t *g_press_space;
extern texture_t *g_title_background;
extern texture_t *g_chip;

extern texture_t *g_logo;
extern Mix_Chunk *g_studio_sfx;
extern Mix_Chunk *g_sfx_type;


int stage_studio(Uint64 start_time, Uint64 cur_time, bool key_pressed);
int stage_title(Uint64 start_time, Uint64 cur_time, const Uint8 *keystate);
int stage_select_level();
int stage_select_player();

#endif 

