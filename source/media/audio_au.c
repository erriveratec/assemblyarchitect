#include <stdbool.h>
#include <SDL.h>
#include <SDL_mixer.h>
#include "aux.h"

Mix_Chunk *g_studio_sfx = NULL;
Mix_Chunk *g_sfx_type = NULL;
Mix_Chunk *g_sfx_ready = NULL;
Mix_Chunk *g_sfx_highlight = NULL;
Mix_Chunk *g_sfx_select = NULL;
Mix_Chunk *g_sfx_iface_back_cancel = NULL;
Mix_Chunk *g_sfx_iface_hover = NULL;
Mix_Chunk *g_sfx_menu = NULL;
Mix_Chunk *g_sfx_run_error = NULL;
Mix_Chunk *g_sfx_run_win = NULL;
Mix_Chunk *g_sfx_run_correct_val = NULL;


/* Function: au_init_audio
 * -------------------------------------
 * Initializes the audio for the sdl game
 *
 * Arguments:
 *	None.
 *
 * Return:
 *	boolean with the state
 */
bool au_init_audio()
{
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) < 0) {
		SDL_Log("Mix_OpenAudio: %s", Mix_GetError());
		return false;
	}
	
	Mix_AllocateChannels(8);

	Mix_Volume(-1, (int)(MIX_MAX_VOLUME * 0.85f));

	return true;
}


/* Function: au_load_audio_media
 * ----------------------------------------------------------------------------
 * Loads the audio media for the sdl game
 *
 * Arguments:
 *	None.
 *
 * Return:
 *	Void.
 */
void au_load_audio_media()
{
	char path[512];
//Audio downloaded
	ax_get_resource_path(path, sizeof(path), "sound/intro_studio.wav");
	g_studio_sfx = Mix_LoadWAV(path);
	if (g_studio_sfx == NULL){
		SDL_Log("Mix_LoadWAC: %s", Mix_GetError());
	}
	
	ax_get_resource_path(path, sizeof(path), "sound/key_press.wav");
	g_sfx_type = Mix_LoadWAV(path);
	if (g_sfx_type == NULL){
		SDL_Log("Mix_LoadWAC: %s", Mix_GetError());
	}

	ax_get_resource_path(path, sizeof(path), "sound/type_ready.wav");
	g_sfx_ready = Mix_LoadWAV(path);
	if (g_sfx_ready == NULL){
		SDL_Log("Mix_LoadWAC: %s", Mix_GetError());
	}
	
	ax_get_resource_path(path, sizeof(path), "sound/player_highlight.wav");
	g_sfx_highlight = Mix_LoadWAV(path);
	if (g_sfx_highlight == NULL){
		SDL_Log("Mix_LoadWAC: %s", Mix_GetError());
	}

	ax_get_resource_path(path, sizeof(path), "sound/interface_select.wav");
	g_sfx_select = Mix_LoadWAV(path);
	if (g_sfx_select == NULL){
		SDL_Log("Mix_LoadWAC: %s", Mix_GetError());
	}
	
	ax_get_resource_path(path, sizeof(path), "sound/interface_back_cancel.wav");
	g_sfx_iface_back_cancel = Mix_LoadWAV(path);
	if (g_sfx_iface_back_cancel == NULL){
		SDL_Log("Mix_LoadWAC: %s", Mix_GetError());
	}

	ax_get_resource_path(path, sizeof(path), "sound/interface_hover.wav");
	g_sfx_iface_hover = Mix_LoadWAV(path);
	if (g_sfx_iface_hover == NULL){
		SDL_Log("Mix_LoadWAC: %s", Mix_GetError());
	}
	
	ax_get_resource_path(path, sizeof(path), "sound/menu_escape.wav");
	g_sfx_menu = Mix_LoadWAV(path);
	if (g_sfx_menu == NULL){
		SDL_Log("Mix_LoadWAC: %s", Mix_GetError());
	}

	ax_get_resource_path(path, sizeof(path), "sound/run_error.wav");
	g_sfx_run_error = Mix_LoadWAV(path);
	if (g_sfx_run_error == NULL){
		SDL_Log("Mix_LoadWAC: %s", Mix_GetError());
	}

	ax_get_resource_path(path, sizeof(path), "sound/run_win.wav");
	g_sfx_run_win = Mix_LoadWAV(path);
	if (g_sfx_run_win == NULL){
		SDL_Log("Mix_LoadWAC: %s", Mix_GetError());
	}

	ax_get_resource_path(path, sizeof(path), "sound/run_correct_value.wav");
	g_sfx_run_correct_val = Mix_LoadWAV(path);
	if (g_sfx_run_correct_val == NULL){
		SDL_Log("Mix_LoadWAC: %s", Mix_GetError());
	}
	Mix_VolumeChunk(g_sfx_run_correct_val, 64);

}
