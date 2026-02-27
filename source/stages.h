#ifndef STAGES_H
#define STAGES_H

#include <SDL.h>
#include <SDL_mixer.h>

extern texture_array_t *g_game_title;
extern texture_t *g_press_space;
extern texture_array_t *g_win_text;
extern texture_t *g_title_background;
extern texture_t *g_chip;


extern bool g_quit;

int stage_title(const Uint8 *keystate);
int stage_select_level();
int stage_select_player();
int stage_level(int level);

#define GAME_TITLE_TEXT "ASSEMBLY ARCHITECT"
#define PRESS_SPACE_TEXT "PRESS SPACE"
#define WIN_TEXT "The challenge is correct"
#endif
