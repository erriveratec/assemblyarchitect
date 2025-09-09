#include <stdbool.h>
#include <assert.h>
#include "mouse_ms.h"
#include "sdl_config.h"
#include "aux.h"
#include "stages.h"
#include "levels_lv.h"
#include "file_fl.h"
#include "stage_buttons_sb.h"
#include "dimensions_dm.h"
#include "text_tx.h"
#include "instruction_window_iw.h"
#include "registers_rg.h"
#include "buffers_bf.h"
#include "game_mechanics_mc.h"
#include "code_window_cw.h"

#define BG_COLOR_BLACK 0x000000FF
#define BG_COLOR_RED 0xFF0000FF
#define BG_COLOR_GREEN 0x00FF00FF

static void initialize_game_assets();

/* Function: initialize_game_assets
 * ----------------------------------------------------------------------------
 * Initializes the assets of the game
 *
 * Arguments:
 * 	Void.
 *
 * Return:
 *	LV_STUDIO_SCREEN
 */
static void initialize_game_assets()
{
	g_studio_name = dw_create_text_texture(STUDIO_NAME_TEXT, C_WHITE);
	g_game_title = dw_create_text_texture(GAME_TITLE_TEXT, C_WHITE);
	g_press_space = dw_create_text_texture(PRESS_SPACE_TEXT, C_WHITE);
	g_win_text = dw_create_text_texture(WIN_TEXT, C_BLACK);

	init_escape_menu();
	sb_init_return_button();
	tx_init_global_msgs();
	iw_init_ins_box_texture();
	rg_init_reg_texture();
	bf_init_buffer_assets();
	mc_init_errors_texture();
	cw_init_code_window_texture();
}


int main(int argc, char *args[])
{
	dm_set_screen_resolution(R1600X900);
	int screen_width = dm_get_screen_width();
	int screen_height = dm_get_screen_height();

	if (init_sdl(screen_width, screen_height, argc, args) == FAIL){
		printf("SDL could not be initialized");
		return FAIL;
	}
	SDL_PixelFormat *format = g_screen->format;
	SDL_GetWindowSize(g_window, &g_width, &g_height);
	
	load_media();
	fl_save_file_init();
	initialize_game_assets();

	int sleep = 0;
	int state = LV_STUDIO_SCREEN;
	int level = LV_NO_LEVEL;
	int stage = LV_NO_STAGE;
	Uint64 next_game_tick = SDL_GetTicks64();
	Uint64 studio_screen_time = SDL_GetTicks64();
	ms_init_mouse();
	
	// Render loop

	while (get_quit_game_value() == false){
		
		Uint64 next_game_tick = SDL_GetTicks64();
		ms_clear_mouse_values();
		
		// Check for new events every frame
		SDL_Event event;
		
		while (SDL_PollEvent(&event)){
			
			switch (event.type){
				case SDL_MOUSEBUTTONDOWN:
					ms_mouse_button_handler(event);	
					break;
				case SDL_MOUSEBUTTONUP:
					ms_mouse_button_handler(event);	
					break;
				case SDL_MOUSEMOTION:
					ms_mouse_motion_handler(event);
					break;
				case SDL_MOUSEWHEEL:
					ms_mouse_wheel_handler(event);
					break;
				case SDL_QUIT:
					set_quit_game();
					break;
				case SDL_KEYDOWN:
					if (event.key.repeat == 0 && event.key.keysym.sym ==
					    SDLK_ESCAPE){
						toggle_escape_menu();
					}
					break;
				default:
					break;
			}
		}

		// I/O handling
		const Uint8 *keystate = SDL_GetKeyboardState(NULL);
		
		// Draw background
		SDL_RenderClear(g_renderer);
		// before color define as 0x000000FF
		Uint32 color = SDL_MapRGBA(format, 0, 0, 0, 0); // black
		SDL_FillRect(g_screen, NULL, color);

		switch (state){
			case LV_STUDIO_SCREEN:
				state = stage_studio(studio_screen_time, SDL_GetTicks64());
				break;
			case LV_TITLE_SCREEN:
				state = stage_title(keystate);
				break;
			case LV_SELECT_PLAYER_SCREEN:
				state = stage_select_player();
				break;
			case LV_LEVEL_SELECTION:
				stage = stage_select_level();
				if (stage != LV_SELECT_PLAYER_SCREEN &&
					stage != LV_LEVEL_SELECTION){
					state = LV_INITIALIZE_LEVEL;
					level = stage;
				} else {
					state = stage;
				}
				break;
			case LV_INITIALIZE_LEVEL:
				init_level(level);
				state = LV_PLAY_LEVEL;
				break;
			case LV_PLAY_LEVEL:
				state = stage_level(level);
				break;
		}

		SDL_UpdateTexture(g_screen_texture,
						  NULL,
						  g_screen->pixels,
						  g_screen->w * sizeof(Uint32));

		SDL_RenderCopy(g_renderer, 
					   g_screen_texture,
					   NULL, 
					   NULL);

		// draw to the display
		SDL_RenderPresent(g_renderer);
		
		// time it takes to render frame in milliseconds
		next_game_tick += 1000/60;
		sleep = next_game_tick - SDL_GetTicks64();
		if (sleep >= 0){
			SDL_Delay(sleep);
		}

	}

}

