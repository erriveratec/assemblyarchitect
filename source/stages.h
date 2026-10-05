#ifndef STAGES_H
#define STAGES_H

#include <SDL.h>
#include <SDL_mixer.h>
#include <stdbool.h>
#include "domain/program.h"

extern bool g_quit;
extern int g_player;

void init_level(int level, aa_program_t *program);
int stage_level(int level, aa_program_t *program);
void stages_cancel_pending_edit(void);

#endif
