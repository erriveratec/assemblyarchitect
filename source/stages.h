#ifndef STAGES_H
#define STAGES_H

#include <SDL.h>
#include <SDL_mixer.h>
#include <stdbool.h>
#include "domain/program.h"
#include "code_line_cl.h"

extern bool g_quit;
extern int g_player;

void init_level(int level, aa_program_t *program);
int stage_level(int level, aa_program_t *program);
void stages_cancel_pending_edit(void);
void stages_cancel_edit_interaction(void);
void stages_forget_destroyed_line(const code_line_t *line);
bool stages_refresh_program_snapshot(aa_program_t *program);
bool stages_reconcile_program_snapshot(aa_program_t *program);

#ifdef STAGES_EDIT_TESTING
typedef struct stages_edit_test_state {
	code_line_t *line;
	bool authorized;
	bool snapshot_valid;
	int result;
	unsigned legacy_fallbacks;
} stages_edit_test_state_t;
extern void (*stages_before_edit_for_test)(void);
stages_edit_test_state_t stages_edit_state_for_test(void);
#endif

#endif
