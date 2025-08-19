#include <stdbool.h>
#include <assert.h>
#include "mouse_ms.h"
#include "sdl_config.h"
#include "aux.h"
#include "stages.h"
#include "levels_lv.h"
#include "file_fl.h"
#include "stage_buttons_sb.h"

#define BG_COLOR_BLACK 0x000000FF
#define BG_COLOR_RED 0xFF0000FF
#define BG_COLOR_GREEN 0x00FF00FF

int main(int argc, char *args[])
{
	//SDL Window setup
	if (init_sdl(SCREEN_WIDTH, SCREEN_HEIGHT, argc, args) == FAIL){
		printf("SDL could not be initialized");
		return FAIL;
	}
	SDL_PixelFormat *format = g_screen->format;
	SDL_GetWindowSize(g_window, &g_width, &g_height);
	
	load_media();
	fl_save_file_init();

	int sleep = 0;
	int state = STUDIO_SCREEN;
	int level = LV_NO_LEVEL;
	int stage = NO_STAGE;
	Uint64 next_game_tick = SDL_GetTicks64();
	Uint64 studio_screen_time = SDL_GetTicks64();
	ms_init_mouse();
	// Render loop

	while (get_quit_game_value() == false){
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
			case STUDIO_SCREEN:
				state = stage_studio(studio_screen_time, SDL_GetTicks64());
				break;
			case TITLE_SCREEN:
				state = stage_title(keystate);
				break;
			case SELECT_PLAYER_SCREEN:
				state = stage_select_player();
				break;
			case INIT_LEVEL_SELECTION:
				sb_initialize_return_button();
				state = LEVEL_SELECTION;
				break;
			case LEVEL_SELECTION:
				stage = stage_select_level();
				if (stage != SELECT_PLAYER_SCREEN &&
					stage != LEVEL_SELECTION){
					state = INITIALIZE_LEVEL;
					level = stage;
				} else {
					state = stage;
				}
				break;
			case INITIALIZE_LEVEL:
				level_initialization(level);
				state = PLAY_LEVEL;
				break;
			case PLAY_LEVEL:
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

