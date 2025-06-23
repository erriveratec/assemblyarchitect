#ifndef GAME_MECHANICS_H
#define GAME_MECHANICS_H
#include "draw_dw.h"
#include "button_bt.h"
#include "code_window_cw.h"

void draw_avatar();
void reset_avatar();
bool mc_run_code();
void generate_win_condition_list(int level);

#endif
