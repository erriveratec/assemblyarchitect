#ifndef STAGES_H
#define STAGES_H

#include <SDL.h>

extern bool g_quit;

int stage_title(const Uint8 *keystate);
int stage_select_level();
int stage_select_player();
int stage_level(int level);
int stage_studio(Uint64 start_time, Uint64 cur_time);
void toggle_escape_menu();
void set_quit_game();
bool get_quit_game_value();
void level_initialization(int level_id);


#endif
