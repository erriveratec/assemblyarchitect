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
#include"players_pl.h"

const char *STUDIO = "One Man Studio";
char *GAME_TITLE = "ASSEMBLY ARCHITECT";
char *PRESS_SPACE = "PRESS SPACE";
char *SELECT_PLAYER_TEXT = "CHOOSE YOUR ARCHITECT";
char *SELECT_LEVEL_TEXT = "SELECT LEVEL";
char *PLAYER_1_TEXT = "EXECUTOR X";
char *PLAYER_2_TEXT = "OPERATIVE Y";
char *PLAYER_3_TEXT = "HANDLER Z";
char *P1_LORETEXT = "Cross-Branch Executor";
char *P2_LORETEXT = "Yield-Loop Operative";
char *P3_LORETEXT = "Zero-Flag Handler";
char *SELECT_SECTOR = "SELECT SECTOR";
char *SECTOR_0 = "[00] Architecture Primer";
char *SECTOR_1 = "[01] Computation Core";
char *SECTOR_2 = "[02] Jump Flow Unit";
char *SECTOR_3 = "[03] Branching Engine";
char *SECTOR_4 = "[04] Advanced Module";
char *SECTOR_0_TITLE = "SECTOR 0";

static const Uint32 STUDIO_SCREEN_DELAY_MS = 2500; 
static const Uint32 FADE_MS = 1250;
static const Uint32 TYPE_DELAY_MS = 90;  
static const Uint32 CHIP_FADE_MS = 1000;   // ~1s fade

static const Uint32 PLAYER_BLOCK_W = 8;

static const Uint32 P_BUTTON_H = 280;
static const Uint32 P_BUTTON_W = 280;
static const Uint32 P_BUTTON_Y = 405;

static const Uint32 TITLE_IMG_H = 480;
static const Uint32 TITLE_IMG_W = 480;
static const Uint32 TITLE_IMG_Y = 325;

static const Uint32 SEL_LEVEL_BUTTON_W = 150;
static const Uint32 SEL_LEVEL_BUTTON_H = 60;

static const Uint32 BIG_SEPARATOR_H = 6;
static const Uint32 SMALL_SEPARATOR_H = 2;
static const Uint32 SECTOR_TITLE_SEPARATOR = 24;

static const Uint32 SECTOR_BTN_SPACING = 60;
static const Uint32 FIRST_BTN_OFS = 10;

texture_t *g_chip = NULL;
texture_t *g_logo = NULL;

static void create_select_level_buttons(iface_btn_t **buttons, bool *levels);
static void create_player_btns(iface_btn_t **player_btns);
static void draw_player_texts(texture_t **player_text, texture_t **lore);

static int get_player_block_size();
static SDL_Rect get_p1_button_box();
static SDL_Rect get_p2_button_box();
static SDL_Rect get_p3_button_box();
static SDL_Rect get_game_title_img_box();
static SDL_Rect get_level_button_box();
static int get_sel_level_offset_y();
static SDL_Rect get_upper_separator();
static void create_sector_btns(btn_t **btns);
static int get_sector_btn_spacing();
static SDL_Rect get_sector_separator();




/* Function: get_sector_btn_spacing
 * -----------------------------------------------------------------------------
 * Returns the spacing define for each of the buttons of the sector selection
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the ammount of spacing
 */
static int get_sector_btn_spacing()
{
	return dm_scale_to_resolution(SECTOR_BTN_SPACING);
}

/* Function: get_sector_btn_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the upper separator o select sector
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the objecto
 */
static SDL_Rect get_sector_btn_box()
{
	int screen_width = dm_get_screen_width();
	int screen_height = dm_get_screen_height();

	SDL_Rect b;
	b.w = screen_width*2/3;
	b.h = dm_get_h_stage_subtitle();
	b.x = dm_get_upper_title_box(SELECT_SECTOR).x;	
	b.y = get_upper_separator().y + get_sector_btn_spacing() 
		  + dm_scale_to_resolution(FIRST_BTN_OFS); 
	return b;
}

/* Function: get_upper_separator
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the sector separator
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the objecto
 */
static SDL_Rect get_sector_separator()
{
	int screen_width = dm_get_screen_width();
	int screen_height = dm_get_screen_height();

	SDL_Rect r = get_sector_btn_box();	
	SDL_Rect b;
	b.w = dm_get_upper_title_box(SELECT_SECTOR).w;
	b.h = dm_scale_to_resolution(SMALL_SEPARATOR_H);
	b.x = dm_get_upper_title_box(SELECT_SECTOR).x;	
	b.y = r.y + r.h + get_sector_btn_spacing()/2;
	
	return b;
}

/* Function: get_upper_separator
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the upper separator o select sector
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the objecto
 */
static SDL_Rect get_upper_separator()
{
	int screen_width = dm_get_screen_width();
	int screen_height = dm_get_screen_height();

	SDL_Rect r = dm_get_upper_title_box(SELECT_LEVEL_TEXT);
	SDL_Rect b;
	b.w = screen_width*3/4;
	b.h = dm_scale_to_resolution(BIG_SEPARATOR_H);
	b.x = screen_width/8;
	b.y = r.y + r.h + dm_scale_to_resolution(SECTOR_TITLE_SEPARATOR); 
	return b;
}

/* Function: dm_get_sel_level_offset_y
 * -----------------------------------------------------------------------------
 * Returns the offset for the sel level buttons
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset for the sel level buttons
 */
static int get_sel_level_offset_y()
{
	int y_offset =  dm_scale_to_resolution(SEL_LEVEL_BUTTON_H + 
										0.5*SEL_LEVEL_BUTTON_H);
	return y_offset;
	
}

/* Function: get_level_button_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
static SDL_Rect get_level_button_box()
{
	int screen_width = dm_get_screen_width();
	int screen_height = dm_get_screen_height();

	SDL_Rect r = dm_get_upper_title_box(SELECT_LEVEL_TEXT);
	SDL_Rect b;
	b.w = dm_scale_to_resolution(SEL_LEVEL_BUTTON_W);
	b.h = dm_scale_to_resolution(SEL_LEVEL_BUTTON_H);
	b.x = (screen_width - (5*b.w + 4*b.w/2))/2;
	b.y = r.y + r.h + b.h;// + get_sel_level_offset_y(); 
	return b;
}

/* Function: get_block_size
 * -----------------------------------------------------------------------------
 * Returns the size of the block used for the figures
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	Block size of the figures
 */
static int get_player_block_size()
{
	return dm_scale_to_resolution(PLAYER_BLOCK_W);
}

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
	b.w = dm_scale_to_resolution(TITLE_IMG_W);
	b.h = dm_scale_to_resolution(TITLE_IMG_H);
	b.x = (screen_width - b.w)/2;
	b.y = dm_scale_to_resolution(TITLE_IMG_Y);
	return b;
}

/* Function: get_p1_button_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
static SDL_Rect get_p1_button_box()
{
	int screen_width = dm_get_screen_width();

	SDL_Rect b;
	b.w = dm_scale_to_resolution(P_BUTTON_W);
	b.h = dm_scale_to_resolution(P_BUTTON_H);
	b.x = (screen_width - 3*b.w)/4;
	b.y = dm_scale_to_resolution(P_BUTTON_Y);
	return b;
}

/* Function: get_p2_button_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
static SDL_Rect get_p2_button_box()
{
	int screen_width = dm_get_screen_width();
	SDL_Rect b;
	b.w = dm_scale_to_resolution(P_BUTTON_W);
	b.h = dm_scale_to_resolution(P_BUTTON_H);
	b.x = 2*(screen_width - 3*b.w)/4 + b.w;
	b.y = dm_scale_to_resolution(P_BUTTON_Y);
	return b;
}

/* Function: get_p3_button_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect get_p3_button_box()
{
	int screen_width = dm_get_screen_width();
	SDL_Rect b;
	b.w = dm_scale_to_resolution(P_BUTTON_W);
	b.h = dm_scale_to_resolution(P_BUTTON_H);
	b.x = 3*(screen_width - 3*b.w)/4 + 2*b.w;
	b.y = dm_scale_to_resolution(P_BUTTON_Y);
	return b;
}


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
	
	SDL_Rect r = get_level_button_box();
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
	//		b.x += r.x + r.w;
			b.x += r.w + r.w/2;
			b.y = r.y;
		}
	}
}

/* Function: create_sector_btns
 * -----------------------------------------------------------------------------
 * Creates the corresponding sector buttons
 * 	
 * Arguments:
 * 	Pointer of pointer of the buttons
 *
 * Return:
 *	Void.
 */
static void create_sector_btns(btn_t **btns)
{
	assert(btns != NULL && "The buttons pointer is NULL");

	int y_space = get_sector_btn_spacing();

	btns[0] = bt_create_btn(get_sector_btn_box(), 
			  				dw_create_text_texture(SECTOR_0, C_WHITE));
	btns[0]->r.w = ax_get_texture_w_fit_h(btns[0]->r.h, btns[0]->t);
	int x_ofs = (dm_get_upper_title_box(SELECT_SECTOR).w - btns[0]->r.w)/2;
	btns[0]->r.x += x_ofs;
	
	btns[1] = bt_create_btn(get_sector_btn_box(), 
			  				dw_create_text_texture(SECTOR_1, C_WHITE));
	btns[1]->r.w = ax_get_texture_w_fit_h(btns[1]->r.h, btns[1]->t);
	btns[1]->r.y = btns[0]->r.y + get_sector_btn_box().h + y_space;
	btns[1]->r.x += x_ofs;
	
	btns[2] = bt_create_btn(get_sector_btn_box(), 
			  				dw_create_text_texture(SECTOR_2, C_WHITE));
	btns[2]->r.w = ax_get_texture_w_fit_h(btns[2]->r.h, btns[2]->t);
	btns[2]->r.y = btns[1]->r.y + get_sector_btn_box().h + y_space;
	btns[2]->r.x += x_ofs;

	btns[3] = bt_create_btn(get_sector_btn_box(), 
			  				dw_create_text_texture(SECTOR_3, C_WHITE));
	btns[3]->r.w = ax_get_texture_w_fit_h(btns[3]->r.h, btns[3]->t);
	btns[3]->r.y = btns[2]->r.y + get_sector_btn_box().h + y_space;
	btns[3]->r.x += x_ofs;

	btns[4] = bt_create_btn(get_sector_btn_box(), 
			  				dw_create_text_texture(SECTOR_4, C_WHITE));
	btns[4]->r.w = ax_get_texture_w_fit_h(btns[4]->r.h, btns[4]->t);
	btns[4]->r.y = btns[3]->r.y + get_sector_btn_box().h + y_space;
	btns[4]->r.x += x_ofs;

}


/* Function: stage_sector_0
 * -----------------------------------------------------------------------------
 * Displays the sections available for the player to choose
 * 	
 * Arguments:
 * 	None.
 *
 * Return:
 *	The number of the level selected, if not valid press was performed
 *	the number of the select label stage will be returned.
 */
int stage_sector_0()
{
	int W = dm_get_screen_width();
	int H = dm_get_screen_height();   
  	static fx_electron_t* fx;
	static Uint64 last_type_ms;
	static Uint64 anim_prev_ms;
	static size_t type_index = 0;
	static bool title_done = false;
	Uint64 cur_time = SDL_GetTicks64();

	static texture_t *sector_0 = NULL;

	static bool level_initialized = false;
	static bool player_levels[LV_LEVEL_QUANTITY];
	static btn_t *sectors[5];	

	int ret_val = LV_SECTOR_0;
	SDL_Rect r = dm_get_upper_title_box(SECTOR_0_TITLE);

	if (level_initialized == false){
		//fl_load_player_levels(g_player, player_levels);
		level_initialized = true;
		last_type_ms = cur_time;
		fx = fx_electron_create(g_renderer, W, H, NULL);
		create_sector_btns(sectors);
	}
//	int dm_get_h_stage_subtitle()
	float dt=(cur_time - anim_prev_ms)/1000.0f;
	anim_prev_ms = cur_time;
	SDL_Rect b = dm_get_upper_title_box(SECTOR_0_TITLE);
	fx_electron_update(fx, dt);
    fx_electron_render(fx, g_renderer);
	
	if (title_done == false && (cur_time - last_type_ms) >= TYPE_DELAY_MS){
		last_type_ms = cur_time;
		
		bool write_complete = tx_draw_create_typewriter_text(&sector_0, 
															 b, 
															 SECTOR_0_TITLE, 
															 &type_index,
															 C_SILVERGREY);
		if (write_complete == true){
			title_done = true;
        }
	}
	if (sector_0 != NULL){
		dw_draw_texture_fit_h(r, sector_0);
	}
	
	//dw_draw_filled_rectangle(get_upper_separator(), C_SHADOWGREY, C_SHADOWGREY);
	sb_draw_return_button();
	
	SDL_Rect sep = get_sector_separator();
	for (int i = 0; i < 5; i++){
//		bt_draw_btn(sectors[i], sb_get_escape_state());
//		dw_draw_filled_rectangle(sep, C_SHADOWGREY, C_SHADOWGREY);
//		sep.y += get_sector_btn_box().h + get_sector_btn_spacing();
	}
	
	bool escape = sb_get_escape_state();
	if (escape == true){
		sb_display_escape_menu(escape);
	} else if (bt_chk_rel_btn(sectors[0], g_sfx_select)){
	} else if (bt_chk_rel_btn(sectors[1], g_sfx_select)){
	} else if (bt_chk_rel_btn(sectors[2], g_sfx_select)){
	} else if (bt_chk_rel_btn(sectors[3], g_sfx_select)){
	} else if (bt_chk_rel_btn(sectors[4], g_sfx_select)){
	} else if (sb_check_clicked_ret_button() == true){
			bt_destroy_button(sectors[0]);
			bt_destroy_button(sectors[1]);
			bt_destroy_button(sectors[2]);
			bt_destroy_button(sectors[3]);
			bt_destroy_button(sectors[4]);
			ret_val = LV_SELECT_SECTOR;	
			level_initialized = false;
  			aa_electron_fx_destroy(fx);
	}
	return ret_val;
}
/* Function: stage_select_sector
 * -----------------------------------------------------------------------------
 * Displays the sections available for the player to choose
 * 	
 * Arguments:
 * 	None.
 *
 * Return:
 *	The number of the level selected, if not valid press was performed
 *	the number of the select label stage will be returned.
 */
int stage_select_sector()
{
	int W = dm_get_screen_width();
	int H = dm_get_screen_height();   
  	static fx_electron_t* fx;
	static Uint64 last_type_ms;
	static Uint64 anim_prev_ms;
	static size_t type_index = 0;
	static bool title_done = false;
	Uint64 cur_time = SDL_GetTicks64();

	static texture_t *select_sector = NULL;

	static bool level_initialized = false;
	static bool player_levels[LV_LEVEL_QUANTITY];
	static btn_t *sectors[5];	

	int ret_val = LV_SELECT_SECTOR;
	SDL_Rect r = dm_get_upper_title_box(SELECT_SECTOR);

	if (level_initialized == false){
		fl_load_player_levels(g_player, player_levels);
		level_initialized = true;
		last_type_ms = cur_time;
		fx = fx_electron_create(g_renderer, W, H, NULL);
		create_sector_btns(sectors);
	}

	float dt=(cur_time - anim_prev_ms)/1000.0f;
	anim_prev_ms = cur_time;
	SDL_Rect b = dm_get_upper_title_box(SELECT_SECTOR);
	fx_electron_update(fx, dt);
    fx_electron_render(fx, g_renderer);
	
	if (title_done == false && (cur_time - last_type_ms) >= TYPE_DELAY_MS){
		last_type_ms = cur_time;
		
		bool write_complete = tx_draw_create_typewriter_text(&select_sector, 
															 b, 
															 SELECT_SECTOR, 
															 &type_index,
															 C_SILVERGREY);
		if (write_complete == true){
			title_done = true;
        }
	}
	if (select_sector != NULL){
		dw_draw_texture_fit_h(r, select_sector);
	}
	
	dw_draw_filled_rectangle(get_upper_separator(), C_SHADOWGREY, C_SHADOWGREY);
	sb_draw_return_button();
	
	SDL_Rect sep = get_sector_separator();
	for (int i = 0; i < 5; i++){
		bt_draw_btn(sectors[i], sb_get_escape_state());
		dw_draw_filled_rectangle(sep, C_SHADOWGREY, C_SHADOWGREY);
		sep.y += get_sector_btn_box().h + get_sector_btn_spacing();
	}
	
	bool escape = sb_get_escape_state();
	if (escape == true){
		sb_display_escape_menu(escape);
	} else if (bt_chk_rel_btn(sectors[0], g_sfx_select)){
		ret_val = LV_SECTOR_0;
	} else if (bt_chk_rel_btn(sectors[1], g_sfx_select)){
	} else if (bt_chk_rel_btn(sectors[2], g_sfx_select)){
	} else if (bt_chk_rel_btn(sectors[3], g_sfx_select)){
	} else if (bt_chk_rel_btn(sectors[4], g_sfx_select)){
	} else if (sb_check_clicked_ret_button() == true){
			bt_destroy_button(sectors[0]);
			bt_destroy_button(sectors[1]);
			bt_destroy_button(sectors[2]);
			bt_destroy_button(sectors[3]);
			bt_destroy_button(sectors[4]);
			ret_val = LV_SELECT_PLAYER_SCREEN;	
			level_initialized = false;
  			aa_electron_fx_destroy(fx);
	}
	return ret_val;
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
	int W = dm_get_screen_width();
	int H = dm_get_screen_height();   
  	static fx_electron_t* fx;
	static Uint64 last_type_ms;
	static Uint64 anim_prev_ms;
	static size_t type_index = 0;
	static bool title_done = false;
	Uint64 cur_time = SDL_GetTicks64();

	static texture_t *select_level = NULL;

	static bool level_initialized = false;
	static iface_btn_t *level_buttons[40];
	static bool player_levels[LV_LEVEL_QUANTITY];
	int ret_val = LV_LEVEL_SELECTION;
	SDL_Rect r = dm_get_upper_title_box(SELECT_LEVEL_TEXT);

	if (level_initialized == false){
		fl_load_player_levels(g_player, player_levels);
		level_initialized = true;
		create_select_level_buttons(level_buttons, player_levels);
		last_type_ms = cur_time;
		fx = fx_electron_create(g_renderer, W, H, NULL);
	}

	float dt=(cur_time - anim_prev_ms)/1000.0f;
	anim_prev_ms = cur_time;
	SDL_Rect b = dm_get_upper_title_box(SELECT_LEVEL_TEXT);
	fx_electron_update(fx, dt);
    fx_electron_render(fx, g_renderer);
	
	if (title_done == false && (cur_time - last_type_ms) >= TYPE_DELAY_MS){
		last_type_ms = cur_time;
		
		bool write_complete = tx_draw_create_typewriter_text(&select_level, 
															 b, 
															 SELECT_LEVEL_TEXT, 
															 &type_index,
															 C_SILVERGREY);
		if (write_complete == true){
			title_done = true;
        }
	}
	if (select_level != NULL){
		dw_draw_texture_fit_h(r, select_level);
	}
	
	sb_draw_return_button();
	for (int i = 0; i < LV_LEVEL_QUANTITY; i++){
			bt_draw_iface_btn(level_buttons[i], sb_get_escape_state(), NULL);
	}
	
	bool escape = sb_get_escape_state();
	if (escape == true){
		sb_display_escape_menu(escape);
	} else {
		for (int i = 0; i < LV_LEVEL_QUANTITY; i++){
			if (bt_chk_rel_iface_btn(level_buttons[i], NULL) == true){
				ret_val = LV_LEVEL_1 + i;
				level_initialized = false;
  				aa_electron_fx_destroy(fx);
				for (int i = 0; i < LV_LEVEL_QUANTITY; i++){
					bt_destroy_iface_btn(level_buttons[i]);
				}
			}
		}
		if (sb_check_clicked_ret_button() == true){
			ret_val = LV_SELECT_PLAYER_SCREEN;	
			level_initialized = false;
  			aa_electron_fx_destroy(fx);
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
	dw_draw_texture_fit_h(b, g_logo);
	
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
	int W = dm_get_screen_width();
	int H = dm_get_screen_height();
 	
	int ret_val;
	const float blink_period_ms = 1500.0f;
	static bool init = false;
	static Uint64 start_time;
	static Uint64 last_type_ms;
	static Uint64 chip_start_ms;
	static Uint64 anim_prev_ms;
	static size_t type_index = 0;
	static bool title_done = false;
	static texture_t *press_space = NULL;
    
	Uint8 chip_alpha = 0;              
	
  	static fx_electron_t* fx;
	Uint64 cur_time = SDL_GetTicks64();

	if (init == false){
		init = true;
		press_space = dw_create_text_texture(PRESS_SPACE, C_SILVERGREY);
		last_type_ms = cur_time;
		anim_prev_ms = cur_time;
		start_time = cur_time;
  		fx = fx_electron_create(g_renderer, W, H, NULL);
		SDL_SetTextureColorMod(g_chip->texture, 192, 192, 192);
		SDL_SetTextureAlphaMod(g_chip->texture, chip_alpha);
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
															 &type_index, 
															 C_SILVERGREY);
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
		dw_draw_texture_fit_h(t, game_title);
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
		dw_free_texture(g_chip);
		dw_free_texture(press_space);
		dw_free_texture(game_title);
  		aa_electron_fx_destroy(fx);
		init = false;
		ret_val = LV_SELECT_PLAYER_SCREEN;
	} else {
		ret_val = LV_TITLE_SCREEN;
	}

	sb_display_escape_menu(sb_get_escape_state());
	return ret_val;
}


/* Function: create_player_btns
 * -----------------------------------------------------------------------------
 * Creates the player buttons of the level
 * 	
 * Arguments:
 * 	buttons_array: an array of buttons that will be created.
 *  player_levels: the list of levels that the player will have available
 *
 * Return:
 *	void
 */
static void create_player_btns(iface_btn_t **player_btns)
{
	assert(player_btns != NULL && "The buttons pointer is NULL");
	SDL_Rect p1_box = get_p1_button_box();
	SDL_Rect p2_box = get_p2_button_box();
	SDL_Rect p3_box = get_p3_button_box();
	
	player_btns[0] = bt_create_iface_btn(p1_box, 
								   dw_create_text_texture(" ", C_BLACK), 
								   true);
	
	player_btns[1] = bt_create_iface_btn(p2_box, 
								   dw_create_text_texture(" ", C_BLACK), 
								   true);

	player_btns[2] = bt_create_iface_btn(p3_box, 
								   dw_create_text_texture(" ", C_BLACK),
								   true);
}

/* Function: create_player_texts
 * -----------------------------------------------------------------------------
 * Creates the player buttons of the level
 * 	
 * Arguments:
 * 	player_text: The texts with the name of the player
 *  lore: The lore of each player
 *
 * Return:
 *	void
 */
static void create_player_texts(texture_t **player_text, texture_t **lore)
{
	assert(player_text != NULL && "The texture is NULL");
	assert(lore != NULL && "The texture is NULL");
	
	player_text[0] = dw_create_text_texture(PLAYER_1_TEXT, C_SILVERGREY);
	player_text[1] = dw_create_text_texture(PLAYER_2_TEXT, C_SILVERGREY);
	player_text[2] = dw_create_text_texture(PLAYER_3_TEXT, C_SILVERGREY);

	lore[0] = dw_create_text_texture(P1_LORETEXT, C_GREY);
	lore[1] = dw_create_text_texture(P2_LORETEXT, C_GREY);
	lore[2] = dw_create_text_texture(P3_LORETEXT, C_GREY);
}

/* Function: draw_player_texts
 * -----------------------------------------------------------------------------
 * Creates the player buttons of the level
 * 	
 * Arguments:
 * 	player_text: The texts with the name of the player
 *  lore: The lore of each player
 *
 * Return:
 *	void
 */
static void draw_player_texts(texture_t **player_text, texture_t **lore)
{
	assert(player_text != NULL && "The texture is NULL");
	assert(lore != NULL && "The texture is NULL");
	
	SDL_Rect p1_box = get_p1_button_box();
	SDL_Rect p2_box = get_p2_button_box();
	SDL_Rect p3_box = get_p3_button_box();

	int box_ofs = dm_get_ofs_player_name();
	int player_h = dm_get_h_player_name();
	p1_box.y += (p1_box.h + box_ofs);
	p1_box.h = player_h;
	p2_box.y += (p2_box.h + box_ofs);
	p2_box.h = player_h;
	p3_box.y += (p3_box.h + box_ofs);
	p3_box.h = player_h;

	dw_draw_texture_center_fit_h(p1_box, player_text[0]);
	dw_draw_texture_center_fit_h(p2_box, player_text[1]);
	dw_draw_texture_center_fit_h(p3_box, player_text[2]);
	
	int lore_ofs = dm_get_ofs_player_lore();
	int lore_h = dm_get_h_player_lore();
	p1_box.y += (p1_box.h + lore_ofs);
	p1_box.h = lore_h;
	p2_box.y += (p2_box.h + lore_ofs);
	p2_box.h = lore_h;
	p3_box.y += (p3_box.h + lore_ofs);
	p3_box.h = lore_h;

	dw_draw_texture_center_fit_h(p1_box, lore[0]);
	dw_draw_texture_center_fit_h(p2_box, lore[1]);
	dw_draw_texture_center_fit_h(p3_box, lore[2]);

}

/* Function: draw_player_figures
 * -----------------------------------------------------------------------------
 * Draws the player figures at the center of the boxes
 * 	
 * Arguments:
 * 	t: Time needed for updating the sign effect
 *
 * Return:
 *	void
 */
static void draw_player_figures(float t)
{
	SDL_Rect p1_box = get_p1_button_box();
	SDL_Rect p2_box = get_p2_button_box();
	SDL_Rect p3_box = get_p3_button_box();

	bool hover_p1 = ax_chk_mouse_hover_rect(p1_box);
	bool hover_p2 = ax_chk_mouse_hover_rect(p2_box);
	bool hover_p3 = ax_chk_mouse_hover_rect(p3_box);
	
	static bool chime_played = false;
	
	int p1_bs = get_player_block_size();
	int p2_bs = get_player_block_size();
	int p3_bs = get_player_block_size();

	int p1_state = AA_OPT_NONE;
	int p2_state = AA_OPT_NONE;
	int p3_state = AA_OPT_NONE;

	float s = bt_get_hover_factor();
	
	SDL_Color p1_color = C_LIGHTGREY;
	SDL_Color p2_color = C_LIGHTGREY;
	SDL_Color p3_color = C_LIGHTGREY;

	if (hover_p1 == true && sb_get_escape_state() == false){
		p1_bs += (int)ceil(((float)p1_bs * s));		
		p1_state = AA_OPT_INVERT;
 		p1_box.y += bt_get_btn_lift();
		p1_color = C_WHITE;
	}
	if (hover_p2 == true && sb_get_escape_state() == false){
		p2_bs += (int)ceil(((float)p2_bs * s));		
		p2_state = AA_OPT_INVERT;
 		p2_box.y += bt_get_btn_lift();
		p2_color = C_WHITE;
	}
	if (hover_p3 == true && sb_get_escape_state() == false){
		p3_bs += (int)ceil(((float)p3_bs * s));		
		p3_state = AA_OPT_INVERT;
 		p3_box.y += bt_get_btn_lift();
		p3_color = C_WHITE;
	}

	p1_box.y += pl_get_sign_y_ofs() * p1_bs;
	p2_box.y += pl_get_sign_y_ofs() * p2_bs;
	p3_box.y += pl_get_sign_y_ofs() * p3_bs;


	int fig_w = pl_get_fig_block_w();
	int fig_h = pl_get_fig_block_h();
	
	p1_box.x += (p1_box.w - fig_w*p1_bs)/2;
	p1_box.y += (p1_box.h - fig_h*p1_bs)/2;
	p2_box.x += (p2_box.w - fig_w*p2_bs)/2;
	p2_box.y += (p2_box.h - fig_h*p2_bs)/2;
	p3_box.x += (p3_box.w - fig_w*p3_bs)/2;
	p3_box.y += (p3_box.h - fig_h*p3_bs)/2;

	struct Item { AA_FigureType type; int x,y; char L; } A[3] = {
        {AA_FIG_WIDE, p1_box.x, p1_box.y,'X'},
        {AA_FIG_MED,  p2_box.x, p2_box.y,'Y'},
        {AA_FIG_SLIM, p3_box.x, p3_box.y,'Z'}
    };
	aa_draw_human(g_renderer, A[0].type, A[0].x, A[0].y, p1_bs, p1_color, 
							  C_NEARBLACK, A[0].L, t, p1_state);
	aa_draw_human(g_renderer, A[1].type, A[1].x, A[1].y, p2_bs, p2_color, 
							  C_NEARBLACK, A[1].L, t, p2_state);
	aa_draw_human(g_renderer, A[2].type, A[2].x, A[2].y, p3_bs, p3_color, 
							  C_NEARBLACK, A[2].L, t, p3_state);

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
	static texture_t *select_player = NULL;
	
	static bool init = false;
	static iface_btn_t *player_btns[3];
	static texture_t *player_text[3];
	static texture_t *player_lore[3];

	int ret_val = LV_SELECT_PLAYER_SCREEN;
	bool player_chosen = false;
	int W = dm_get_screen_width();
	int H = dm_get_screen_height();   
  	static fx_electron_t* fx;
	static Uint64 last_type_ms;
	static Uint64 anim_prev_ms;
	static size_t type_index = 0;
	static bool title_done = false;
	Uint64 cur_time = SDL_GetTicks64();

	SDL_Rect p1_box = get_p1_button_box();
	SDL_Rect p2_box = get_p2_button_box();
	SDL_Rect p3_box = get_p3_button_box();

	if (init == false){
		init = true;
		create_player_btns(player_btns);
		create_player_texts(player_text, player_lore);
		last_type_ms = cur_time;
		anim_prev_ms = cur_time;
		fx = fx_electron_create(g_renderer, W, H, NULL);
	}

	static float t = 0.0f;
	float dt=(cur_time - anim_prev_ms)/1000.0f;
	anim_prev_ms = cur_time;
	t += dt;
	SDL_Rect b = dm_get_upper_title_box(SELECT_PLAYER_TEXT);
	fx_electron_update(fx, dt);
    fx_electron_render(fx, g_renderer);
	
	if (title_done == false && (cur_time - last_type_ms) >= TYPE_DELAY_MS){
		last_type_ms = cur_time;
		
		bool write_complete = tx_draw_create_typewriter_text(&select_player, 
															 b, 
															 SELECT_PLAYER_TEXT, 
															 &type_index,
															 C_SILVERGREY);
		if (write_complete == true){
			title_done = true;
        }
	}
	if (select_player != NULL){
		dw_draw_texture_fit_h(b, select_player);
	}
	
	int plate_ofs = dm_get_ofs_player_dark_plate();
	SDL_Rect dark_plate_1 = ax_pad_rectangle(p1_box, plate_ofs, false);
	SDL_Rect dark_plate_2 = ax_pad_rectangle(p2_box, plate_ofs, false);
	SDL_Rect dark_plate_3 = ax_pad_rectangle(p3_box, plate_ofs, false);
	SDL_Color bg_plate = C_BLACK;
	
	bg_plate.a = 140;
	dw_draw_filled_rectangle(dark_plate_1, bg_plate, bg_plate);
	dw_draw_filled_rectangle(dark_plate_2, bg_plate, bg_plate);
	dw_draw_filled_rectangle(dark_plate_3, bg_plate, bg_plate);
	
	bt_draw_iface_btn(player_btns[0], sb_get_escape_state(), g_sfx_highlight);
	bt_draw_iface_btn(player_btns[1], sb_get_escape_state(), g_sfx_highlight);
	bt_draw_iface_btn(player_btns[2], sb_get_escape_state(), g_sfx_highlight);

	draw_player_texts(player_text, player_lore);
	draw_player_figures(t);

	bool escape_menu = sb_get_escape_state();
	if (escape_menu == true){
		sb_display_escape_menu(sb_get_escape_state());
	} else {
		if (bt_chk_rel_iface_btn(player_btns[0], g_sfx_select) == true){
			player_chosen = true;
			g_player = FL_PLAYER_1;
		} else if (bt_chk_rel_iface_btn(player_btns[1], g_sfx_select) == true){
			player_chosen = true;
			g_player = FL_PLAYER_2;
		} else if (bt_chk_rel_iface_btn(player_btns[2], g_sfx_select) == true){
			player_chosen = true;
			g_player = FL_PLAYER_3;
		}
		if (player_chosen == true){
			ret_val = LV_SELECT_SECTOR;		
			bt_destroy_iface_btn(player_btns[0]);
			bt_destroy_iface_btn(player_btns[1]);
			bt_destroy_iface_btn(player_btns[2]);
		//	dw_free_texture(select_player);
	//		select_player = NULL;
//			type_index = 0;
//			title_done = false;
			dw_free_texture(player_text[0]);
			dw_free_texture(player_text[1]);
			dw_free_texture(player_text[2]);
			dw_free_texture(player_lore[0]);
			dw_free_texture(player_lore[1]);
			dw_free_texture(player_lore[2]);
  			aa_electron_fx_destroy(fx);
			init = false;
		}
	}
	error:
	return ret_val;
}
