#include <assert.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <string.h>
#include "sdl_config.h"
#include "aux.h"
#include "draw_dw.h"
#include "stage_buttons_sb.h"
#include "registers_rg.h"
#include "buffers_bf.h"
#include "game_mechanics_mc.h"
#include "levels_lv.h"
#include "arrow_ar.h"

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
	if (SDL_Init(SDL_INIT_VIDEO) < 0){
		
		printf("SDL could not initialize! SDL_Error: %s\n", 
				SDL_GetError());
		return FAIL;

	}// end if
	//Set texture filtering to linear	
	if (SDL_FALSE == SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")){
		printf("Warning: Linear texture filtering could not be enabled!");
	}//end if

	for (int i = 0; i < argc; i++){
		
		//Create window
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
		}// end else
	}//end for
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

	step_back_button = load_texture_from_file("img/step-back.png");
	if (step_back_button == NULL){
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

	g_reg_arrow = load_texture_from_file("img/rightarrow.png");
	if (return_button == NULL){
		printf("Could not load the level image SDL_Error: %s\n",
				SDL_GetError());
		return FAIL;
	}
	
	g_lv_arrow = load_texture_from_file("img/rightarrow.png");
	if (return_button == NULL){
		printf("Could not load the level image SDL_Error: %s\n",
				SDL_GetError());
		return FAIL;
	}
	g_ib_arrow = load_texture_from_file("img/rightarrow.png");
	if (return_button == NULL){
		printf("Could not load the level image SDL_Error: %s\n",
				SDL_GetError());
		return FAIL;
	}
	g_ob_arrow = load_texture_from_file("img/rightarrow.png");
	if (return_button == NULL){
		printf("Could not load the level image SDL_Error: %s\n",
				SDL_GetError());
		return FAIL;
	}

	g_exec_arrow = load_texture_from_file("img/rightarrow.png");
	if (return_button == NULL){
		printf("Could not load the level image SDL_Error: %s\n",
				SDL_GetError());
		return FAIL;
	}

	//Open the font
	g_font = TTF_OpenFont("AnomalyMono.ttf", 130); //96 old value point size
	//g_font = TTF_OpenFont("EnigmaSans.ttf", 96); //96 old value point size
	if (NULL == g_font){
		printf("Failed to load font! SDL_ttf Error: %s\n", 
				TTF_GetError());
		return FAIL;
	} 

	return SUCCESS;
}


