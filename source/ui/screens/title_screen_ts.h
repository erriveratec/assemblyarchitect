#ifndef INTRO_SCREEN_H
#define INTRO_SCREEN_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL.h>
#include "draw_dw.h"

extern texture_t *g_logo;

int stage_studio(Uint64 start_time, Uint64 cur_time, bool key_pressed);

#endif 
