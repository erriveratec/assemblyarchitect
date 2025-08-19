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

enum stageStates{
	NO_STAGE,
	QUIT,
	STUDIO_SCREEN,
	TITLE_SCREEN,
	SELECT_PLAYER_SCREEN,
	INIT_LEVEL_SELECTION,
	LEVEL_SELECTION,
	INITIALIZE_LEVEL,
	PLAY_LEVEL
};
#endif
