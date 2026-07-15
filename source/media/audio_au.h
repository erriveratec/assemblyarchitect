#ifndef AUDIO_AU_H
#define AUDIO_AU_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL_mixer.h>

extern Mix_Chunk *g_sfx_type;
extern Mix_Chunk *g_sfx_ready;
extern Mix_Chunk *g_sfx_highlight;
extern Mix_Chunk *g_sfx_select;
extern Mix_Chunk *g_sfx_iface_back_cancel;
extern Mix_Chunk *g_sfx_iface_hover;
extern Mix_Chunk *g_sfx_menu;
extern Mix_Chunk *g_sfx_run_error;
extern Mix_Chunk *g_sfx_run_win;
extern Mix_Chunk *g_sfx_run_correct_val;

bool au_init_audio();
void au_load_audio_media();
Mix_Chunk *au_load_audio_file(const char *path);


#endif 
