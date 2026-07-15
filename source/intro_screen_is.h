#ifndef INTRO_SCREEN_H
#define INTRO_SCREEN_H

#include <SDL.h>
#include <SDL_mixer.h>
#include "draw_dw.h"

extern char *GAME_TITLE;
extern char *PRESS_SPACE;

extern texture_t *g_press_space;
extern texture_t *g_chip;


int stage_select_player();
int stage_select_sector();
int stage_sector_0();
int stage_sector_1();

#endif 

