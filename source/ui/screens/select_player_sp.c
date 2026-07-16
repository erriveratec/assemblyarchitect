#include <assert.h>
#include "draw_dw.h"
#include "ui/button_bt.h"
#include "levels_lv.h"
#include "dimensions_dm.h"
#include "sdl_config.h"
#include "aux.h"
#include "ui/escape_menu_em.h"
#include "media/audio_au.h"
#include "players_pl.h"
#include "stage_buttons_sb.h"
#include "stages.h"
#include "file_fl.h"
#include "ui/screens/screen_common_sc.h"


static const Uint32 P_BUTTON_H = 280;
static const Uint32 P_BUTTON_W = 280;
static const Uint32 P_BUTTON_Y = 405;

char *SELECT_PLAYER_TEXT = "CHOOSE YOUR ARCHITECT";
char *PLAYER_1_TEXT = "EXECUTOR X";
char *PLAYER_2_TEXT = "OPERATIVE Y";
char *PLAYER_3_TEXT = "HANDLER Z";
char *P1_LORETEXT = "Cross-Branch Executor";
char *P2_LORETEXT = "Yield-Loop Operative";
char *P3_LORETEXT = "Zero-Flag Handler";

static const Uint32 TYPE_DELAY_MS = 90;  

static const Uint32 PLAYER_BLOCK_W = 8;

static SDL_Rect get_p1_button_box();
static SDL_Rect get_p2_button_box();
static SDL_Rect get_p3_button_box();
static void create_player_btns(iface_btn_t **player_btns);
static void create_player_texts(texture_t **player_text, texture_t **lore);
static void draw_player_texts(texture_t **player_text, texture_t **lore);
static int get_player_block_size();

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

/* Function: sp_stage_select_player
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
int sp_stage_select_player()
{
    static bool init = false;
    static iface_btn_t *player_btns[3];
    static texture_t *player_text[3];
    static texture_t *player_lore[3];

    int ret_val = LV_SELECT_PLAYER_SCREEN;
    bool player_chosen = false;
	
	static sc_fx_t fx_state = {0};
    static Uint64 anim_prev_ms;
    Uint64 cur_time = SDL_GetTicks64();

    SDL_Rect p1_box = get_p1_button_box();
    SDL_Rect p2_box = get_p2_button_box();
    SDL_Rect p3_box = get_p3_button_box();

	static sc_typewriter_t title = {0};
    if (init == false){
	   init = true;
		sc_typewriter_reset(&title);
		sc_fx_init(&fx_state, cur_time);
	    create_player_btns(player_btns);
	    create_player_texts(player_text, player_lore);
	    anim_prev_ms = cur_time;
   }

	sc_fx_update_render(&fx_state, cur_time);
    
	static float t = 0.0f;
    float dt=(cur_time - anim_prev_ms)/1000.0f;
    anim_prev_ms = cur_time;
    t += dt;
 
	SDL_Rect b = dm_get_upper_title_box(SELECT_PLAYER_TEXT);
	bool title_done = sc_typewriter_update(&title, 
										 cur_time, 
										 TYPE_DELAY_MS, 
										 b, 
										 SELECT_PLAYER_TEXT, 
										 C_SILVERGREY);  
	if (title.texture != NULL){
		dw_draw_texture_fit_h(b, title.texture);
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

   bool clear_stage = false;
   if (em_get_escape_state() == false){
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
			sc_fx_destroy(&fx_state);
		   init = false;

	   }
   }
   error:
   return ret_val;
}
