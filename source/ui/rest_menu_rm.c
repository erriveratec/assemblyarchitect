#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "draw_dw.h"
#include "ui/button_bt.h"
#include "dimensions_dm.h"
#include "ui/reset_menu_rm.h"
#include "ui/escape_menu_em.h"
#include "media/audio_au.h"


static char *SYSTEM_ALERT = "SYSTEM ALERT";
#define RST_MENU_TEXT0 "Do you want to reset the current address?"
#define RST_MENU_TEXT1 "NO"
#define RST_MENU_TEXT2 "YES"

static Uint32 RST_MENU_BTNS_W = 100;
static Uint32 RST_MENU_BTNS_H = 60;

bool g_rst_menu = false;
	
texture_array_t *g_rst_menu_text = NULL;
texture_t *g_rst_b1_texture = NULL;
texture_t *g_rst_b2_texture = NULL;
texture_t *g_system_alert_texture = NULL;

iface_btn_t *g_rst_b1;
iface_btn_t *g_rst_b2;

static SDL_Rect get_rst_b1_box();
static SDL_Rect get_rst_b2_box();

/* Function: sb_get_rst_menu_state
 * ----------------------------------------------------------------------------
 * This function return the boolean state of the rst menu.
 *
 * Arguments:
 * 	None.
 *
 * Return:
 *	Boolean with the state of the rst menu variable
 */
bool rm_chk_rst_menu_state()
{
	return g_rst_menu;
}

/* Function: rm_chk_rst_menu_btns
 * -----------------------------------------------------------------------------
 * Returns a boolean indicating what buttons was pressed in the rst menu
 * 
 * Arguments:
 *	Void.
 *
 * Return:
 *	false if not level rst, true if level reset
 */
bool rm_chk_rst_menu_btns(bool show_menu)
{
	bool reset = false;
	if (show_menu == true){

		if (bt_chk_rel_iface_btn(g_rst_b1, g_sfx_iface_back_cancel) == true){
			reset = false;
			rm_set_rst_menu(false);
		} else if (bt_chk_rel_iface_btn(g_rst_b2, g_sfx_iface_back_cancel) == true){
			reset = true;
			rm_set_rst_menu(false);
		} 	
	}
	return reset;
}

/* Function: rm_set_rst_menu
 * ----------------------------------------------------------------------------
 * Sets the status of the rst menu to be shown to the player
 *
 * Arguments:
 * 	state: state ot it the g_rst_menu variable will be set.
 *
 * Return:
 *	Void.	
 */
void rm_set_rst_menu(bool state)
{
	g_rst_menu = state;
}

/* Function: dm_get_rst_b1_box
 * -----------------------------------------------------------------------------
 * Box position of the button 1 of the reset menu
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
static SDL_Rect get_rst_b1_box()
{
	int cont_w = dm_scale_to_res(RST_MENU_BTNS_W);
	int offset = dm_get_ofs_res_box();
	SDL_Rect rb = dw_get_iface_big_center_box();
	SDL_Rect cb = dw_get_iface_content_box(rb);
	SDL_Rect b;
	b.w = dm_scale_to_res(RST_MENU_BTNS_W);
	b.h = dm_scale_to_res(RST_MENU_BTNS_H);
	b.x = cb.x + (cb.w - (b.w + cont_w))/3;
	b.y = rb.y + rb.h - b.h - offset;
	return b;

}

/* Function: dm_get_rst_b2_box
 * -----------------------------------------------------------------------------
 * Box position of the button 2 of the reset menu
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
static SDL_Rect get_rst_b2_box()
{
	int back_w = dm_scale_to_res(RST_MENU_BTNS_W);
	int cont_w = dm_scale_to_res(RST_MENU_BTNS_W);
	int offset = dm_get_ofs_res_box();
	SDL_Rect rb = dw_get_iface_big_center_box();
	SDL_Rect cb = dw_get_iface_content_box(rb);
	SDL_Rect b;
	b.w = dm_scale_to_res(RST_MENU_BTNS_W);
	b.h = dm_scale_to_res(RST_MENU_BTNS_H);
	b.x = cb.x + back_w + 2*(cb.w - (back_w + cont_w))/3 ;
	b.y = rb.y + rb.h - b.h - offset;
	return b;

}

/* Function: mb_init_rst_menu
 * ----------------------------------------------------------------------------
 * Initializes the assets of the reset menu.
 *
 * Arguments:
 * 	Void.
 *
 * Return:
 *	Void.	
 */
void rm_init_rst_menu()
{
	g_system_alert_texture = dw_create_text_tex(SYSTEM_ALERT, C_GREY);
	g_rst_b1_texture = dw_create_text_tex(RST_MENU_TEXT1, C_WHITE);

	g_rst_b2_texture = dw_create_text_tex(RST_MENU_TEXT2, C_WHITE);
	
	SDL_Rect r = get_rst_b1_box();
	g_rst_b1 = bt_create_iface_btn(r, g_rst_b1_texture, true);

	r = get_rst_b2_box();
	g_rst_b2 = bt_create_iface_btn(r, g_rst_b2_texture, true);

	int h = dm_get_h_msg();
	int w = dw_get_iface_content_box(dw_get_iface_big_center_box()).w;
	g_rst_menu_text = dw_create_text_tex_array_by_h(w, 
													h, 
													C_WHITE, 
													RST_MENU_TEXT0);
	return;
}


/* Function: rm_render_rst_menu
 * -----------------------------------------------------------------------------
 * Displays the rst menu that is show when the reset button is pressed.
 * 
 * Arguments:
 *	show_menu: a variable that determines if the menu will be shown.
 *
 * Return:
 *	bool indicating if the menu will be result
 */
void rm_render_rst_menu(bool show_menu)
{
	if (show_menu == true){
		SDL_Rect r = dw_get_iface_big_center_box();
		dw_draw_iface_box(r, g_system_alert_texture);

		SDL_Rect text_box = dw_get_iface_content_box(r);
		text_box.h -= get_rst_b1_box().h;

		int text_h = dm_get_h_msg();		
		dw_draw_wrapped_texture_by_h(text_box, text_h, g_rst_menu_text);

		bt_draw_iface_btn(g_rst_b1, em_get_escape_state(), g_sfx_iface_hover);
		bt_draw_iface_btn(g_rst_b2, em_get_escape_state(), g_sfx_iface_hover);
	}
	return;
}
