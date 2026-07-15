
#include <stdbool.h>
#include <SDL.h>
#include <SDL_mixer.h>
#include "dimensions_dm.h"
#include "aux.h"
#include "draw_dw.h"
#include "sdl_config.h"
#include "levels_lv.h"
#include "draw_dw.h"

static const char *PATH_LOGO = "img/oms.png";


texture_t *g_logo = NULL;

static const Uint32 FADE_MS = 1250;
static const Uint32 STUDIO_SCREEN_DELAY_MS = 2500; 

/* Function: ts_init_stage_studio
 * ----------------------------------------------------------------------------
 * This function initializes the stage where the name of the studio is shown.
 * Loads the logo and the sound effect of the studio.
 *
 * Arguments:
 *	None.
 *
 * Return:
 *	Void.
 */
void ts_init_stage_studio()
{
	g_logo = dw_load_texture(PATH_LOGO);

}
 

/* Function: stage_studio
 * ----------------------------------------------------------------------------
 * This function displays the stage where the name of the studio is shown.
 *
 * Arguments:
 *	start_time: starting time that will be used for the delay of the screen.
 *	cur_time: the current time used to calculate the delay of the screen.
 *
 * Return:
 *	TITLE_SCREEN if the delay time is already finished
 *	LEVEL_SELECTION if otherwise
 */
int ts_stage_studio(Uint64 start_time, Uint64 cur_time, bool key_pressed)
{
	int delay = cur_time - start_time;	
	SDL_Rect b = dm_get_studio_name_msg_box();
	int w = ax_get_texture_w_fit_h(b.h, g_logo);
	b.x = (dm_get_screen_width() - w)/2;
	b.w = w;
	Uint8 alpha = 255;
	static bool sfx_played = false;

	if (sfx_played == false) {
		Mix_PlayChannel(-1, g_studio_sfx, 0);
		sfx_played = true;
	}
	
	if (delay <= FADE_MS){
		float k = (float)delay / (float)FADE_MS;
		alpha = (Uint8)(k * 255.0f);
		SDL_SetTextureAlphaMod(g_logo->texture, alpha);
	} else if (delay > FADE_MS){
		float k = 1.0f - (float)(delay - FADE_MS) / (float)FADE_MS;
		alpha = (Uint8)(k * 255.0f);
		SDL_SetTextureAlphaMod(g_logo->texture, alpha);
	}

	SDL_SetTextureColorMod(g_logo->texture, 192, 192, 192);
	dw_draw_texture_fit_h(b, g_logo);
	
	if (delay > STUDIO_SCREEN_DELAY_MS || key_pressed == true){
		dw_free_texture(g_logo);
		return LV_TITLE_SCREEN;
	} else {
		return LV_STUDIO_SCREEN;
	}
}
