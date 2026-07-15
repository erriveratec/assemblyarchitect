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
#include "arrow_ar.h"
#include "media/audio_au.h"

#define ERROR -1

SDL_Window *g_window = NULL;
SDL_Renderer *g_renderer = NULL;
SDL_Surface *g_screen = NULL;
SDL_Texture *g_screen_texture = NULL;

int g_width;
int g_height;

TTF_Font *g_font = NULL;


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
	
	/* Destroy the renderer SDL_CreateWindowAndRenderer created and recreate 
	with vsync */
	SDL_DestroyRenderer(g_renderer);
	g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED 
								    | SDL_RENDERER_PRESENTVSYNC);
	if (!g_renderer) {
   		SDL_Log("CreateRenderer failed: %s", SDL_GetError());
		SDL_DestroyWindow(g_window);
		return -1;
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
	char path[512];

	ax_get_resource_path(path, sizeof(path), "img/stop.png");
	stop_button = dw_load_texture(path);
	if (stop_button == NULL){
		printf("Could not load the level image SDL_Error: %s\n",
				SDL_GetError());
		return FAIL;
	}

	ax_get_resource_path(path, sizeof(path), "img/fast.png");
	fast_button = dw_load_texture(path);
	if (fast_button == NULL){
		printf("Could not load the level image SDL_Error: %s\n",
				SDL_GetError());
		return FAIL;
	}

	ax_get_resource_path(path, sizeof(path), "img/play.png");
	play_button = dw_load_texture(path);
	if (play_button == NULL){
		printf("Could not load the level image SDL_Error: %s\n",
				SDL_GetError());
		return FAIL;
	}

	ax_get_resource_path(path, sizeof(path), "img/step-forward.png");
	step_forward_button = dw_load_texture(path);
	if (step_forward_button == NULL){
		printf("Could not load the level image SDL_Error: %s\n",
				SDL_GetError());
		return FAIL;
	}

	ax_get_resource_path(path, sizeof(path), "img/return.png");
	return_button = dw_load_texture(path);
	if (return_button == NULL){
		printf("Could not load the level image SDL_Error: %s\n",
				SDL_GetError());
		return FAIL;
	}

	ax_get_resource_path(path, sizeof(path), "img/reset.png");
	reset_button = dw_load_texture(path);
	if (reset_button == NULL){
		printf("Could not load the level image SDL_Error: %s\n",
				SDL_GetError());
		return FAIL;
	}

	ax_get_resource_path(path, sizeof(path), "img/rightarrow.png");
	g_reg_arrow = dw_load_texture(path);
	if (g_reg_arrow == NULL){
		printf("Could not load the level image SDL_Error: %s\n",
				SDL_GetError());
		return FAIL;
	}
	
	ax_get_resource_path(path, sizeof(path), "img/rightarrow.png");
	g_imm_up_arrow = dw_load_texture(path);
	if (g_imm_up_arrow == NULL){
		printf("Could not load the level image SDL_Error: %s\n",
				SDL_GetError());
		return FAIL;
	}

	ax_get_resource_path(path, sizeof(path), "img/rightarrow.png");
	g_lv_arrow = dw_load_texture(path);
	if (g_lv_arrow == NULL){
		printf("Could not load the level image SDL_Error: %s\n",
				SDL_GetError());
		return FAIL;
	}

	ax_get_resource_path(path, sizeof(path), "img/rightarrow.png");
	g_ib_arrow = dw_load_texture(path);
	if (g_ib_arrow == NULL){
		printf("Could not load the level image SDL_Error: %s\n",
				SDL_GetError());
		return FAIL;
	}
	
	ax_get_resource_path(path, sizeof(path), "img/rightarrow.png");
	g_ob_arrow = dw_load_texture(path);
	if (g_ob_arrow == NULL){
		printf("Could not load the level image SDL_Error: %s\n",
				SDL_GetError());
		return FAIL;
	}

	ax_get_resource_path(path, sizeof(path), "img/rightarrow.png");
	g_exec_arrow = dw_load_texture(path);
	if (g_exec_arrow == NULL){
		printf("Could not load the level image SDL_Error: %s\n",
				SDL_GetError());
		return FAIL;
	}

	ax_get_resource_path(path, sizeof(path), "img/chip.png");
	g_chip = dw_load_texture(path);
	if (g_chip == NULL){
		printf("Could not load the level image SDL_Error: %s\n",
				SDL_GetError());
		return FAIL;
	}

	//Open the font
	ax_get_resource_path(path, sizeof(path), "fonts/DOSVGA437.ttf");
	g_font = TTF_OpenFont(path, 130); //96 old value point size
	if (NULL == g_font){
		printf("Failed to load font! SDL_ttf Error: %s\n", 
				TTF_GetError());
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

//    if (g_studio_sfx)  Mix_FreeChunk(g_studio_sfx);
    if (g_sfx_type) Mix_FreeChunk(g_sfx_type);
    if (g_sfx_ready) Mix_FreeChunk(g_sfx_ready);
    TTF_CloseFont(g_font);
    SDL_DestroyRenderer(g_renderer);
    SDL_DestroyWindow(g_window);
    Mix_CloseAudio();
    TTF_Quit();
    SDL_Quit();
}
