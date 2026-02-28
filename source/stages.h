#ifndef STAGES_H
#define STAGES_H

#include <SDL.h>
#include <SDL_mixer.h>

extern texture_array_t *g_win_text;
extern bool g_quit;
extern int g_player;

int stage_level(int level);

#define WIN_TEXT "The challenge is correct"
#endif
