#ifndef STAGES_H
#define STAGES_H

#include <SDL.h>


int stage_title(const Uint8 *keystate);
int stage_select_level();
int stage_select_player();
int stage_level(int level);
int stage_studio(Uint64 start_time, Uint64 cur_time);
void player_pressed_escape_key();


#endif
