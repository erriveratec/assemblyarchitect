#include <assert.h>
#include <stdbool.h>
#include <SDL.h>
#include <SDL_mixer.h>
#include "dimensions_dm.h"
#include "levels_lv.h"
#include "aux.h"
#include "stage_buttons_sb.h"
#include "file_fl.h"
#include "stages.h"
#include "electron_fx.h"
#include "sdl_config.h"
#include "text_tx.h"
#include "players_pl.h"
#include "ui/escape_menu_em.h"


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
char *SECTOR_0 = "[00] ARCHITECTURE PRIMER";
char *SECTOR_1 = "[01] JUMP FLOW UNIT";
char *SECTOR_2 = "[02] COMPUTATION CORE";
char *SECTOR_3 = "[03] BRANCHING ENGINE";
char *SECTOR_4 = "[04] ADVANCED MODULE";
char *SECTOR_0_TITLE = "SECTOR [00]";
char *SECTOR_1_TITLE = "SECTOR [01]";

static const Uint32 STUDIO_SCREEN_DELAY_MS = 2500; 
static const Uint32 FADE_MS = 1250;
static const Uint32 TYPE_DELAY_MS = 90;  
static const Uint32 CHIP_FADE_MS = 1000;   // ~1s fade
static const Uint32 SUBTITLE_FADE_MS = 350;   
static const Uint32 DESCRIPTION_FADE_MS = 150;   

static const Uint32 PLAYER_BLOCK_W = 8;

static const Uint32 P_BUTTON_H = 280;
static const Uint32 P_BUTTON_W = 280;
static const Uint32 P_BUTTON_Y = 405;

static const Uint32 TITLE_IMG_H = 480;
static const Uint32 TITLE_IMG_W = 480;
static const Uint32 TITLE_IMG_Y = 325;

static const Uint32 SEL_LEVEL_BUTTON_W = 200;
static const Uint32 SEL_LEVEL_BUTTON_H = 140;
static const Uint32 SEL_LEVEL_Y_OFS = 70;

static const Uint32 BIG_SEPARATOR_H = 6;
static const Uint32 SMALL_SEPARATOR_H = 2;
static const Uint32 SECTOR_TITLE_SEPARATOR = 24;
static const Uint32 SEP_LEGTH = 1200;


static const Uint32 SECTOR_BTN_SPACING = 60;
static const Uint32 FIRST_BTN_OFS = 3;



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
static void create_sector_btns(btn_t **btns, bool *levels);
static int get_sector_btn_spacing();
static SDL_Rect get_sector_separator();
static SDL_Rect get_sector_subtitle_box(char *msg);
static SDL_Rect get_subtitle_separator();
static void create_sector_lvl_btns(iface_btn_t **btns, bool *levels, int lv_qty);
static SDL_Rect get_sector_0_lower_separator();


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
	return dm_scale_to_res(SECTOR_BTN_SPACING);
}

/* Function: get_fst_btn_ofs
 * -----------------------------------------------------------------------------
 * Returns the spacing define for each of the buttons of the sector selection
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the ammount of spacing
 */
static int get_fst_btn_ofs()
{
	return dm_scale_to_res(FIRST_BTN_OFS);
}



/* Function: get_subtitle_upper_separator
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the upper separator o select sector
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the objecto
 */
static SDL_Rect get_subtitle_separator()
{
	int screen_width = dm_get_screen_width();
	int screen_height = dm_get_screen_height();

	SDL_Rect r = get_sector_subtitle_box("PLACEHOLDER");
	SDL_Rect b;
	b.w = dm_scale_to_res(SEP_LEGTH);
	b.h = dm_scale_to_res(BIG_SEPARATOR_H);
	b.x = (screen_width - b.w)/2;
	b.y = r.y + r.h + dm_scale_to_res(SECTOR_TITLE_SEPARATOR); 
	return b;
}

/* Function: get_sector_subtitle_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the upper separator o select sector
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the objecto
 */
static SDL_Rect get_sector_subtitle_box(char *msg)
{
	int screen_width = dm_get_screen_width();
	int screen_height = dm_get_screen_height();

	SDL_Rect r = dm_get_upper_title_box(SELECT_LEVEL_TEXT);
	SDL_Rect b;
	b.h = dm_get_h_stage_subtitle();
	b.w = get_text_width_fits_height(b.h, msg);
	b.x = dm_get_screen_width()/2 - b.w/2;
	b.y = r.y + r.h;
	return b;
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
	     + get_fst_btn_ofs(); 
	return b;
}

/* Function: get_sector_0_lower_separator
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the sector separator
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the objecto
 */
static SDL_Rect get_sector_0_lower_separator()
{
	int screen_width = dm_get_screen_width();
	int screen_height = dm_get_screen_height();

	SDL_Rect r = get_level_button_box();	
	int y_offset = get_sel_level_offset_y();
	SDL_Rect b;
	b.w = dm_scale_to_res(SEP_LEGTH);
	b.h = dm_scale_to_res(BIG_SEPARATOR_H);
	b.x = (screen_width - b.w)/2;	
	b.y = r.y + 2*y_offset;
	
	return b;
}

/* Function: get_sector_separator
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
	b.h = dm_scale_to_res(SMALL_SEPARATOR_H);
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
	b.h = dm_scale_to_res(BIG_SEPARATOR_H);
	b.x = screen_width/8;
	b.y = r.y + r.h + dm_scale_to_res(SECTOR_TITLE_SEPARATOR); 
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
	int y_offset =  dm_scale_to_res(SEL_LEVEL_BUTTON_H + 
										SEL_LEVEL_Y_OFS);
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

	SDL_Rect r = get_subtitle_separator();
	SDL_Rect b;
	b.w = dm_scale_to_res(SEL_LEVEL_BUTTON_W);
	b.h = dm_scale_to_res(SEL_LEVEL_BUTTON_H);
	b.x = (screen_width - (4*b.w + 3*b.w/2))/2;
	b.y = r.y + r.h + get_sector_btn_spacing() 
	      + get_fst_btn_ofs(); 

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
	return dm_scale_to_res(PLAYER_BLOCK_W);
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
	b.w = dm_scale_to_res(TITLE_IMG_W);
	b.h = dm_scale_to_res(TITLE_IMG_H);
	b.x = (screen_width - b.w)/2;
	b.y = dm_scale_to_res(TITLE_IMG_Y);
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
	b.w = dm_scale_to_res(P_BUTTON_W);
	b.h = dm_scale_to_res(P_BUTTON_H);
	b.x = (screen_width - 3*b.w)/4;
	b.y = dm_scale_to_res(P_BUTTON_Y);
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
	b.w = dm_scale_to_res(P_BUTTON_W);
	b.h = dm_scale_to_res(P_BUTTON_H);
	b.x = 2*(screen_width - 3*b.w)/4 + b.w;
	b.y = dm_scale_to_res(P_BUTTON_Y);
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
	b.w = dm_scale_to_res(P_BUTTON_W);
	b.h = dm_scale_to_res(P_BUTTON_H);
	b.x = 3*(screen_width - 3*b.w)/4 + 2*b.w;
	b.y = dm_scale_to_res(P_BUTTON_Y);
	return b;
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
static void create_sector_btns(btn_t **btns, bool *levels)
{
	assert(btns != NULL && "The buttons pointer is NULL");

	int y_space = get_sector_btn_spacing();

	btns[0] = bt_create_btn(get_sector_btn_box(), 
			  				dw_create_text_tex(SECTOR_0, C_WHITE));
	btns[0]->r.w = ax_get_texture_w_fit_h(btns[0]->r.h, btns[0]->t);
	int x_ofs = (dm_get_upper_title_box(SELECT_SECTOR).w - btns[0]->r.w)/2;
	btns[0]->r.x += x_ofs;
	
	btns[1] = bt_create_btn(get_sector_btn_box(), 
			  				dw_create_text_tex(SECTOR_1, C_WHITE));
	btns[1]->r.w = ax_get_texture_w_fit_h(btns[1]->r.h, btns[1]->t);
	btns[1]->r.y = btns[0]->r.y + get_sector_btn_box().h + y_space;
	btns[1]->r.x += x_ofs;
	if (levels[LV_SECTOR_1_START] == false){
		btns[1]->enabled = false;
	}
	
	btns[2] = bt_create_btn(get_sector_btn_box(), 
			  				dw_create_text_tex(SECTOR_2, C_WHITE));
	btns[2]->r.w = ax_get_texture_w_fit_h(btns[2]->r.h, btns[2]->t);
	btns[2]->r.y = btns[1]->r.y + get_sector_btn_box().h + y_space;
	btns[2]->r.x += x_ofs;
	if (levels[LV_SECTOR_2_START] == false){
		btns[2]->enabled = false;
	}

	btns[3] = bt_create_btn(get_sector_btn_box(), 
			  				dw_create_text_tex(SECTOR_3, C_WHITE));
	btns[3]->r.w = ax_get_texture_w_fit_h(btns[3]->r.h, btns[3]->t);
	btns[3]->r.y = btns[2]->r.y + get_sector_btn_box().h + y_space;
	btns[3]->r.x += x_ofs;
	if (levels[LV_SECTOR_3_START] == false){
		btns[3]->enabled = false;
	}

	btns[4] = bt_create_btn(get_sector_btn_box(), 
			  				dw_create_text_tex(SECTOR_4, C_WHITE));
	btns[4]->r.w = ax_get_texture_w_fit_h(btns[4]->r.h, btns[4]->t);
	btns[4]->r.y = btns[3]->r.y + get_sector_btn_box().h + y_space;
	btns[4]->r.x += x_ofs;
	if (levels[LV_SECTOR_4_START] == false){
		btns[4]->enabled = false;
	}
}

/* Function: create_level_btns
 * -----------------------------------------------------------------------------
 * Creates the corresponding sector buttons
 * 	
 * Arguments:
 * 	Pointer of pointer of the buttons
 *	Pointer with an array of available levels
 *	The quantity of levels that will be initialized
 *
 * Return:
 *	Void.
 */
static void create_sector_lvl_btns(iface_btn_t **btns, bool *lvls, int sec_id)
{
	assert(btns != NULL && "The buttons pointer is NULL");
	assert(lvls != NULL && "The levels pointer is NULL");

	SDL_Rect r = get_level_button_box();
	SDL_Rect b = r;
	int y_offset = get_sel_level_offset_y();

	int start;
	int finish;

	if (sec_id == LV_SECTOR_0){
		start = 0;
		finish = LV_SECTOR_1_START;
	} else if (sec_id == LV_SECTOR_1){
		start = LV_SECTOR_1_START;
		finish = LV_SECTOR_2_START;
	} else if (sec_id == LV_SECTOR_2){
		start = LV_SECTOR_2_START;
		finish = LV_SECTOR_3_START;
	} else if (sec_id == LV_SECTOR_3){
		start = LV_SECTOR_3_START;
		finish = LV_SECTOR_4_START;
	} else if (sec_id == LV_SECTOR_4){
		start = LV_SECTOR_4_START;
		finish = LV_LEVEL_QUANTITY;
	}
	int counter = 0;	
	for (int i = start; i < finish; i++){
		char *btn_text = ax_number_to_hex_string_two_digits(i);
		texture_t *btn_texture = NULL;
		if (lvls[i] == true){
			btn_texture = dw_create_text_tex(btn_text, C_WHITE);
			btns[counter] = bt_create_iface_btn(b, btn_texture, true);
		} else {
			btn_texture = dw_create_text_tex(btn_text, C_GREY);
			btns[counter] = bt_create_iface_btn(b, btn_texture, false);
		}
		b.x += r.w + r.w/2;
		if ((i % 4) == 3){
			b.x = r.x;
			b.y += y_offset;	
		}
		counter++;
	}


}
/* Function: stage_sector_1
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
int stage_sector_1()
{
	int W = dm_get_screen_width();
	int H = dm_get_screen_height();   
  	static fx_electron_t* fx;
	static Uint64 last_type_ms;
	static Uint64 anim_prev_ms;
	static Uint64 sub_start_ms;
	static size_t type_index = 0;
	static bool title_done = false;
	Uint64 cur_time = SDL_GetTicks64();
	bool change_stage = false;

	static texture_t *sector_0 = NULL;
	static texture_t *subtitle = NULL;
	char *sub_text = SECTOR_1 + 5;
	Uint8 sub_alpha = 0;              

	static bool level_initialized = false;
	static bool player_levels[LV_LEVEL_QUANTITY];
	static iface_btn_t **levels = NULL;

	int ret_val = LV_SECTOR_1;
	SDL_Rect r = dm_get_upper_title_box(SECTOR_1_TITLE);

	if (level_initialized == false){
		fl_load_hover_level_msgs();
		fl_load_player_levels(g_player, player_levels);
		level_initialized = true;
		last_type_ms = cur_time;
		fx = fx_electron_create(g_renderer, W, H, NULL);
		levels = malloc(sizeof(iface_btn_t *) * LV_SECTOR_LV_QTY);
		create_sector_lvl_btns(levels, player_levels, LV_SECTOR_1);
		subtitle = dw_create_text_tex(sub_text, C_SILVERGREY);
        SDL_SetTextureAlphaMod(subtitle->texture, sub_alpha);
	}

	float dt=(cur_time - anim_prev_ms)/1000.0f;
	anim_prev_ms = cur_time;
	fx_electron_update(fx, dt);
    fx_electron_render(fx, g_renderer);
	
	if (title_done == false && (cur_time - last_type_ms) >= TYPE_DELAY_MS){
		last_type_ms = cur_time;
		bool write_complete = tx_draw_create_typewriter_text(&sector_0, 
															 r, 
															 SECTOR_1_TITLE, 
															 &type_index,
															 C_SILVERGREY);
		if (write_complete == true){
			title_done = true;
			sub_start_ms = cur_time;
        }
	}
	if (sector_0 != NULL){
		dw_draw_texture_fit_h(r, sector_0);
	}
	if (title_done == true) {
    	Uint64 elapsed = cur_time - sub_start_ms;
        if (elapsed < SUBTITLE_FADE_MS) {
            float t = (float)elapsed / (float)SUBTITLE_FADE_MS;
            if (t < 0.0f) t = 0.0f; if (t > 1.0f) t = 1.0f;
            sub_alpha = (Uint8)(t * 255.0f);
        } else {
            sub_alpha = 255;
        }
        	SDL_SetTextureAlphaMod(subtitle->texture, sub_alpha);
    }

	SDL_Rect sub_box = get_sector_subtitle_box(sub_text);
	dw_draw_texture_fit_h(sub_box, subtitle);
	
	dw_draw_filled_rectangle(get_subtitle_separator(), 
							 C_SHADOWGREY, C_SHADOWGREY);
	sb_draw_ret_btn();

	static texture_array_t *hover = NULL;
	static Uint64 level_sub_start_ms;
	Uint8 level_alpha = 0;
	static bool prev_hov = false;
	bool button_hovered = false;
	
	for (int i = 0; i < LV_SECTOR_LV_QTY; i++){
		int hov = bt_draw_iface_btn(levels[i], em_get_escape_state(), NULL);
		if ((hov == BTN_HOVER || hov == BTN_CLICKPRESS) && prev_hov == false){
			hover = tx_get_message_texture(LV_SECTOR_1_START + i);
			level_sub_start_ms = cur_time;
			prev_hov = true;
		} 	
		if (hov == BTN_HOVER || hov == BTN_CLICKPRESS){
			button_hovered = true;
		}
	}
	if (button_hovered == false){
		prev_hov = false;
		hover = NULL;
	}

	
	SDL_Rect sep = get_sector_0_lower_separator();
	sep.y += sep.h + dm_scale_to_res(SECTOR_TITLE_SEPARATOR);
	
	if (hover != NULL) {
		dw_set_array_texture_color_mod(hover, C_GREY);
    	Uint64 elapsed = cur_time - level_sub_start_ms;
        if (elapsed < DESCRIPTION_FADE_MS) {
            float t = (float)elapsed / (float)DESCRIPTION_FADE_MS;
            if (t < 0.0f) t = 0.0f; if (t > 1.0f) t = 1.0f;
            level_alpha = (Uint8)(t * 255.0f);
        } else {
            level_alpha = 255;
        }
        	SDL_SetTextureAlphaMod(hover->t[0]->texture, level_alpha);
		dw_draw_wrapped_texture_by_h(sep, dm_get_h_stage_subsubtitle(), hover);
    }

	bool escape = em_get_escape_state();
	if (escape == true){
		sb_display_escape_menu(escape);
	} else if (bt_chk_rel_iface_btn(levels[0], g_sfx_select)){
		ret_val = LV_LEVEL_8;
		change_stage = true;
	} else if (bt_chk_rel_iface_btn(levels[1], g_sfx_select)){
		ret_val = LV_LEVEL_9;
		change_stage = true;
	} else if (bt_chk_rel_iface_btn(levels[2], g_sfx_select)){
		ret_val = LV_LEVEL_10;
		change_stage = true;
	} else if (bt_chk_rel_iface_btn(levels[3], g_sfx_select)){
		ret_val = LV_LEVEL_11;
		change_stage = true;
	} else if (bt_chk_rel_iface_btn(levels[4], g_sfx_select)){
		ret_val = LV_LEVEL_12;
		change_stage = true;
	} else if (bt_chk_rel_iface_btn(levels[5], g_sfx_select)){
		ret_val = LV_LEVEL_13;
		change_stage = true;
	} else if (bt_chk_rel_iface_btn(levels[6], g_sfx_select)){
		ret_val = LV_LEVEL_14;
		change_stage = true;
	} else if (bt_chk_rel_iface_btn(levels[7], g_sfx_select)){
		ret_val = LV_LEVEL_15;
		change_stage = true;
	} else if (sb_chck_rel_ret_btn() == true){
		bt_destroy_iface_btn(levels[0]);
		bt_destroy_iface_btn(levels[1]);
		bt_destroy_iface_btn(levels[2]);
		bt_destroy_iface_btn(levels[3]);
		bt_destroy_iface_btn(levels[4]);
		bt_destroy_iface_btn(levels[5]);
		bt_destroy_iface_btn(levels[6]);
		bt_destroy_iface_btn(levels[7]);
		ret_val = LV_SELECT_SECTOR;	
		change_stage = true;
	}
	if (change_stage == true){
		level_initialized = false;
  		aa_electron_fx_destroy(fx);
		tx_free_level_text_textures();
	}

	return ret_val;
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
	static Uint64 sub_start_ms;
	static size_t type_index = 0;
	static bool title_done = false;
	Uint64 cur_time = SDL_GetTicks64();
	bool change_stage = false;

	static texture_t *sector_0 = NULL;
	static texture_t *subtitle = NULL;
	char *sub_text = SECTOR_0 + 5;
	Uint8 sub_alpha = 0;              

	static bool level_initialized = false;
	static bool player_levels[LV_LEVEL_QUANTITY];
	static iface_btn_t **levels = NULL;

	int ret_val = LV_SECTOR_0;
	SDL_Rect r = dm_get_upper_title_box(SECTOR_0_TITLE);

	if (level_initialized == false){
		fl_load_hover_level_msgs();
		fl_load_player_levels(g_player, player_levels);
		level_initialized = true;
		last_type_ms = cur_time;
		fx = fx_electron_create(g_renderer, W, H, NULL);
		levels = malloc(sizeof(iface_btn_t *) * LV_SECTOR_LV_QTY);
		create_sector_lvl_btns(levels, player_levels, LV_SECTOR_0);
		subtitle = dw_create_text_tex(sub_text, C_SILVERGREY);
        SDL_SetTextureAlphaMod(subtitle->texture, sub_alpha);
	}

	float dt=(cur_time - anim_prev_ms)/1000.0f;
	anim_prev_ms = cur_time;
	fx_electron_update(fx, dt);
    fx_electron_render(fx, g_renderer);
	
	if (title_done == false && (cur_time - last_type_ms) >= TYPE_DELAY_MS){
		last_type_ms = cur_time;
		bool write_complete = tx_draw_create_typewriter_text(&sector_0, 
															 r, 
															 SECTOR_0_TITLE, 
															 &type_index,
															 C_SILVERGREY);
		if (write_complete == true){
			title_done = true;
			sub_start_ms = cur_time;
        }
	}
	if (sector_0 != NULL){
		dw_draw_texture_fit_h(r, sector_0);
	}
	
	if (title_done == true) {
    	Uint64 elapsed = cur_time - sub_start_ms;
        if (elapsed < SUBTITLE_FADE_MS) {
            float t = (float)elapsed / (float)SUBTITLE_FADE_MS;
            if (t < 0.0f) t = 0.0f; if (t > 1.0f) t = 1.0f;
            sub_alpha = (Uint8)(t * 255.0f);
        } else {
            sub_alpha = 255;
        }
        	SDL_SetTextureAlphaMod(subtitle->texture, sub_alpha);
    }

	SDL_Rect sub_box = get_sector_subtitle_box(sub_text);
	dw_draw_texture_fit_h(sub_box, subtitle);
	
	dw_draw_filled_rectangle(get_subtitle_separator(), 
							 C_SHADOWGREY, C_SHADOWGREY);
	sb_draw_ret_btn();

	static texture_array_t *hover = NULL;
	static Uint64 level_sub_start_ms;
	Uint8 level_alpha = 0;
	static bool prev_hov = false;
	bool button_hovered = false;
	
	for (int i = 0; i < LV_SECTOR_LV_QTY; i++){
		int hov = bt_draw_iface_btn(levels[i], em_get_escape_state(), NULL);
		if ((hov == BTN_HOVER || hov == BTN_CLICKPRESS) && prev_hov == false){
			hover = tx_get_message_texture(i);
			level_sub_start_ms = cur_time;
			prev_hov = true;
		} 	
		if (hov == BTN_HOVER || hov == BTN_CLICKPRESS){
			button_hovered = true;
		}
	}
	if (button_hovered == false){
		prev_hov = false;
		hover = NULL;
	}

	
	SDL_Rect sep = get_sector_0_lower_separator();
	sep.y += sep.h + dm_scale_to_res(SECTOR_TITLE_SEPARATOR);
	
	if (hover != NULL) {
		dw_set_array_texture_color_mod(hover, C_GREY);
    	Uint64 elapsed = cur_time - level_sub_start_ms;
        if (elapsed < DESCRIPTION_FADE_MS) {
            float t = (float)elapsed / (float)DESCRIPTION_FADE_MS;
            if (t < 0.0f) t = 0.0f; if (t > 1.0f) t = 1.0f;
            level_alpha = (Uint8)(t * 255.0f);
        } else {
            level_alpha = 255;
        }
        	SDL_SetTextureAlphaMod(hover->t[0]->texture, level_alpha);
		dw_draw_wrapped_texture_by_h(sep, dm_get_h_stage_subsubtitle(), hover);
    }

	bool escape = em_get_escape_state();
	if (escape == true){
		sb_display_escape_menu(escape);
	} else if (bt_chk_rel_iface_btn(levels[0], g_sfx_select)){
		ret_val = LV_LEVEL_0;
		change_stage = true;
	} else if (bt_chk_rel_iface_btn(levels[1], g_sfx_select)){
		ret_val = LV_LEVEL_1;
		change_stage = true;
	} else if (bt_chk_rel_iface_btn(levels[2], g_sfx_select)){
		ret_val = LV_LEVEL_2;
		change_stage = true;
	} else if (bt_chk_rel_iface_btn(levels[3], g_sfx_select)){
		ret_val = LV_LEVEL_3;
		change_stage = true;
	} else if (bt_chk_rel_iface_btn(levels[4], g_sfx_select)){
		ret_val = LV_LEVEL_4;
		change_stage = true;
	} else if (bt_chk_rel_iface_btn(levels[5], g_sfx_select)){
		ret_val = LV_LEVEL_5;
		change_stage = true;
	} else if (bt_chk_rel_iface_btn(levels[6], g_sfx_select)){
		ret_val = LV_LEVEL_6;
		change_stage = true;
	} else if (bt_chk_rel_iface_btn(levels[7], g_sfx_select)){
		ret_val = LV_LEVEL_7;
		change_stage = true;
	} else if (sb_chck_rel_ret_btn() == true){
		bt_destroy_iface_btn(levels[0]);
		bt_destroy_iface_btn(levels[1]);
		bt_destroy_iface_btn(levels[2]);
		bt_destroy_iface_btn(levels[3]);
		bt_destroy_iface_btn(levels[4]);
		bt_destroy_iface_btn(levels[5]);
		bt_destroy_iface_btn(levels[6]);
		bt_destroy_iface_btn(levels[7]);
		ret_val = LV_SELECT_SECTOR;	
		change_stage = true;
	}
	if (change_stage == true){
		level_initialized = false;
  		aa_electron_fx_destroy(fx);
		tx_free_level_text_textures();
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
		create_sector_btns(sectors, player_levels);
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
	sb_draw_ret_btn();
	
	SDL_Rect sep = get_sector_separator();
	for (int i = 0; i < 5; i++){
		bt_draw_btn(sectors[i], em_get_escape_state(), false);
		dw_draw_filled_rectangle(sep, C_SHADOWGREY, C_SHADOWGREY);
		sep.y += get_sector_btn_box().h + get_sector_btn_spacing();
	}
	
	bool escape = em_get_escape_state();
	bool clear_stage = false;
	if (escape == true){
		sb_display_escape_menu(escape);
	} else if (bt_chk_rel_btn(sectors[0], g_sfx_select)){
		ret_val = LV_SECTOR_0;
		clear_stage = true;
	} else if (bt_chk_rel_btn(sectors[1], g_sfx_select)){
		ret_val = LV_SECTOR_1;
		clear_stage = true;
	} else if (bt_chk_rel_btn(sectors[2], g_sfx_select)){
	} else if (bt_chk_rel_btn(sectors[3], g_sfx_select)){
	} else if (bt_chk_rel_btn(sectors[4], g_sfx_select)){
	} else if (sb_chck_rel_ret_btn() == true){
		ret_val = LV_SELECT_PLAYER_SCREEN;	
		clear_stage = true;
	}
	if (clear_stage == true){
		bt_destroy_button(sectors[0]);
		bt_destroy_button(sectors[1]);
		bt_destroy_button(sectors[2]);
		bt_destroy_button(sectors[3]);
		bt_destroy_button(sectors[4]);
		level_initialized = false;
  		aa_electron_fx_destroy(fx);
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
		press_space = dw_create_text_tex(PRESS_SPACE, C_SILVERGREY);
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
	//	dw_free_texture(g_chip);
//		dw_free_texture(press_space);
//		dw_free_texture(game_title);
  		aa_electron_fx_destroy(fx);
		init = false;
		ret_val = LV_SELECT_PLAYER_SCREEN;
	} else {
		ret_val = LV_TITLE_SCREEN;
	}

	sb_display_escape_menu(em_get_escape_state());
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
								   dw_create_text_tex(" ", C_BLACK), 
								   true);
	
	player_btns[1] = bt_create_iface_btn(p2_box, 
								   dw_create_text_tex(" ", C_BLACK), 
								   true);

	player_btns[2] = bt_create_iface_btn(p3_box, 
								   dw_create_text_tex(" ", C_BLACK),
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
	
	player_text[0] = dw_create_text_tex(PLAYER_1_TEXT, C_SILVERGREY);
	player_text[1] = dw_create_text_tex(PLAYER_2_TEXT, C_SILVERGREY);
	player_text[2] = dw_create_text_tex(PLAYER_3_TEXT, C_SILVERGREY);

	lore[0] = dw_create_text_tex(P1_LORETEXT, C_GREY);
	lore[1] = dw_create_text_tex(P2_LORETEXT, C_GREY);
	lore[2] = dw_create_text_tex(P3_LORETEXT, C_GREY);
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
	int lore_h = dm_get_h_stage_subsubtitle();
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

	if (hover_p1 == true && em_get_escape_state() == false){
		p1_bs += (int)ceil(((float)p1_bs * s));		
		p1_state = AA_OPT_INVERT;
 		p1_box.y += bt_get_btn_lift();
		p1_color = C_WHITE;
	}
	if (hover_p2 == true && em_get_escape_state() == false){
		p2_bs += (int)ceil(((float)p2_bs * s));		
		p2_state = AA_OPT_INVERT;
 		p2_box.y += bt_get_btn_lift();
		p2_color = C_WHITE;
	}
	if (hover_p3 == true && em_get_escape_state() == false){
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
	
	bt_draw_iface_btn(player_btns[0], em_get_escape_state(), g_sfx_highlight);
	bt_draw_iface_btn(player_btns[1], em_get_escape_state(), g_sfx_highlight);
	bt_draw_iface_btn(player_btns[2], em_get_escape_state(), g_sfx_highlight);

	draw_player_texts(player_text, player_lore);
	draw_player_figures(t);
	sb_draw_ret_btn();

	bool escape_menu = em_get_escape_state();
	bool clear_stage = false;
	if (escape_menu == true){
		sb_display_escape_menu(em_get_escape_state());
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
		} else if (sb_chck_rel_ret_btn() == true){
			ret_val = LV_TITLE_SCREEN;	
			clear_stage = true;
		}
		if (player_chosen == true){
			ret_val = LV_SELECT_SECTOR;		
			clear_stage = true;
		}
		if (clear_stage == true){
			bt_destroy_iface_btn(player_btns[0]);
			bt_destroy_iface_btn(player_btns[1]);
			bt_destroy_iface_btn(player_btns[2]);
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
