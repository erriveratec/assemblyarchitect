#ifndef INTRO_SCREEN_H
#define INTRO_SCREEN_H

#include <SDL.h>
#include <SDL_mixer.h>

extern char *GAME_TITLE;
extern char *PRESS_SPACE;

extern texture_t *g_press_space;
extern texture_t *g_chip;
extern texture_t *g_logo;


int stage_studio(Uint64 start_time, Uint64 cur_time, bool key_pressed);
int stage_title(const Uint8 *keystate);
int stage_select_level();
int stage_select_player();
int stage_select_sector();
int stage_sector_0();
int stage_sector_1();

#endif 

