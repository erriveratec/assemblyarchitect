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
#include "ui/escape_menu_em.h"
#include "media/audio_au.h"

char *SELECT_LEVEL_TEXT = "SELECT LEVEL";

char *SELECT_SECTOR = "SELECT SECTOR";
char *SECTOR_0 = "[00] ARCHITECTURE PRIMER";
char *SECTOR_1 = "[01] JUMP FLOW UNIT";
char *SECTOR_2 = "[02] COMPUTATION CORE";
char *SECTOR_3 = "[03] BRANCHING ENGINE";
char *SECTOR_4 = "[04] ADVANCED MODULE";
char *SECTOR_0_TITLE = "SECTOR [00]";
char *SECTOR_1_TITLE = "SECTOR [01]";

static const Uint32 TYPE_DELAY_MS = 90;  
static const Uint32 SUBTITLE_FADE_MS = 350;   
static const Uint32 DESCRIPTION_FADE_MS = 150;   

static const Uint32 SEL_LEVEL_BUTTON_W = 200;
static const Uint32 SEL_LEVEL_BUTTON_H = 140;
static const Uint32 SEL_LEVEL_Y_OFS = 70;

static const Uint32 BIG_SEPARATOR_H = 6;
static const Uint32 SMALL_SEPARATOR_H = 2;
static const Uint32 SECTOR_TITLE_SEPARATOR = 24;
static const Uint32 SEP_LEGTH = 1200;

static const Uint32 SECTOR_BTN_SPACING = 60;
static const Uint32 FIRST_BTN_OFS = 3;


static void create_select_level_buttons(iface_btn_t **buttons, bool *levels);

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
	if (bt_chk_rel_iface_btn(levels[0], g_sfx_select)){
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

	if (bt_chk_rel_iface_btn(levels[0], g_sfx_select)){
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
	
	bool clear_stage = false;
	if (em_get_escape_state() == false){
		if (bt_chk_rel_btn(sectors[0], g_sfx_select) == true){
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




