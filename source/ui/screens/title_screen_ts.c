
#include <stdbool.h>
#include <SDL.h>
#include <SDL_mixer.h>
#include "dimensions_dm.h"
#include "aux.h"
#include "draw_dw.h"
#include "media/audio_au.h"
#include "levels_lv.h"
#include "draw_dw.h"
#include "electron_fx.h"
#include "sdl_config.h"
#include "text_tx.h"
#include "ui/screens/screen_common_sc.h"

static const char *PATH_LOGO = "img/oms.png";
static const char *PATH_AUDIO = "sound/intro_studio.wav";
static const char *PATH_CHIP = "img/chip.png";

char *GAME_TITLE = "ASSEMBLY ARCHITECT";
char *PRESS_SPACE = "PRESS SPACE";

static const Uint32 TYPE_DELAY_MS = 90; //repetida con la anterior
static const Uint32 CHIP_FADE_MS = 1000;   // ~1s fade

Mix_Chunk *g_studio_sfx = NULL;
texture_t *g_logo = NULL;

texture_t *g_chip = NULL;

static const Uint32 TITLE_IMG_H = 480;
static const Uint32 TITLE_IMG_W = 480;
static const Uint32 TITLE_IMG_Y = 325;

static const Uint32 FADE_MS = 1250;
static const Uint32 STUDIO_SCREEN_DELAY_MS = 2500; 


static SDL_Rect get_game_title_img_box();



/* Function: get_game_title_img_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object.
 */
static SDL_Rect get_game_title_img_box()
{
	int screen_width = dm_get_screen_width();
	SDL_Rect b;
	b.w = dm_scale_to_res(TITLE_IMG_W);
	b.h = dm_scale_to_res(TITLE_IMG_H);
	b.x = (screen_width - b.w)/2;
	b.y = dm_scale_to_res(TITLE_IMG_Y);
	return b;
}

/* Function: ts_stage_title
 * -----------------------------------------------------------------------------
 * This function renders the title screen of the game
 *
 * Arguments:
 *	None;
 *
 * Return:
 *	TITLE_SCREEN if user has not pressed spacebar, 
 *	LEVEL_SELECTION if otherwise
 */
int ts_stage_title(const Uint8 *keystate)
{
	int ret_val;
	const float blink_period_ms = 1500.0f;
	static bool init = false;
	static Uint64 start_time;
	static Uint64 chip_start_ms;
	static texture_t *press_space = NULL;
    
	Uint8 chip_alpha = 0;              
	static sc_fx_t fx_state = {0};
	Uint64 cur_time = SDL_GetTicks64();

	static sc_typewriter_t title = {0};

	if (init == false){
		init = true;
		press_space = dw_create_text_tex(PRESS_SPACE, C_SILVERGREY);
		start_time = cur_time;
		sc_typewriter_reset(&title);
		sc_fx_init(&fx_state, cur_time);
		SDL_SetTextureColorMod(g_chip->texture, 192, 192, 192);
		SDL_SetTextureAlphaMod(g_chip->texture, chip_alpha);
	}
	
	sc_fx_update_render(&fx_state, cur_time);
	
	bool title_done = sc_typewriter_update(&title, 
										 cur_time, 
										 TYPE_DELAY_MS, 
										 dm_get_game_title_box(GAME_TITLE), 
										 GAME_TITLE, 
										 C_SILVERGREY);
	if (title_done == true && chip_start_ms == 0) {
		chip_start_ms = cur_time;
		if (g_sfx_ready) Mix_PlayChannel(-1, g_sfx_ready, 0);
	}

	float chip_scale = 1.0f;
        if (title_done == true) {
            Uint64 elapsed = cur_time - chip_start_ms;
            if (elapsed < CHIP_FADE_MS) {
                // ease-in alpha
                float t = (float)elapsed / (float)CHIP_FADE_MS;
                if (t < 0.0f) t = 0.0f; if (t > 1.0f) t = 1.0f;
                chip_alpha = (Uint8)(t * 255.0f);
            } else {
                chip_alpha = 255;
            }
            // Pulse scale: ±2% over 2s
            float pulse = sinf((cur_time - chip_start_ms) * 
						  (2.0f * (float)M_PI / 2000.0f));
            chip_scale = 1.0f + 0.02f * pulse;
			SDL_SetTextureAlphaMod(g_chip->texture, chip_alpha);
        }

	if (title.texture != NULL){
		dw_draw_texture_fit_h(dm_get_game_title_box(GAME_TITLE), title.texture);
	}

	float blink_phase = fmodf((float)(cur_time - start_time), blink_period_ms) 
					   / blink_period_ms; // 0..1
    float blink_alpha_f = (blink_phase < 0.5f) ? (blink_phase * 2.0f)  
						: ((1.0f - blink_phase) * 2.0f);
    Uint8 press_alpha = (Uint8)(blink_alpha_f * 255.0f);
	SDL_SetTextureAlphaMod(press_space->texture, press_alpha);

	SDL_Rect img =  get_game_title_img_box();
	img = ax_scale_rect_proportion(img, chip_scale);
	dw_draw_texture_fit_h(img, g_chip);

	SDL_Rect s = dm_get_press_space_box(PRESS_SPACE);
	dw_draw_texture_fit_h(s, press_space);

	if (keystate[SDL_SCANCODE_SPACE]){
		sc_fx_destroy(&fx_state);
		init = false;
		sc_typewriter_free(&title);
		chip_start_ms = 0;
		ret_val = LV_SELECT_PLAYER_SCREEN;
	} else {
		ret_val = LV_TITLE_SCREEN;
	}
	return ret_val;
}

/* Function: ts_init_stage_title
 * ----------------------------------------------------------------------------
 * This function initializes the stage where the title of the game is shown.
 * Loads the chip image.
 *
 * Arguments:
 *	None.
 *
 * Return:
 *	Void.
 */
void ts_init_stage_title()
{
	g_chip = dw_load_texture(PATH_CHIP);
}

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
	g_studio_sfx = au_load_audio_file(PATH_AUDIO);
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




