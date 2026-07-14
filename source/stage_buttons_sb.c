#include <stdlib.h>
#include <stdio.h>
#include "stage_buttons_sb.h"
#include "ui/button_bt.h"
#include "ui/escape_menu_em.h"
#include "ui/reset_menu_rm.h"
#include "dimensions_dm.h"
#include "code_window_cw.h"
#include "assert.h"
#include "sdl_config.h"
#include "aux.h"

static Uint32 STAGE_BUTTONS_MOVEMENT_DELTA = 10;
static Uint32 RET_BUTTON_W = 90;
static Uint32 RET_BUTTON_H = 75;
static Uint32 RST_BTN_W = 90;
static Uint32 RST_BTN_H = 75;

static Uint32 STAGE_BUTTON_W = 60;
static Uint32 STAGE_BUTTON_H = 60;
static Uint32 STAGE_BUTTON_Y = 820;

static Uint32 SCREEN_BORDERS_OFS = 5;

bool g_step_btns_avail = true;

texture_t *stop_button = NULL;
texture_t *fast_button = NULL;
texture_t *play_button = NULL;
texture_t *step_forward_button = NULL;
texture_t *return_button = NULL;
texture_t *reset_button = NULL;

iface_btn_t *stop;
iface_btn_t *fast;
iface_btn_t *play;
iface_btn_t *step;
iface_btn_t *ret_btn;
iface_btn_t *rst_btn;

static SDL_Rect get_return_button_box();
static SDL_Rect get_rst_btn_box();
static int get_ofs_space_stage_buttons();
static SDL_Rect get_rst_b1_box();
static SDL_Rect get_rst_b2_box();

/* Function: sb_get_escape_state
 * ----------------------------------------------------------------------------
 * This function return the boolean state of the state.
 *
 * Arguments:
 * 	None.
 *
 * Return:
 *	Boolean with the state fo the escape_menu variable
 */
bool sb_chk_rst_esc_menu_active()
{
	bool active = (rm_chk_rst_menu_state() | em_get_escape_state());
	return active;
}

/* Function: dm_get_ofs_space_stage_buttons
 * -----------------------------------------------------------------------------
 *	Return the offset value of the contents of the buffer. 
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset
 */
static int get_ofs_space_stage_buttons()
{
	SDL_Rect sb = sb_get_stage_btns();
	return sb.w/2;
}

/* Function: sb_get_stage_btns
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object, x and y are initialize at 0
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect sb_get_stage_btns()
{
	SDL_Rect b;
	SDL_Rect ib = dm_get_stage_instruction_box();
	int sh = dm_get_screen_height();
	int shadow = bt_get_ofs_button_shadow();
	b.w = dm_scale_to_res(STAGE_BUTTON_W);
	b.h = dm_scale_to_res(STAGE_BUTTON_H);
	b.x = ib.x + ib.w;
	b.y = sh - b.h - 2*shadow;
	return b;
}

/* Function: dm_get_rst_btn_box
 * -----------------------------------------------------------------------------
 * Returns the box of the reset button of the stage
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
static SDL_Rect get_rst_btn_box()
{
	SDL_Rect ret_box = get_return_button_box();
	SDL_Rect b;
	b.w = dm_scale_to_res(RST_BTN_W);
	b.h = dm_scale_to_res(RST_BTN_H);
	b.x = ret_box.x;
	b.y = ret_box.y - b.h - 3*bt_get_ofs_button_shadow();	
	return b;
}



/* Function: get_return_button_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object, x and y are initialize at 0
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
static SDL_Rect get_return_button_box()
{
	int screen_height = dm_get_screen_height();
	SDL_Rect b;
	b.w = dm_scale_to_res(RET_BUTTON_W);
	b.h = dm_scale_to_res(RET_BUTTON_H);
	b.x = dm_scale_to_res(SCREEN_BORDERS_OFS);
	b.y = screen_height - b.h - 2*bt_get_ofs_button_shadow();
	return b;
}

/* Function: sb_set_step_btns_avail
 * ----------------------------------------------------------------------------
 * Sets the global variable that determines if the step buttons will be shown
 * as a part of a level.
 *
 * Arguments:
 * 	state: state that the g_step_btns_avail variable will be set
 *
 * Return:
 *	Void.	
 */
void sb_set_step_btns_avail(bool state)
{
	g_step_btns_avail = state;
}

/* Function: sb_get_sb_rect
 * -----------------------------------------------------------------------------
 * Return the rectangle element of a stage button member
 *
 * Arguments:
 * member: The specific member that is being requested.
 *
 * Return:
 *	The rect of the requested member.
 */
SDL_Rect sb_get_sb_rect(int member)
{
	SDL_Rect r;
	switch (member){
		case STOP:
			r = stop->r;
			break;
		case STEP:
			r = step->r;
			break;
		case PLAY:
			r = play->r;
			break;
		case FAST:
			r = fast->r;
			break;
	}
	return r;
}


/* Function: sb_get_play_button_member
 * -----------------------------------------------------------------------------
 * This function returns the solicited button value from the play button
 *
 * Arguments:
 * member: The specific member that is being requested.
 *
 * Return:
 *	The accessed member.
 */
int sb_get_play_button_member(int member)
{
	assert(member > MEMBER_MIN && member < MEMBER_MAX && "Invalid member");

	int return_value;
	switch (member){
		case MEMBER_X:
			return_value = play->r.x;
			break;
		case MEMBER_Y:
			return_value = play->r.y;
			break;
		case MEMBER_W:
			return_value = play->r.w;
			break;
		case MEMBER_H:
			return_value = play->r.h;
			break;
	}
	return return_value;
}

static bool sb_check_released_in_stage_button(); // not used

/* Function: adjust_stage_buttons_position
 * -------------------------------------
 * Arguments:
 * 	status:
 *
 * Return:
 *	Void.
 */
void adjust_stage_buttons_position(int code_size)
{
	int hidden_y = dm_get_y_hidden_stage_buttons();
	SDL_Rect sb = sb_get_stage_btns();
	int y_final;
	if (code_size == CW_EMPTY){
		y_final = hidden_y;
	} else {
		y_final = sb.y;
	}

	int delta = STAGE_BUTTONS_MOVEMENT_DELTA;

	if (stop->r.y < y_final){
		stop->r.y += delta;
		fast->r.y += delta;
		play->r.y += delta;
		step->r.y += delta;
	} else if (stop->r.y > y_final){
		stop->r.y -= delta;
		fast->r.y -= delta;
		play->r.y -= delta;
		step->r.y -= delta;
	}

}

/* Function: sb_draw_stage_btns
 * -------------------------------------
 * Arguments:
 * 	None.
 *
 * Return:
 *	Void.
 */
void sb_draw_stage_btns(int code_size)
{
	adjust_stage_buttons_position(code_size);
	bt_draw_iface_btn(stop, sb_chk_rst_esc_menu_active(), g_sfx_iface_hover);
	bt_draw_iface_btn(play, sb_chk_rst_esc_menu_active(), g_sfx_iface_hover);
	if (g_step_btns_avail == true){
		bt_draw_iface_btn(fast, sb_chk_rst_esc_menu_active(), g_sfx_iface_hover);
		bt_draw_iface_btn(step, sb_chk_rst_esc_menu_active(), g_sfx_iface_hover);
	}
}

/* Function: sb_draw_rst_btn
 * ----------------------------------------------------------------------------
 * This function draws the reset button that will be shown to clean the code
 *
 * Arguments:
 * 	None.
 *
 * Return:
 *	Void.
 */
void sb_draw_rst_btn()
{
	bt_draw_iface_btn(rst_btn, sb_chk_rst_esc_menu_active(), g_sfx_iface_hover);
}

/* Function: sb_init_rst_btn
 * -----------------------------------------------------------------------------
 * This function initialises the reset button of the stage.
 *
 * Arguments:
 * 	None.
 *
 * Return:
 *	Void.
 */
void sb_init_rst_btn()
{
	if (rst_btn == NULL){
		SDL_Rect r = get_rst_btn_box();
		rst_btn = malloc(sizeof(iface_btn_t));
		rst_btn = bt_create_iface_btn(r, reset_button, true);
	}
	return;
}

/* Function: sb_chk_hov_rst_ret_btns
 * -----------------------------------------------------------------------------
 * Verifies if the player is above the rst or ret bths
 *
 * Arguments:
 * 	None.
 *
 * Return:
 *	true if inside that area
 */
bool sb_chk_hov_rst_ret_btns()
{
	int hov = false;

	if (bt_chk_hover_iface_btn(ret_btn) == true) {
		hov = true;
	} else if (bt_chk_hover_iface_btn(rst_btn) == true) {
		hov = true;
	}
	return hov;
}

/* Function: sb_chk_rel_rst_btn
 * -----------------------------------------------------------------------------
 * Verifies if the reset button has been clicked
 *
 * Arguments:
 * 	None.
 *
 * Return:
 *	true if button clicked, false if otherwise.
 */
bool sb_chk_rel_rst_btn()
{
	int rst = false;

	if(sb_chk_rst_esc_menu_active() == false){
		if (bt_chk_rel_iface_btn(rst_btn, g_sfx_menu) == true) {
			rst = true;
		}
	}
	return rst;
}

/* Function: sb_draw_ret_btn
 * ----------------------------------------------------------------------------
 * This function draws the return button that will be shown on several displays
 *
 * Arguments:
 * 	None.
 *
 * Return:
 *	Void.
 */
void sb_draw_ret_btn()
{
	bt_draw_iface_btn(ret_btn, sb_chk_rst_esc_menu_active(), g_sfx_iface_hover);
}

/* Function: sb_init_ret_btns
 * -----------------------------------------------------------------------------
 * This function initialises the return button that is used and present in 
 * several stages.
 *
 * Arguments:
 * 	None.
 *
 * Return:
 *	Void.
 */
void sb_init_ret_btn()
{
	if (ret_btn == NULL){
		SDL_Rect r = get_return_button_box();
		ret_btn = malloc(sizeof(iface_btn_t));
		ret_btn = bt_create_iface_btn(r, return_button, true);
	}
	return;
}

/* Function: sb_init_stage_btns
 * -----------------------------------------------------------------------------
 * Arguments:
 * 	None.
 *
 * Return:
 *	Void.
 */
void sb_init_stage_btns()
{
	SDL_Rect sb = sb_get_stage_btns();
	int hidden_y = dm_get_y_hidden_stage_buttons();
	SDL_Rect r0 = {.x = sb.x, .y = hidden_y, .w = sb.w, .h = sb.h};
	
	stop = malloc(sizeof(iface_btn_t));
	stop = bt_create_iface_btn(r0, stop_button, true);

	int space =  get_ofs_space_stage_buttons();
	SDL_Rect r1 = {.x = sb.x + sb.w + space,
				   .y = hidden_y, 
				   .w = sb.w, 
				   .h = sb.h};

	step = malloc(sizeof(iface_btn_t));
	step = bt_create_iface_btn(r1, step_forward_button, true);
	
	SDL_Rect r2 = {.x = sb.x + 2*sb.w + 2*space, 
			       .y = hidden_y, 
				   .w = sb.w, 
				   .h = sb.h};

	play = malloc(sizeof(iface_btn_t));
	play = bt_create_iface_btn(r2, play_button, true);
	
	SDL_Rect r3 = {.x = sb.x + 3*sb.w + 3*space, 
				   .y = hidden_y, 
				   .w = sb.w, 
				   .h = sb.h};
	
	fast = malloc(sizeof(iface_btn_t));
	fast = bt_create_iface_btn(r3, fast_button, true);
	
	return;
}

/* Function: sb_chck_rel_ret_btn
 * -----------------------------------------------------------------------------
 * This function verifies if the player has clicked on the return button
 *
 * Arguments:
 * 	None.
 *
 * Return:
 *	true if button clicked, false if otherwise.
 */
bool sb_chck_rel_ret_btn()
{
	int ret = false;
	
	if(sb_chk_rst_esc_menu_active() == false){
		if (bt_chk_rel_iface_btn(ret_btn, g_sfx_iface_back_cancel) == true) {
			ret = true;
		}
	}
	return ret;
}

/* Function: sb_chk_click_stage_btn
 * ----------------------------------------------------------------------------
 * Verifies if a stage button was clicked
 *
 * Arguments:
 * 	None.
 *
 * Return:
 *	true if button clicked, false if otherwise.
 */
bool sb_chk_click_stage_btn()
{
	int ret = false;
	if (sb_chk_rst_esc_menu_active() == false){
		if (bt_chk_mouse_click_iface_btn(stop) == true ||
			bt_chk_mouse_click_iface_btn(play) == true){
			ret = true;
		}
		else if (g_step_btns_avail == true && 
				 (bt_chk_mouse_click_iface_btn(fast) == true ||
				  bt_chk_mouse_click_iface_btn(step) == true)){
			ret = true;	
		}
	}
	return ret;
}

/* Function: identify_clicked_stage_button
 * -------------------------------------
 * Arguments:
 * 	None.
 *
 * Return:
 *	true if released in stage button, false if otherwise.
 */
int identify_clicked_stage_button()
{
	int clicked_button = INVALID;
	
	if (bt_chk_mouse_click_iface_btn(stop) == true){
		clicked_button = STOP;
	} else if (bt_chk_mouse_click_iface_btn(fast) == true && 
			   g_step_btns_avail == true){
		clicked_button = FAST;
	} else if (bt_chk_mouse_click_iface_btn(play) == true){
		clicked_button = PLAY;
	} else if (bt_chk_mouse_click_iface_btn(step) == true &&
			   g_step_btns_avail == true){
		clicked_button = STEP;
	}
	return clicked_button;
}


