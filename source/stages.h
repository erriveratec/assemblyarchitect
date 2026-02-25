#ifndef STAGES_H
#define STAGES_H

#include <SDL.h>

extern texture_array_t *g_game_title;
extern texture_t *g_press_space;
extern texture_array_t *g_win_text;
extern texture_t *g_title_background;
extern texture_t *g_logo;
extern texture_t *g_chip;

extern bool g_quit;

int stage_title(const Uint8 *keystate);
int stage_select_level();
int stage_select_player();
int stage_level(int level);
int stage_studio(Uint64 start_time, Uint64 cur_time, bool key_pressed);

#define STUDIO_NAME_TEXT "One Man Studio"
#define GAME_TITLE_TEXT "ASSEMBLY ARCHITECT"
#define PRESS_SPACE_TEXT "PRESS SPACE"
#define WIN_TEXT "The challenge is correct"
#endif
