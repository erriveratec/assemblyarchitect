#include<stdio.h>
#include<stdlib.h>
#include"dimensions_dm.h"
#include"registers_rg.h"
#include"code_window_cw.h"
#include"aux.h"
#include"draw_dw.h"
#include <SDL.h>


#define RES_BOX_W 450
#define RES_BOX_H 270
#define RES_BOX_OFFSET 35

#define TEXT_BOX_LOWER_X 675
#define TEXT_BOX_LOWER_Y 565

#define MSG_BOX_W 270
#define MSG_BOX_H 270

// This is a new version of the dimensions set by a ration of the resolution

#define VALUE_BOX_H 40
#define VALUE_BOX_W 50
#define VAL_BOX_XOFS 10
#define REG_BOX_OFS 25
#define BET_REG_OFS 5

#define ARROW_H 30
#define ARROW_W 30

#define AVATAR_W 50
#define AVATAR_H 50

#define TEXT_H_BIG 50
#define TEXT_H_MSG 35

#define TEXT_H_ERROR_MSG 35
#define TEXT_H_TOTAL_MSG 160
#define TEXT_H_STAGE_ELEMENTS_TITLES 25
#define TEXT_H_CODE 40

#define TEXT_H_STAGE_TITLE 110
#define TEXT_H_STAGE_SUBTITLE 55
#define TEXT_H_STAGE_SUBSUBTITLE_H 40

#define P_NAME_OFS 80
#define P_NAME_H 50
#define P_LORE_OFS 22
#define P_DARK_PLATE_OFS 6

#define CODE_BUTTON_W 90
#define CODE_BUTTON_H 40
#define COMMA_OFS 15

#define RST_MENU_BUTTON_W 350
#define RST_MENU_BUTTON_H 60

#define RET_RES_OFS 5


#define RES_BUTTON_H 60
#define RES_BUTTON_W 155

#define RAIL_W 4
#define RAIL_END_W 16

// Padding of the text borders
#define W_PADDING 12
#define H_PADDING 10
#define BUT_PADDING 25


#define BORDERS_WIDTH 5 //Used for the interface boxes

// Intro screens
#define TITLE_H	140
#define TITLE_Y	140

#define SEL_PLAYER_Y 155

#define PRESS_H 60
#define PRESS_Y 870

#define STUDIO_W 600
#define STUDIO_H 600

#define INS_BOX_X 0
#define INS_BOX_W 170
#define INS_BOX_H 225
#define INS_BOX_Y 225


#define REG_BOX_W 250
#define REG_BOX_H 400
#define REG_BOX_Y 0

#define IMM_TXT_UP_Y 0

#define BTN_ANIM_MAX 15
#define BTN_ANIM_DELTA 1
#define BTN_ANIM_SPEED 1 //Lower is faster

#define MISS_OP_W 3

#define BUF_X 1400
#define BUF_H 75
#define BUF_W 1000




int g_res_id;
int g_screen_width;
int g_screen_height;

static int dm_get_ofs_val_box_x();
// dm_get_box_msg_wh TODO remove  dw_get_ofs_iface_filled_border(

/* Function: dm_res_box_offset
 * -----------------------------------------------------------------------------
 * Returns the width that is going to be used for the missing op animation
 * square.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	The max level of the animation
 */
int dm_get_ofs_res_box()
{
	return dm_scale_to_res(RES_BOX_OFFSET);
}



/* Function: dm_get_w_miss_op
 * -----------------------------------------------------------------------------
 * Returns the width that is going to be used for the missing op animation
 * square.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	The max level of the animation
 */
int dm_get_w_miss_op()
{
	return dm_scale_to_res(MISS_OP_W);
}

/* Function: dm_get_btn_anim_max
 * -----------------------------------------------------------------------------
 *	Return the max level of the button animation
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	The max level of the animation
 */
int dm_get_btn_anim_max()
{
	return dm_scale_to_res(BTN_ANIM_MAX);
}

/* Function: dm_get_btn_anim_delta
 * -----------------------------------------------------------------------------
 *	Return the delta change of the button animation
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	The delta level  of the animation
 */
int dm_get_btn_anim_delta()
{
	return dm_scale_to_res(BTN_ANIM_DELTA);
}

/* Function: dm_get_btn_anim_speed
 * -----------------------------------------------------------------------------
 * Returns the speed level of the animation 
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	The speed level of the animation
 */
int dm_get_btn_anim_speed()
{
	return BTN_ANIM_SPEED;
}


/* Function: dm_get_h_between_code
 * -----------------------------------------------------------------------------
 *	Returns the ammount of space that will be between instructions
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	Instructions sapes
 */
int dm_get_h_between_code()
{
	return dm_get_code_button_wh().h + dm_get_h_padding();
}

/* Function: dm_get_w_borders
 * -----------------------------------------------------------------------------
 *	Returns the width of the borders of each section of the game
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	The of the borders 
 */
int dm_get_w_borders()
{
	return dm_scale_to_res(BORDERS_WIDTH);
}


/* Function: dm_get_screen_height
 * -----------------------------------------------------------------------------
 *	Returns the screen width
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	screen width
 */
int dm_get_screen_height()
{
	return g_screen_height;
}

/* Function: dm_get_screen_width
 * -----------------------------------------------------------------------------
 *	Returns the screen width
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	screen width
 */
int dm_get_screen_width()
{
	return g_screen_width;
}

/* Function: dm_get_ofs_reg_value_box
 * -----------------------------------------------------------------------------
 *	Return the offset value of the contents of the buffer. 
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset
 */
int dm_get_ofs_reg_value_box()
{
	SDL_Rect vbox = dm_get_value_box_wh();
	return vbox.h/4;
}

/* Function: dm_get_ofs_buffer_value_box
 * -----------------------------------------------------------------------------
 *	Return the offset value of the contents of the buffer. 
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset
 */
int dm_get_ofs_between_value_box()
{
	int ofs =  dm_get_ofs_buffer_value_box();
	return (ofs*3/2);
}

/* Function: dm_get_ofs_buffer_value_box
 * -----------------------------------------------------------------------------
 *	Return the offset value of the contents of the buffer. 
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset
 */
int dm_get_ofs_buffer_value_box()
{
	SDL_Rect vbox =  dm_get_value_box_wh();
	SDL_Rect ib = dm_get_stage_input_buffer_box();
	int ofs = (ib.h - vbox.h)/2;
	return ofs;
}

/* Function: dm_get_w_code_box_text
 * -----------------------------------------------------------------------------
 *	Return the offset value of the contents of the buffer. 
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset
 */
int dm_get_w_code_box_text()
{
	int ofs = dm_get_w_border_padding();
	SDL_Rect cb = cw_get_stage_code_box();
	int w = cb.w - 2*ofs;
	return w;
}


/* Function: dm_get_w_border_padding
 * -----------------------------------------------------------------------------
 *	Return the value of the padding
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset
 */
int dm_get_w_border_padding()
{
	return dm_scale_to_res(W_PADDING) + dm_get_w_borders();
}

/* Function: dm_get_h_border_padding
 * -----------------------------------------------------------------------------
 *	Returns the scale value for the vertical padding
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset
 */
int dm_get_h_border_padding()
{
	return dm_scale_to_res(H_PADDING) + dm_get_w_borders();
}

/* Function: dm_get_w_padding
 * -----------------------------------------------------------------------------
 *	Return the value of the padding
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset
 */
int dm_get_w_padding()
{
	return dm_scale_to_res(W_PADDING);
}

/* Function: dm_get_h_padding
 * -----------------------------------------------------------------------------
 *	Returns the scale value for the vertical padding
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset
 */
int dm_get_h_padding()
{
	return dm_scale_to_res(H_PADDING);
}


/* Function: dm_get_h_stage_titles
 * -----------------------------------------------------------------------------
 *	Return the offset value of the contents of the buffer. 
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset
 */
int dm_get_h_stage_titles()
{
	int h = dm_scale_to_res(TEXT_H_STAGE_TITLE);
	return h;
}

/* Function: dm_get_h_stage_subtitle
 * -----------------------------------------------------------------------------
 *	Return the offset value of the contents of the buffer. 
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset
 */
int dm_get_h_stage_subtitle()
{
	int h = dm_scale_to_res(TEXT_H_STAGE_SUBTITLE);
	return h;
}
/* Function: dm_get_h_stage_titles
 * -----------------------------------------------------------------------------
 *	Return the offset value of the contents of the buffer. 
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset
 */
int dm_get_h_stage_elements_titles()
{
	int h = dm_scale_to_res(TEXT_H_STAGE_ELEMENTS_TITLES);
	return h;
	
}

/* Function: dm_get_ofs_comma
 * -----------------------------------------------------------------------------
 *	Return the offset value for the comma that is draw in instructions
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset
 */
int dm_get_ofs_code_comma()
{
	int ofs = dm_get_ofs_code_op2() - dm_scale_to_res(COMMA_OFS);
	return ofs;
}

/* Function: dm_get_ofs_code_op1
 * -----------------------------------------------------------------------------
 *	Return the offset value of the contents of the buffer. 
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset
 */
int dm_get_ofs_code_op1()
{
	SDL_Rect cb = dm_get_code_button_wh();
	int ofs = cb.w;
	return ofs;
}

/* Function: dm_get_ofs_code_op2
 * -----------------------------------------------------------------------------
 *	Return the offset value of the contents of the buffer. 
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset
 */
int dm_get_ofs_code_op2()
{
	SDL_Rect cb = dm_get_code_button_wh();
	int ofs = 2*cb.w + dm_get_w_padding();
	return ofs;
}

/* Function: dm_get_ofs_bet_regs
 * -----------------------------------------------------------------------------
 *	Return the offset value of the contents of the buffer. 
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset
 */
int dm_get_ofs_bet_regs()
{
	int offset = dm_scale_to_res(BET_REG_OFS);
	return offset;
	
}

/* Function: dm_get_ofs_val_box_x
 * -----------------------------------------------------------------------------
 *	Return the offset value of the contents of the buffer. 
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset
 */
static int dm_get_ofs_val_box_x()
{
	int offset = dm_scale_to_res(VAL_BOX_XOFS);
	return offset;
	
}

/* Function: dm_get_ofs_reg_box
 * -----------------------------------------------------------------------------
 *	Return the offset value of the contents of the buffer. 
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset
 */
int dm_get_ofs_stage_reg_box()
{
	int offset = dm_scale_to_res(REG_BOX_OFS);
	return offset;
	
}

/* Function: dm_get_y_hidden_stage_buttons
 * -----------------------------------------------------------------------------
 * Returns the hidden y value for the stage buttons
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset for the sel level buttons
 */
int dm_get_y_hidden_stage_buttons()
{
	return g_screen_height + 150;
	
}

/* Function: dm_get_w_stage_rail
 * -----------------------------------------------------------------------------
 * Returns the w value of the stage rail
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset for the sel level buttons
 */
int dm_get_w_stage_rail()
{
	int w = dm_scale_to_res(RAIL_W);
	return w;
	
}

/* Function: dm_get_w_stage_rail_end
 * -----------------------------------------------------------------------------
 * Returns the w value for the ends of the stage raisl
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset for the sel level buttons
 */
int dm_get_w_stage_rail_end()
{
	int w = dm_scale_to_res(RAIL_END_W);
	return w;
	
}
/* Function: dm_get_value_box_val_wh
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object. 
 * 
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_value_box_val_wh()
{
	SDL_Rect vb = dm_get_value_box_wh();
	SDL_Rect b;
	b.w = vb.w;
	b.h = vb.h - vb.h/10;
	b.x = 0;
	b.y = 0;
	return b;
}
/* Function: dm_get_value_box_wh
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object. 
 * 
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_value_box_wh()
{
	SDL_Rect b;
	b.w = dm_scale_to_res(VALUE_BOX_W);
	b.h = dm_scale_to_res(VALUE_BOX_H);
	b.x = 0;
	b.y = 0;
	return b;
}

/* Function: dm_get_arrow_wh
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object. 
 * 
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_arrow_wh()
{
	SDL_Rect b;
	b.w = dm_scale_to_res(ARROW_W);
	b.h = dm_scale_to_res(ARROW_H);
	b.x = 0;
	b.y = 0;
	return b;
}

/* Function: dm_get_text_box_result_but3
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object. This is for the text of the
 * result box.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_text_box_result_but3()
{
	int cont_w = dm_scale_to_res(RES_BUTTON_W);
	int offset = dm_scale_to_res(RES_BOX_OFFSET);
	SDL_Rect rb = dw_get_iface_big_lower_box();
	SDL_Rect b;
	b.w = dm_scale_to_res(RES_BUTTON_W);
	b.h = dm_scale_to_res(RES_BUTTON_H);
	b.x = rb.x + (rb.w - b.w)/2;
	b.y = rb.y + rb.h - b.h - offset;
	return b;
}

/* Function: dm_get_text_box_result_but2
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object. This is for the text of the
 * result box.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_text_box_result_but2()
{
	int back_w = dm_scale_to_res(RES_BUTTON_W);
	int cont_w = dm_scale_to_res(RES_BUTTON_W);
	int offset = dm_scale_to_res(RES_BOX_OFFSET);
	SDL_Rect rb = dw_get_iface_big_lower_box();
	SDL_Rect cb = dw_get_iface_content_box(dw_get_iface_big_lower_box());
	SDL_Rect b;
	b.w = dm_scale_to_res(RES_BUTTON_W);
	b.h = dm_scale_to_res(RES_BUTTON_H);
	b.x = cb.x + back_w + 2*(cb.w - (back_w + cont_w))/3 ;
	b.y = rb.y + rb.h - b.h - offset;
	return b;
}

/* Function: dm_get_text_box_result_but1
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object. This is for the text of the
 * result box.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_text_box_result_but1()
{
	int cont_w = dm_scale_to_res(RES_BUTTON_W);
	int offset = dm_scale_to_res(RES_BOX_OFFSET);
	SDL_Rect rb = dw_get_iface_big_lower_box();
	SDL_Rect cb = dw_get_iface_content_box(dw_get_iface_big_lower_box());
	SDL_Rect b;
	b.w = dm_scale_to_res(RES_BUTTON_W);
	b.h = dm_scale_to_res(RES_BUTTON_H);
	b.x = cb.x + (cb.w - (b.w + cont_w))/3;
	b.y = rb.y + rb.h - b.h - offset;
	return b;
}



/* Function: dm_get_box_msg_wh
 * -----------------------------------------------------------------------------
 * Returns the box dimensions of the box message
 * SI TODO SALE BIEN ESTO TIENE QUE DEJAR DE EXISTIR
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_box_msg_wh()
{
	SDL_Rect b;
	b.w = dm_scale_to_res(MSG_BOX_W);
	b.h = dm_scale_to_res(MSG_BOX_H);
	b.x = 0;
	b.y = 0;
	return b;
}//MUST BE REMOVED

/* Function: dm_get_text_box_error
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_text_box_error()
{
	SDL_Rect rb = dw_get_iface_big_lower_box();
	SDL_Rect mb = dm_get_box_msg_wh();
	SDL_Rect ab = dm_get_arrow_wh();
	
	SDL_Rect b;
	b.w = rb.w*2/3;
	b.h = mb.h;
	b.x = rb.x + (rb.w/3)/2;
	b.y = rb.y + rb.h + 2*ab.h;
	return b;
}





/* Function: dm_get_text_box_stagebutton
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_text_box_stagebutton()
{
	SDL_Rect cb = cw_get_stage_code_box();	
	SDL_Rect mb = dm_get_box_msg_wh();	
	SDL_Rect b;
	b.w = mb.w;
	b.h = mb.h;
	b.x = cb.x + (cb.w - mb.w)/2;
	b.y = (cb.y + cb.h) - mb.h/2;
	return b;
}




/* Function: dm_get_text_box_upper_right
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_text_box_upper_right()
{
	SDL_Rect mb = dm_get_box_msg_wh();	
	SDL_Rect b;
	b.w = mb.w;
	b.h = mb.h;
	b.x = dm_get_stage_imm_up().x + 12*dm_get_value_box_wh().w;
	b.y = dw_get_ofs_iface_filled_border();
	return b;
}

/* Function: dm_get_stage_ib_text_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_stage_ib_text_box()
{
	SDL_Rect ib = dm_get_stage_input_buffer_box();
	SDL_Rect b;
	b.w = 0;
	b.h = dm_get_h_code_text();
	b.x = ib.x;
	b.y = 0;
	return b;
}

/* Function: dm_get_stage_ob_text_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_stage_ob_text_box()
{
	SDL_Rect ob = dm_get_stage_output_buffer_box();
	SDL_Rect b;
	b.w = 0;
	b.h = dm_get_h_code_text();
	b.x = ob.x;
	b.y = g_screen_height - b.h;
	return b;
}

/* Function: dm_get_stage_input_buffer_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object, x and y are initialize at 0
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_stage_input_buffer_box()
{
	SDL_Rect b;
	b.w = dm_scale_to_res(BUF_W);
	b.h = dm_scale_to_res(BUF_H);
	b.x = dm_scale_to_res(BUF_X);
	b.y = dm_get_h_code_text();

	return b;
}

/* Function: dm_get_stage_output_buffer_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object, x and y are initialize at 0
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_stage_output_buffer_box()
{
	SDL_Rect ib = dm_get_stage_ib_text_box();
	SDL_Rect b;
	b.w = dm_scale_to_res(BUF_W);
	b.h = dm_scale_to_res(BUF_H);
	b.x = dm_scale_to_res(BUF_X);
	b.y = g_screen_height - ib.h - b.h;
	return b;
}

/* Function: dm_get_stage_instruction_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the instruction box, x and y are initialize 
 * at 0. Instruction box is where the player selects instructions from.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_stage_instruction_box()
{
	SDL_Rect b;
	b.w = dm_scale_to_res(INS_BOX_W);
	b.h = dm_scale_to_res(INS_BOX_H);
	b.x = dm_scale_to_res(INS_BOX_X);
	b.y = dm_scale_to_res(INS_BOX_Y);
	return b;
}



/* Function: dm_get_stage_ibox
 * -----------------------------------------------------------------------------
 * Returns the box for the object
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_stage_ibox()
{
	SDL_Rect vb = dm_get_value_box_wh();
	SDL_Rect rb = rg_get_register_box();
	SDL_Rect rbi = dm_get_stage_reg_box();
	SDL_Rect b;
	b.w = vb.w;
	b.h = vb.h;
	b.x = rbi.x + rbi.w - b.w - b.w/5;
	b.y = rb.y - b.h - b.h/4;
	return b;
}

/* Function: dm_get_stage_obox
 * -----------------------------------------------------------------------------
 * Returns the box for the object
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_stage_obox()
{
	SDL_Rect vb = dm_get_value_box_wh();
	SDL_Rect rb = rg_get_register_box();
	SDL_Rect rbi = dm_get_stage_reg_box();
	SDL_Rect b;
	b.w = vb.w;
	b.h = vb.h;
	b.x = rbi.x + rbi.w - b.w - b.w/5;
	b.y = rb.y + rb.h + b.h/4;
	return b;
}

/* Function: dm_get_stage_zfbox
 * -----------------------------------------------------------------------------
 * Returns the box for the object
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_stage_zfbox()
{
	SDL_Rect vb = dm_get_value_box_wh();
	SDL_Rect rbi = dm_get_stage_reg_box();
	SDL_Rect b;
	b.w = vb.w;
	b.h = vb.h;
	b.x = rbi.x + rbi.w + dm_get_ofs_val_box_x();
	b.y = rg_get_reg_box_y_pos(0);
	return b;
}

/* Function: dm_get_stage_reg_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object, x and y are initialize at 0
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_stage_reg_box()
{
	SDL_Rect cb = cw_get_stage_code_box();
	SDL_Rect b;
	b.w = dm_scale_to_res(REG_BOX_W);
	b.h = dm_scale_to_res(REG_BOX_H);
	b.x = cb.x + cb.w;
	b.y = dm_scale_to_res(REG_BOX_Y);
	return b;
}


/* Function: dm_get_h_msg
 * -----------------------------------------------------------------------------
 * Returns the h value for the click anywhere message
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset for the sel level buttons
 */
int dm_get_h_msg()
{
	int h = dm_scale_to_res(TEXT_H_MSG);
	return h;
	
}
/* Function: dm_get_h_line_number
 * -----------------------------------------------------------------------------
 * Returns the h value for the line numberj
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the height for the line number
 */
int dm_get_h_code_text()
{
	int h = dm_scale_to_res(TEXT_H_CODE);
	return h;
}
/* Function: dm_get_big_msg_h
 * -----------------------------------------------------------------------------
 * Returns the h value for the big messages
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset for the sel level buttons
 */
int dm_get_h_big_text()
{
	int h = dm_scale_to_res(TEXT_H_BIG);
	return h;
}



/* Function: dm_get_upper_title_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object.
 */
SDL_Rect dm_get_upper_title_box(char *msg)
{
	SDL_Rect b;
	b.h = dm_scale_to_res(TEXT_H_STAGE_TITLE);
	b.w = get_text_width_fits_height(b.h, msg);
	b.x = g_screen_width/2 - b.w/2;
	b.y = dm_scale_to_res(SEL_PLAYER_Y);

	return b;
}

/* Function: dm_get_avatar_wh
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object, x and y are initialize at 0
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_avatar_wh()
{
	SDL_Rect b;
	b.w = dm_scale_to_res(AVATAR_W);
	b.h = dm_scale_to_res(AVATAR_H);
	b.x = 0;
	b.y = 0;
	return b;
}



/* Function: dm_get_stage_imm_up
 * -----------------------------------------------------------------------------
 * Returns the coordinates for the stage immediates for the upper part
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_stage_imm_up()
{
	SDL_Rect b;
	SDL_Rect cb = cw_get_stage_code_box();
	SDL_Rect vb = dm_get_value_box_wh();
	b.w = 0;
	b.h = dm_get_h_stage_elements_titles();
	b.x = dm_get_stage_ibox().x;
	b.y = dm_scale_to_res(IMM_TXT_UP_Y);
	return b;
}

/* Function: dm_get_code_button_wh
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object, x and y are initialize at 0
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_code_button_wh()
{
	SDL_Rect b;
	b.w = dm_scale_to_res(CODE_BUTTON_W);
	b.h = dm_scale_to_res(CODE_BUTTON_H);
	b.x = 0;
	b.y = 0;
	return b;
}




/* Function: dm_get_ofs_player_name
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset value.
 */
int dm_get_ofs_player_name()
{
	return dm_scale_to_res(P_NAME_OFS);
}

/* Function: dm_get_ofs_player_lore
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset value.
 */
int dm_get_ofs_player_lore()
{
	return dm_scale_to_res(P_LORE_OFS);
}

/* Function: dm_get_ofs_player_dark_plate
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset value.
 */
int dm_get_ofs_player_dark_plate()
{
	return dm_scale_to_res(P_DARK_PLATE_OFS);
}


/* Function: dm_get_h_player_name
 * -----------------------------------------------------------------------------
 * Returns the text height of the player name.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the height of the player name.
 */
int dm_get_h_player_name()
{
	return dm_scale_to_res(P_NAME_H);
}

/* Function: dm_get_h_stage_subsubtitle
 * -----------------------------------------------------------------------------
 * Returns the text height of the player name.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the height of the player name.
 */
int dm_get_h_stage_subsubtitle()
{
	return dm_scale_to_res(TEXT_H_STAGE_SUBSUBTITLE_H);
}
/* Function: dm_get_press_space_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object.
 */
SDL_Rect dm_get_press_space_box(char *msg)
{
	SDL_Rect b;
	b.h = dm_scale_to_res(PRESS_H);
	b.w = get_text_width_fits_height(b.h, msg);
	b.x = g_screen_width/2 - b.w/2;
	b.y = dm_scale_to_res(PRESS_Y);
	return b;
}

/* Function: dm_get_studio_name_msg_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object.
 */
SDL_Rect dm_get_studio_name_msg_box()
{
	SDL_Rect b;
	b.w = dm_scale_to_res(STUDIO_W);
	b.h = dm_scale_to_res(STUDIO_H);
	b.x = 0;
	b.y = (g_screen_height - b.h)/2;
	return b;
}



/* Function: dm_get_game_title_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object.
 */
SDL_Rect dm_get_game_title_box(char *title)
{
	SDL_Rect b;
	b.h = dm_scale_to_res(TITLE_H);
	b.w =get_text_width_fits_height(b.h, title);
	b.x = (g_screen_width - b.w)/2;
	b.y = dm_scale_to_res(TITLE_Y);

	return b;
}

/* Function: set_screen_resolution
 * -----------------------------------------------------------------------------
 * Sets the screen resolution that will be used in the game
 *
 * Arguments:
 *	resolution_id: The id of the resolution that will be used
 *
 * Return:
 *	Void.
 */
void dm_set_screen_resolution(int resolution_id)
{
	switch(resolution_id){
		case R1920X1080:
			g_screen_width = 1920;
			g_screen_height = 1080;
			g_res_id = R1920X1080;
			break;
		case R1600X900:
			g_screen_width = 1600;
			g_screen_height = 900;
			g_res_id = R1600X900;
			break;
		default:
			g_screen_width = 1600;
			g_screen_height = 900;
			g_res_id = R1600X900;
			break;
	}	
}

/* Function: scale_to_resoution
 * -----------------------------------------------------------------------------
 * Scales the given dimension according to the scree resolution. The baseline
 * resolution is 1600x900
 *
 * Arguments:
 *	dim: The dimension that will be scaled according to the resolution
 *
 * Return:
 *	Scale variable.
 */
int dm_scale_to_res(int dim)
{
	int scaled_dim;

	switch(g_res_id){
		case R1920X1080:
			scaled_dim = dim*1;
			break;

		case R1600X900:
			scaled_dim = dim*1;
			break;
		default:
			scaled_dim = dim*1;
			break;
	}
	return scaled_dim;
}


















