#include <assert.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <stdio.h>
#include <string.h>
#include "sdl_config.h"
#include "intro_screen_is.h"
#include "aux.h"
#include "draw_dw.h"
#include "stage_buttons_sb.h"
#include "registers_rg.h"
#include "buffers_bf.h"
#include "game_mechanics_mc.h"
#include "levels_lv.h"
#include "arrow_ar.h"
#include "stages.h"

#define ERROR -1

SDL_Window *g_window = NULL;
SDL_Renderer *g_renderer = NULL;
SDL_Surface *g_screen = NULL;
SDL_Texture *g_screen_texture = NULL;

int g_width;
int g_height;

TTF_Font *g_font = NULL;

Mix_Chunk *g_studio_sfx = NULL;
Mix_Chunk *g_sfx_type = NULL;
Mix_Chunk *g_sfx_ready = NULL;
Mix_Chunk *g_sfx_highlight = NULL;
Mix_Chunk *g_sfx_select = NULL;


static bool init_audio();



/* Function: init_audio
 * -------------------------------------
 * Initializes the audio for the sdl game
 *
 * Arguments:
 *	None.
 *
 * Return:
 *	boolean with the state
 */
static bool init_audio()
{
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) < 0) {
		SDL_Log("Mix_OpenAudio: %s", Mix_GetError());
		return false;
	}
	
	Mix_AllocateChannels(8);

	Mix_Volume(-1, (int)(MIX_MAX_VOLUME * 0.85f));

	return true;
}



/* Function: init_sdl
 * -------------------------------------
 * This function initialises what is necessary for SDL
 * to work.
 *
 * Arguments:
 *	None.
 *
 * Return:
 *	Void.
 */
int init_sdl(int width, int height, int argc, char *args[])
{
	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0){
		printf("SDL could not initialize! SDL_Error: %s\n", 
				SDL_GetError());
		return FAIL;
	}
	//Set texture filtering to linear	
	if (SDL_FALSE == SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")){
		printf("Warning: Linear texture filtering could not be enabled!");
	}

	if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) < 0){
		SDL_Log("Mix_OpenAudio failed: %s", Mix_GetError());
		return FAIL;
	}

	//Create window
	for (int i = 0; i < argc; i++){
		if (strcmp(args[i], "-f")){
			SDL_CreateWindowAndRenderer(width,
										height,
										SDL_WINDOW_SHOWN,
										&g_window,
										&g_renderer);
		} else {
			SDL_CreateWindowAndRenderer(width,
										height,
										SDL_WINDOW_FULLSCREEN_DESKTOP,
										&g_window,
										&g_renderer);
		}
	}
	if (g_window == NULL){
		printf("Window could not be created! SDL_Error: %s\n", 
				SDL_GetError());
		return FAIL;
	}
	
	g_screen =  SDL_CreateRGBSurfaceWithFormat(
				0, width, height, 32, SDL_PIXELFORMAT_RGBA32);

	if (g_screen == NULL){
		printf("Could not create the screen surface! SDL_Error: %s\n", 
				SDL_GetError());
		return FAIL;
	}

	// Create the screen texture
	g_screen_texture = SDL_CreateTextureFromSurface(g_renderer, 
														g_screen);
	if (g_screen_texture == NULL){
		printf("Could not create the screen texture! SDL_Error: %s\n", 
				SDL_GetError());
		return FAIL;
	}

	if (TTF_Init() == ERROR){
		printf("SDL_ttf could not be initialized! SDL ttf Error: %s \n",
			   TTF_GetError());
		return FAIL;
	}	

	return SUCCESS;

}




/* Function: load_media
 * ----------------------------
 * Arguments: 
 * 	None.
 *
 * Return:
 *	SUCCESS or FAIL
 */
int load_media()
{

	stop_button = load_texture_from_file("img/stop.png");
	if (stop_button == NULL){
		printf("Could not load the level image SDL_Error: %s\n",
				SDL_GetError());
		return FAIL;
	}

	fast_button = load_texture_from_file("img/fast.png");
	if (fast_button == NULL){
		printf("Could not load the level image SDL_Error: %s\n",
				SDL_GetError());
		return FAIL;
	}

	play_button = load_texture_from_file("img/play.png");
	if (play_button == NULL){
		printf("Could not load the level image SDL_Error: %s\n",
				SDL_GetError());
		return FAIL;
	}

	step_forward_button = load_texture_from_file("img/step-forward.png");
	if (step_forward_button == NULL){
		printf("Could not load the level image SDL_Error: %s\n",
				SDL_GetError());
		return FAIL;
	}

	return_button = load_texture_from_file("img/return.png");
	if (return_button == NULL){
		printf("Could not load the level image SDL_Error: %s\n",
				SDL_GetError());
		return FAIL;
	}

	reset_button = load_texture_from_file("img/reset.png");
	if (reset_button == NULL){
		printf("Could not load the level image SDL_Error: %s\n",
				SDL_GetError());
		return FAIL;
	}

	g_reg_arrow = load_texture_from_file("img/rightarrow.png");
	if (g_reg_arrow == NULL){
		printf("Could not load the level image SDL_Error: %s\n",
				SDL_GetError());
		return FAIL;
	}
	
	g_imm_up_arrow = load_texture_from_file("img/rightarrow.png");
	if (g_imm_up_arrow == NULL){
		printf("Could not load the level image SDL_Error: %s\n",
				SDL_GetError());
		return FAIL;
	}

	g_lv_arrow = load_texture_from_file("img/rightarrow.png");
	if (g_lv_arrow == NULL){
		printf("Could not load the level image SDL_Error: %s\n",
				SDL_GetError());
		return FAIL;
	}

	g_ib_arrow = load_texture_from_file("img/rightarrow.png");
	if (g_ib_arrow == NULL){
		printf("Could not load the level image SDL_Error: %s\n",
				SDL_GetError());
		return FAIL;
	}
	g_ob_arrow = load_texture_from_file("img/rightarrow.png");
	if (g_ob_arrow == NULL){
		printf("Could not load the level image SDL_Error: %s\n",
				SDL_GetError());
		return FAIL;
	}

	g_exec_arrow = load_texture_from_file("img/rightarrow.png");
	if (g_exec_arrow == NULL){
		printf("Could not load the level image SDL_Error: %s\n",
				SDL_GetError());
		return FAIL;
	}
	
	g_logo = load_texture_from_file("img/oms.png");
	if (g_logo == NULL){
		printf("Could not load the level image SDL_Error: %s\n",
				SDL_GetError());
		return FAIL;
	}

	g_chip = load_texture_from_file("img/chip.png");
	if (g_chip == NULL){
		printf("Could not load the level image SDL_Error: %s\n",
				SDL_GetError());
		return FAIL;
	}

	//Open the font
	g_font = TTF_OpenFont("DOSVGA437.ttf", 130); //96 old value point size
	if (NULL == g_font){
		printf("Failed to load font! SDL_ttf Error: %s\n", 
				TTF_GetError());
		return FAIL;
	} 
	
	//Audio downloaded
	g_studio_sfx = Mix_LoadWAV("sound/studio.wav");
	if (g_studio_sfx == NULL){
		SDL_Log("Mix_LoadWAC: %s", Mix_GetError());
		return FAIL;
	}
	
	g_sfx_type = Mix_LoadWAV("sound/key_press.wav");
	if (g_sfx_type == NULL){
		SDL_Log("Mix_LoadWAC: %s", Mix_GetError());
		return FAIL;
	}

	g_sfx_ready = Mix_LoadWAV("sound/ready.wav");
	if (g_sfx_ready == NULL){
		SDL_Log("Mix_LoadWAC: %s", Mix_GetError());
		return FAIL;
	}
	g_sfx_highlight = Mix_LoadWAV("sound/click_ping.wav");
	if (g_sfx_highlight == NULL){
		SDL_Log("Mix_LoadWAC: %s", Mix_GetError());
		return FAIL;
	}

	g_sfx_select = Mix_LoadWAV("sound/ping_confirm.wav");
	if (g_sfx_select == NULL){
		SDL_Log("Mix_LoadWAC: %s", Mix_GetError());
		return FAIL;
	}
	return SUCCESS;
}

/* Function: close_sdl
 * ----------------------------------------------------------------------------
 * 	Closes all the resources regarding sdl.
 *
 * Arguments: 
 * 	None.
 *
 * Return:
 *	SUCCESS or FAIL
 */
void close_sdl()
{

    if (g_studio_sfx)  Mix_FreeChunk(g_studio_sfx);
    if (g_sfx_type) Mix_FreeChunk(g_sfx_type);
    if (g_sfx_ready) Mix_FreeChunk(g_sfx_ready);
    TTF_CloseFont(g_font);
    SDL_DestroyRenderer(g_renderer);
    SDL_DestroyWindow(g_window);
    Mix_CloseAudio();
    TTF_Quit();
    SDL_Quit();
}
