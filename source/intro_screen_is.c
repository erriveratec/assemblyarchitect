#include <assert.h>
#include <stdbool.h>
#include <SDL.h>
#include <SDL_mixer.h>
#include "dimensions_dm.h"
#include "levels_lv.h"
#include "aux.h"
#include "stage_buttons_sb.h"
#include "dbg.h"
#include "file_fl.h"
#include "stages.h"
#include "electron_fx.h"
#include "sdl_config.h"
#include"text_tx.h"

const char *STUDIO = "One Man Studio";
char *GAME_TITLE = "ASSEMBLY ARCHITECT";
char *PRESS_SPACE = "PRESS SPACE";
char *SELECT_PLAYER_TEXT = "Which hacker are you?";
char *SELECT_LEVEL_TEXT = "Select Level";
char *PLAYER_1_TEXT = "HACKER W";
char *PLAYER_2_TEXT = "HACKER X";
char *PLAYER_3_TEXT = "HACKER Y";


static const Uint32 STUDIO_SCREEN_DELAY_MS = 2500; 
static const Uint32 FADE_MS = 1250;
static const Uint32 TYPE_DELAY_MS = 90;  
static const Uint32 CHIP_FADE_MS = 1000;   // ~1s fade

texture_t *g_press_space = NULL;
texture_t *g_chip = NULL;
texture_t *g_logo = NULL;
texture_t *g_title_background = NULL;

static void create_select_level_buttons(iface_btn_t **buttons, bool *levels);

/* Function: create_select_level_buttons
 * -----------------------------------------------------------------------------
 * Creates the total of buttons according to the numbers of levels that will be
 * present in the game.
 * 	
 * Arguments:
 * 	buttons_array: an array of buttons that will be created.
 *  player_levels: the list of levels that the player will have available
 *
 * Return:
 *	void
 */
static void create_select_level_buttons(iface_btn_t **buttons, bool *levels)
{
	assert(buttons != NULL && "The buttons pointer is NULL");
	assert(levels != NULL && "The levels pointer is NULL");
	
	SDL_Rect r = dm_get_level_button_box();
	SDL_Rect b = r;
	int y_offset = get_sel_level_offset_y();
	for (int i = 1; i <= LV_LEVEL_QUANTITY; i++){
		char *btn_text = create_string_append_number(ax_level_text, i);
		texture_t *btn_texture = NULL;
		if (levels[i-1] == true){
			btn_texture = dw_create_text_texture(btn_text, 
							 C_WHITE);
			buttons[i-1] = bt_create_iface_btn(b, btn_texture, true);
		} else {
			btn_texture = dw_create_text_texture(btn_text, 
							 C_GREY);
			buttons[i-1] = bt_create_iface_btn(b, btn_texture, false);
		}
		b.y += y_offset;	
		if (i != 0 && i%8 == 0){
			b.x += r.x + r.w;
			b.y = r.y;
		}
	}
}



/* Function: stage_select_level
 * -----------------------------------------------------------------------------
 * Displays the levels that a player has available according to its save file
 * 	
 * Arguments:
 * 	None.
 *
 * Return:
 *	The number of the level selected, if not valid press was performed
 *	the number of the select label stage will be returned.
 */
int stage_select_level()
{
	static texture_array_t *select_level = NULL;

	static bool level_initialized = false;
	static iface_btn_t *level_buttons[40];
	static bool player_levels[LV_LEVEL_QUANTITY];
	int ret_val = LV_LEVEL_SELECTION;
	SDL_Rect r = dm_get_upper_title_box();

	if (level_initialized == false){
		fl_load_player_levels(g_player, player_levels);
		level_initialized = true;
		create_select_level_buttons(level_buttons, player_levels);
		select_level = dw_new_text_texture_by_h(r.w, r.h, C_SILVERGREY, 
															 SELECT_LEVEL_TEXT);
	}
	dw_draw_wrapped_texture_by_h(r, r.h, select_level);
	sb_draw_return_button();
	for (int i = 0; i < LV_LEVEL_QUANTITY; i++){
			bt_draw_iface_btn(level_buttons[i]);
	}
	
	bool escape = sb_get_escape_menu_state();
	if (escape == true){
		sb_display_escape_menu(escape);
	} else {
		for (int i = 0; i < LV_LEVEL_QUANTITY; i++){
			if (bt_chk_mouse_rel_iface_btn(level_buttons[i]) == true){
				ret_val = LV_LEVEL_1 + i;
				level_initialized = false;
				for (int i = 0; i < LV_LEVEL_QUANTITY; i++){
					bt_destroy_iface_btn(level_buttons[i]);
				}
			}
		}
		if (sb_check_clicked_ret_button() == true){
			ret_val = LV_SELECT_PLAYER_SCREEN;	
			level_initialized = false;
			for (int i = 0; i < LV_LEVEL_QUANTITY; i++){
				bt_destroy_iface_btn(level_buttons[i]);
			}
		}
	}
	return ret_val;
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
int stage_studio(Uint64 start_time, Uint64 cur_time, bool key_pressed)
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
	dw_draw_texture_fits_height(b, g_logo);
	
	if (delay > STUDIO_SCREEN_DELAY_MS || key_pressed == true){
		dw_free_texture(g_logo);
		return LV_TITLE_SCREEN;
	} else {
		return LV_STUDIO_SCREEN;
	}
}

/* Function: stage_title
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
int stage_title(const Uint8 *keystate)
{
	int w = dm_get_screen_width();
	int h = dm_get_screen_height();
	SDL_Rect screen = {0, 0, w, h};
	dw_draw_texture_fits_width(screen, g_title_background);
 	
	int ret_val;
	const float blink_period_ms = 1500.0f;
	static bool init = false;
	static Uint64 start_time;
	static Uint64 last_type_ms;
	static Uint64 chip_start_ms;
	static Uint64 anim_prev_ms;
	static size_t type_index = 0;
	static bool title_done = false;
    Uint8 chip_alpha = 0;              
	
	int W = dm_get_screen_width();
	int H = dm_get_screen_height();   
  	static fx_electron_t* fx;
	
	Uint64 cur_time = SDL_GetTicks64();
	if (init == false){
		last_type_ms = cur_time;
		anim_prev_ms = cur_time;
		start_time = cur_time;
  		fx = fx_electron_create(g_renderer, W, H, NULL);
		SDL_SetTextureColorMod(g_chip->texture, 192, 192, 192);
		SDL_SetTextureAlphaMod(g_chip->texture, chip_alpha);
		init = true;
	}
	
	float dt=(cur_time - anim_prev_ms)/1000.0f;
	anim_prev_ms = cur_time;
    fx_electron_update(fx, dt);
    fx_electron_render(fx, g_renderer);

	static texture_t *game_title = NULL;
	SDL_Rect t = dm_get_game_title_box(GAME_TITLE);
	size_t full_length = strlen(GAME_TITLE);

	if (title_done == false && (cur_time - last_type_ms) >= TYPE_DELAY_MS){
		last_type_ms = cur_time;
		
		bool write_complete = tx_draw_create_typewriter_text(&game_title, 
															 t, 
															 GAME_TITLE, 
															 &type_index);
		if (write_complete == true){
			title_done = true;
			chip_start_ms = cur_time;
			if (g_sfx_ready) Mix_PlayChannel(-1, g_sfx_ready, 0);
        }
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
	
	if (game_title != NULL){
		dw_draw_texture_fits_height(t, game_title);
	}
	
	float blink_phase = fmodf((float)(cur_time - start_time), blink_period_ms) 
					   / blink_period_ms; // 0..1
    float blink_alpha_f = (blink_phase < 0.5f) ? (blink_phase * 2.0f)  
						: ((1.0f - blink_phase) * 2.0f);
    Uint8 press_alpha = (Uint8)(blink_alpha_f * 255.0f);
	SDL_SetTextureAlphaMod(g_press_space->texture, press_alpha);

	
	SDL_Rect img =  dm_get_game_title_img_box();
	img = ax_scale_rect(img, chip_scale);
	dw_draw_texture_fits_height(img, g_chip);

	SDL_Rect s = dm_get_press_space_box(PRESS_SPACE);
	dw_draw_texture_fits_height(s, g_press_space);

	if (keystate[SDL_SCANCODE_SPACE]){
		dw_free_texture(g_chip);
		dw_free_texture(g_title_background);
		dw_free_texture(g_press_space);
		dw_free_texture(game_title);
  		aa_electron_fx_destroy(fx);
		init = false;
		ret_val = LV_SELECT_PLAYER_SCREEN;
	} else {
		ret_val = LV_TITLE_SCREEN;
	}

	sb_display_escape_menu(sb_get_escape_menu_state());
	return ret_val;
}


/* Function: stage_select_player
 * ----------------------------------------------------------------------------
 * This function displays the screen where the player is selected
 *
 * Arguments:
 *	None.
 *
 * Return:
 *	SELECT_PLAYER_SCREEN if the user have not selected a player. 
 *	LEVEL_SELECTION if a player has been chosen
 */
int stage_select_player()
{
	static texture_array_t *select_player = NULL;
	static bool init = false;
	static iface_btn_t *player_1;
	static iface_btn_t *player_2;
	static iface_btn_t *player_3;
	int ret_val = LV_SELECT_PLAYER_SCREEN;
	bool player_chosen = false;

	int W = dm_get_screen_width();
	int H = dm_get_screen_height();   
  	static fx_electron_t* fx;
	static Uint64 last_type_ms;
	static Uint64 anim_prev_ms;
	
	SDL_Rect b = dm_get_upper_title_box();
	int text_h = dm_get_h_stage_titles();
	
	 Uint64 cur_time = SDL_GetTicks64();

	if (init == false){
	
		init = true;
  		last_type_ms = cur_time;
		anim_prev_ms = cur_time;
		fx = fx_electron_create(g_renderer, W, H, NULL);
		
		select_player = dw_new_text_texture_by_h(b.w, 
												 text_h, 
												 C_SILVERGREY, 
												 SELECT_PLAYER_TEXT);
		texture_t *b1_texture = dw_create_text_texture(PLAYER_1_TEXT, C_WHITE);
		check_mem(b1_texture);
		texture_t *b2_texture = dw_create_text_texture(PLAYER_2_TEXT, C_WHITE);
		check_mem(b2_texture);
		texture_t *b3_texture = dw_create_text_texture(PLAYER_3_TEXT, C_WHITE);
		check_mem(b3_texture);
		
		SDL_Rect b = dm_get_p1_button_box();
		player_1 = bt_create_iface_btn(b, b1_texture, true);
		check_mem(player_1);
		
		b = dm_get_p2_button_box();
		player_2 = bt_create_iface_btn(b, b2_texture, true);
		check_mem(player_2);

		b = dm_get_p3_button_box();
		player_3 = bt_create_iface_btn(b, b3_texture, true);
		check_mem(player_3);
	}
	
	float dt=(cur_time - anim_prev_ms)/1000.0f;
	anim_prev_ms = cur_time;
	fx_electron_update(fx, dt);
    fx_electron_render(fx, g_renderer);

	dw_draw_wrapped_texture_by_h(b, text_h, select_player);
	
	bt_draw_iface_btn(player_1);
	bt_draw_iface_btn(player_2);
	bt_draw_iface_btn(player_3);


	bool escape_menu = sb_get_escape_menu_state();
	if (escape_menu == true){
		sb_display_escape_menu(escape_menu);
	} else {
		if (bt_chk_mouse_rel_iface_btn(player_1) == true){
			player_chosen = true;
			g_player = FL_PLAYER_1;
		} else if (bt_chk_mouse_rel_iface_btn(player_2) == true){
			player_chosen = true;
			g_player = FL_PLAYER_2;
		} else if (bt_chk_mouse_rel_iface_btn(player_3) == true){
			player_chosen = true;
			g_player = FL_PLAYER_3;
		}
		if (player_chosen == true){
			ret_val = LV_LEVEL_SELECTION;		
			bt_destroy_iface_btn(player_1);
			bt_destroy_iface_btn(player_2);
			bt_destroy_iface_btn(player_3);
			init = false;
		}
	}
	error:
	return ret_val;
}
