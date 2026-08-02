
#include <stdbool.h>
#include <assert.h>
#include "dimensions_dm.h"
#include "draw_dw.h"
#include "levels_lv.h"
#include "file_fl.h"
#include "stages.h"
#include "aux.h"
#include "stage_buttons_sb.h"
#include "ui/escape_menu_em.h"
#include "media/audio_au.h"
#include "ui/screens/screen_common_sc.h"

static const Uint32 SECTOR_BTN_SPACING = 60;
static const Uint32 FIRST_BTN_OFS = 3;
static const Uint32 TYPE_DELAY_MS = 90;  
static const Uint32 SMALL_SEPARATOR_H = 2;
static const Uint32 BIG_SEPARATOR_H = 6;
static const Uint32 SECTOR_TITLE_SEPARATOR = 24;


static char *SELECT_SECTOR = "SELECT SECTOR";

static char *SECTOR_0 = "[00] ARCHITECTURE PRIMER";
static char *SECTOR_1 = "[01] JUMP FLOW UNIT";
static char *SECTOR_2 = "[02] COMPUTATION CORE";
static char *SECTOR_3 = "[03] BRANCHING ENGINE";
static char *SECTOR_4 = "[04] ADVANCED MODULE";

static void create_sector_btns(btn_t **btns, bool *levels);
static int get_sector_btn_spacing();
static SDL_Rect get_sector_btn_box();
static SDL_Rect get_upper_separator();
static int get_fst_btn_ofs();
static SDL_Rect get_sector_separator();


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

	SDL_Rect r = dm_get_upper_title_box(SELECT_SECTOR);
	SDL_Rect b;
	b.w = screen_width*3/4;
	b.h = dm_scale_to_res(BIG_SEPARATOR_H);
	b.x = screen_width/8;
	b.y = r.y + r.h + dm_scale_to_res(SECTOR_TITLE_SEPARATOR); 
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
int stage_select_sector_ss()
{

	static bool level_initialized = false;
	static bool player_levels[LV_LEVEL_QUANTITY];
	static btn_t *sectors[5];	

	int ret_val = LV_SELECT_SECTOR;
	SDL_Rect r = dm_get_upper_title_box(SELECT_SECTOR);
	static sc_fx_t fx_state = {0};
    static Uint64 anim_prev_ms;
    Uint64 cur_time = SDL_GetTicks64();
	static sc_typewriter_t title = {0};

	if (level_initialized == false){
		fl_load_player_levels(g_player, player_levels);
		level_initialized = true;
		sc_typewriter_reset(&title);
		sc_fx_init(&fx_state, cur_time);
		create_sector_btns(sectors, player_levels);
	    anim_prev_ms = cur_time;
	}

	sc_fx_update_render(&fx_state, cur_time);

	static float t = 0.0f;
    float dt=(cur_time - anim_prev_ms)/1000.0f;
    anim_prev_ms = cur_time;
    t += dt;
	
	SDL_Rect b = dm_get_upper_title_box(SELECT_SECTOR);
	bool title_done = sc_typewriter_update(&title, 
										 cur_time, 
										 b, 
										 SELECT_SECTOR, 
										 C_SILVERGREY);  
	if (title.texture != NULL){
		dw_draw_texture_fit_h(b, title.texture);
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
		sc_fx_destroy(&fx_state);
		sc_typewriter_free(&title);
	}

	return ret_val;
}


