#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL.h>
#include <SDL_mixer.h>
#include "sdl_config.h"
#include "ui/escape_menu_em.h"
#include "ui/button_bt.h"
#include "draw_dw.h"
#include "dimensions_dm.h"

#define ESC_MENU_TEXT1 "RETURN TO GAME"
#define ESC_MENU_TEXT2 "TOGGLE FULL SCREEN"
#define ESC_MENU_TEXT3 "EXIT GAME"
static char *ESC_MENU_HEADER = "ESC MENU";

static Uint32 ESC_MENU_BUTTON_W = 340;
static Uint32 ESC_MENU_BUTTON_H = 60;
static Uint32 ESC_MENU_BUTTON_SPACE = 12;

bool g_escape_menu = false;

texture_t *g_escape_b1_texture = NULL;
texture_t *g_escape_b2_texture = NULL;
texture_t *g_escape_b3_texture = NULL;
texture_t *g_escape_header_texture = NULL;


iface_btn_t *g_escape_b1;
iface_btn_t *g_escape_b2;
iface_btn_t *g_escape_b3;


static SDL_Rect get_escape_b1_box();
static SDL_Rect get_escape_b2_box();
static SDL_Rect get_escape_b3_box();




/* Function: get_escape_b1_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
static SDL_Rect get_escape_b1_box()
{
	SDL_Rect mb = dw_get_iface_big_center_box();

	SDL_Rect b;
	b.w = dm_scale_to_res(ESC_MENU_BUTTON_W);
	b.h = dm_scale_to_res(ESC_MENU_BUTTON_H);
	b.x = mb.x + mb.w/2 - b.w/2;
	b.y = mb.y + mb.h - 3*b.h - 5*dm_scale_to_res(ESC_MENU_BUTTON_SPACE);
	return b;
}

/* Function: get_escape_b2_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
static SDL_Rect get_escape_b2_box()
{
	SDL_Rect mb = dw_get_iface_big_center_box();
	SDL_Rect b1 = get_escape_b1_box();
	
	SDL_Rect b;
	b.w = dm_scale_to_res(ESC_MENU_BUTTON_W);
	b.h = dm_scale_to_res(ESC_MENU_BUTTON_H);
	b.x = b1.x;
	b.y = b1.y + b.h + dm_scale_to_res(ESC_MENU_BUTTON_SPACE);
	return b;
}

/* Function: get_escape_b3_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
static SDL_Rect get_escape_b3_box()
{
	SDL_Rect mb = dw_get_iface_big_center_box();
	SDL_Rect b2 = get_escape_b2_box();
	
	SDL_Rect b;
	b.w = dm_scale_to_res(ESC_MENU_BUTTON_W);
	b.h = dm_scale_to_res(ESC_MENU_BUTTON_H);
	b.x = b2.x;
	b.y = b2.y + b.h + dm_scale_to_res(ESC_MENU_BUTTON_SPACE);
	return b;
}


/* Function: player_pressed_escape_key
 * ----------------------------------------------------------------------------
 * This function inverts the state of the escape menu flag in case that the 
 * player has pressed the escape key.
 *
 * Arguments:
 * 	None.
 *
 * Return:
 *	void.	
 */
void em_toggle_escape_menu()
{
	g_escape_menu = !g_escape_menu;

	if (g_escape_menu == true){			
		if (g_sfx_menu) Mix_PlayChannel(-1, g_sfx_menu, 0);
	} else if (g_escape_menu == false){			
		if (g_sfx_iface_back_cancel) Mix_PlayChannel(-1, 
			g_sfx_iface_back_cancel, 0);
	} 
}

/* Function: em_get_escape_state
 * ----------------------------------------------------------------------------
 * This function return the boolean state of the state.
 *
 * Arguments:
 * 	None.
 *
 * Return:
 *	Boolean with the state fo the escape_menu variable
 */
bool em_get_escape_state()
{
	return g_escape_menu;
}


/* Function: em_init_escape_menu
 * ----------------------------------------------------------------------------
 * Initializes all the assets of the escape menu.
 *
 * Arguments:
 * 	Void.
 *
 * Return:
 *	Void.	
 */
void em_init_escape_menu()
{
	g_escape_b1_texture = dw_create_text_tex(ESC_MENU_TEXT1, C_WHITE);

	g_escape_b2_texture = dw_create_text_tex(ESC_MENU_TEXT2, C_WHITE);

	g_escape_b3_texture = dw_create_text_tex(ESC_MENU_TEXT3, C_WHITE);
	
	g_escape_header_texture = dw_create_text_tex(ESC_MENU_HEADER, C_GREY);
	

	SDL_Rect r = get_escape_b1_box();
	g_escape_b1 = bt_create_iface_btn(r, g_escape_b1_texture, true);

	r = get_escape_b2_box();
	g_escape_b2 = bt_create_iface_btn(r, g_escape_b2_texture, true);
	
	r = get_escape_b3_box();
	g_escape_b3 = bt_create_iface_btn(r, g_escape_b3_texture, true);
	
	return;
}

/* Function: sb_display_escape_menu
 * -----------------------------------------------------------------------------
 * This function is called in all the stages, it will show the escape menu 
 * according to the state of the g_escape_menu variable.
 * 
 * Arguments:
 *	menu_variable_state: The state of the show menu variable.
 *
 * Return:
 *	void
 *
void sb_display_escape_menu(bool show_menu)
{
	if (show_menu == true){
		
		SDL_Rect r = dw_get_iface_big_center_box();
		dw_draw_iface_box(r, g_escape_header_texture);

		bt_draw_iface_btn(g_escape_b1, false, g_sfx_iface_hover);
		bt_draw_iface_btn(g_escape_b2, false, g_sfx_iface_hover);
		bt_draw_iface_btn(g_escape_b3, false, g_sfx_iface_hover);

		if (bt_chk_rel_iface_btn(g_escape_b1, NULL) == true){
			em_toggle_escape_menu();
		} else if (bt_chk_rel_iface_btn(g_escape_b2, g_sfx_select) == true){
			puts("Full screen must be implemented");	
		} else if (bt_chk_rel_iface_btn(g_escape_b3, g_sfx_select) == true){
			set_quit_game();
		}
	}	
	return;
}*/

/* Function: sb_render_escape_menu
 * -----------------------------------------------------------------------------
 * This function is called in all the stages, it will draw the escape menu 
 * according to the state of the show_menu variable. Input/interaction checks
 * must be handled in the corresponding update function.
 * 
 * Arguments:
 *	show_menu: The state of the show menu variable.
 *
 * Return:
 *	void
 */
void em_render_escape_menu(bool show_menu)
{
	if (show_menu == true){
		
		SDL_Rect r = dw_get_iface_big_center_box();
		dw_draw_iface_box(r, g_escape_header_texture);

		bt_draw_iface_btn(g_escape_b1, false, g_sfx_iface_hover);
		bt_draw_iface_btn(g_escape_b2, false, g_sfx_iface_hover);
		bt_draw_iface_btn(g_escape_b3, false, g_sfx_iface_hover);
	}	
	return;
}

/* Function: em_update_escape_menu
 * -----------------------------------------------------------------------------
 * This function is called in all the stages, it will update the escape menu 
 * according to the state of the g_escape_menu variable and handle input
 * interactions for the menu buttons.
 * 
 * Arguments:
 *	menu_variable_state: The state of the show menu variable.
 *
 * Return:
 *	void
 */
int em_update_escape_menu(bool show_menu)
{
	int press_btn_state = ESC_MENU_NONE;
	
if (show_menu == true){
		
		if (bt_chk_rel_iface_btn(g_escape_b1, NULL) == true){
			em_toggle_escape_menu();
		} else if (bt_chk_rel_iface_btn(g_escape_b2, g_sfx_select) == true){
			puts("Full screen must be implemented");	
		} else if (bt_chk_rel_iface_btn(g_escape_b3, g_sfx_select) == true){
			press_btn_state = ESC_MENU_EXIT;
		}
	}	
	return press_btn_state;
}


