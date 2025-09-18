#include<stdio.h>
#include<stdlib.h>
#include"dimensions_dm.h"
#include"registers_rg.h"
#include <SDL.h>


#define ESC_MENU_BOX_W 600
#define ESC_MENU_BOX_H 300
#define SCREEN_BORDERS_OFS 2
#define RES_BOX_W 500
#define RES_BOX_H 300

#define RES_BOX_OFFSET 25
#define VALUE_BOX_H 40
#define VALUE_BOX_W 50
#define REG_BOX_OFS 25

#define ARROW_H 30
#define ARROW_W 30
#define AVATAR_W 50
#define AVATAR_H 50

//#define CODE_LINE_NUMBER_OFFSET 15

#define TEXT_H_BIG 50
#define TEXT_H_BOTTOM_MSG 20
#define TEXT_H_MSG 30
#define TEXT_H_ERROR_MSG 40
#define TEXT_H_TOTAL_MSG 160
#define TEXT_H_STAGE_ELEMENTS_TITLES 40
#define TEXT_H_STAGE_TITLES 100
#define TEXT_H_LINE_NUMBER 40

#define STAGE_BUTTON_W 40
#define STAGE_BUTTON_H 40
#define P_BUTTON_H 200
#define P_BUTTON_W 200
#define CODE_BUTTON_W 75
#define CODE_BUTTON_H 40
#define COMMA_OFS 10
#define ESC_MENU_BUTTON_W 500
#define ESC_MENU_BUTTON_H 60
#define SEL_LEVEL_BUTTON_W 100
#define SEL_LEVEL_BUTTON_H 50
#define RET_BUTTON_W 90
#define RET_BUTTON_H 75
#define RES_BUTTON_H 60
#define RES_BACK_BUTTON_W 100
#define RES_CONT_BUTTON_W 200

#define RAIL_W 4
#define RAIL_END_W 16

int g_res_id;
int g_screen_width;
int g_screen_height;

int scale_to_resolution(int dim);

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
int dm_get_ofs_space_stage_buttons()
{
	SDL_Rect sb = dm_get_stage_buttons();
	return sb.w/2;
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
	SDL_Rect vbox = dm_get_vbox_wh();
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
	SDL_Rect vbox =  dm_get_vbox_wh();
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
	int ofs = dm_get_ofs_code_box_border();
	SDL_Rect cb = dm_get_stage_code_box();
	int w = cb.w - 2*ofs;
	return w;
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
	int h = scale_to_resolution(TEXT_H_STAGE_TITLES);
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
	int h = scale_to_resolution(TEXT_H_STAGE_ELEMENTS_TITLES);
	return h;
	
}

/* Function: dm_get_ofs_code_box_border
 * -----------------------------------------------------------------------------
 *	Return the offset value of the contents of the buffer. 
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset
 */
int dm_get_ofs_code_box_border()
{
	SDL_Rect cb = dm_get_stage_code_box();
	int ofs = scale_to_resolution(0.05*cb.w);
	return ofs;
}
/* Function: dm_get_ofs_code_number
 * -----------------------------------------------------------------------------
 *	Return the offset value of the contents of the buffer. 
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset
 */
int dm_get_ofs_code_number()
{
	SDL_Rect r = dm_get_code_button_wh();
	int ofs = r.w/2;
	return ofs;
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
	int ofs = dm_get_ofs_code_op2() - scale_to_resolution(COMMA_OFS);
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
	int line_number_ofs = dm_get_ofs_code_number();
	int ofs = cb.w;// + line_number_ofs;
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
	int line_number_ofs = dm_get_ofs_code_number();
	int ofs = 2*cb.w;// + 2*line_number_ofs;
	return ofs;
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
	int offset = scale_to_resolution(REG_BOX_OFS);
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
	int w = scale_to_resolution(RAIL_W);
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
	int w = scale_to_resolution(RAIL_END_W);
	return w;
	
}
/* Function: dm_get_vbox_val_wh
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object. 
 * 
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_vbox_val_wh()
{
	SDL_Rect vb = dm_get_vbox_wh();
	SDL_Rect b;
	b.w = vb.w;
	b.h = vb.h - vb.h/10;
	b.x = 0;
	b.y = 0;
	return b;
}
/* Function: dm_get_vbox_wh
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object. 
 * 
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_vbox_wh()
{
	SDL_Rect b;
	b.w = scale_to_resolution(VALUE_BOX_W);
	b.h = scale_to_resolution(VALUE_BOX_H);
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
	b.w = scale_to_resolution(ARROW_W);
	b.h = scale_to_resolution(ARROW_H);
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
	int cont_w = scale_to_resolution(RES_CONT_BUTTON_W);
	int offset = scale_to_resolution(RES_BOX_OFFSET);
	SDL_Rect rb = dm_get_text_box_result();
	SDL_Rect b;
	b.w = scale_to_resolution(RES_BACK_BUTTON_W);
	b.h = scale_to_resolution(RES_BUTTON_H);
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
	int back_w = scale_to_resolution(RES_BACK_BUTTON_W);
	int cont_w = scale_to_resolution(RES_CONT_BUTTON_W);
	int offset = scale_to_resolution(RES_BOX_OFFSET);
	SDL_Rect rb = dm_get_text_box_result();
	SDL_Rect b;
	b.w = scale_to_resolution(RES_CONT_BUTTON_W);
	b.h = scale_to_resolution(RES_BUTTON_H);
	b.x = rb.x + back_w + 2*(rb.w - (back_w + cont_w))/3 ;
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
	int cont_w = scale_to_resolution(RES_CONT_BUTTON_W);
	int offset = scale_to_resolution(RES_BOX_OFFSET);
	SDL_Rect rb = dm_get_text_box_result();
	SDL_Rect b;
	b.w = scale_to_resolution(RES_BACK_BUTTON_W);
	b.h = scale_to_resolution(RES_BUTTON_H);
	b.x = rb.x + (rb.w - (b.w + cont_w))/3;
	b.y = rb.y + rb.h - b.h - offset;
	return b;
}

/* Function: dm_get_text_box_result_text2
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
SDL_Rect dm_get_text_box_result_text2()
{
	SDL_Rect rb = dm_get_text_box_result();
	int offset = scale_to_resolution(RES_BOX_OFFSET);
	SDL_Rect b;
	b.w = rb.w - 2*offset;
	b.h = scale_to_resolution(TEXT_H_TOTAL_MSG);
	b.x = rb.x + (rb.w - b.w)/2;
	b.y = rb.y + offset;
	return b;
}

/* Function: dm_get_text_box_result_text1
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
SDL_Rect dm_get_text_box_result_text1()
{
	SDL_Rect rb = dm_get_text_box_result();
	int offset = scale_to_resolution(RES_BOX_OFFSET);
	SDL_Rect b;
	b.w = rb.w - 2*offset;
	b.h = scale_to_resolution(TEXT_H_MSG);
	b.x = rb.x + (rb.w - b.w)/2;
	b.y = rb.y + offset;
	return b;
}

/* Function: dm_get_text_box_result
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_text_box_result()
{
	SDL_Rect b;
	b.w = scale_to_resolution(RES_BOX_W);
	b.h = scale_to_resolution(RES_BOX_H);
	b.x = (g_screen_width - b.w)/2;
	b.y = g_screen_height/2 - b.h*2/3;
	return b;
}


/* Function: dm_get_box_msg_wh
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object, x and y are initialize at 0
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_box_msg_wh()
{
	SDL_Rect ib = dm_get_stage_instruction_box();
	SDL_Rect b;
	b.w = ib.w + ib.w/3;
	b.h = g_screen_height/4;
	b.x = 0;
	b.y = 0;
	return b;
}

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
	SDL_Rect rb = dm_get_text_box_result();
	SDL_Rect mb = dm_get_box_msg_wh();
	SDL_Rect ab = dm_get_arrow_wh();
	
	SDL_Rect b;
	b.w = rb.w*2/3;
	b.h = mb.h;
	b.x = rb.x + (rb.w/3)/2;
	b.y = rb.y + rb.h + 2*ab.h;
	return b;
}

/* Function: dm_get_text_box_lower
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_text_box_lower()
{
	SDL_Rect rb = dm_get_text_box_result();
	SDL_Rect mb = dm_get_box_msg_wh();	
	SDL_Rect ab = dm_get_arrow_wh();
	
	SDL_Rect b;
	b.w = mb.w;
	b.h = mb.h;
	b.x = g_screen_width/2 - mb.w/2;
	b.y = rb.y + rb.h + 2*ab.h;
	return b;
}

/* Function: dm_get_text_box_big
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_text_box_big()
{
	SDL_Rect b;
	b.w = g_screen_width/3;
	b.h = g_screen_height/2;
	b.x = g_screen_width/2 - b.w/2;
	b.y = g_screen_height/4;
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
	SDL_Rect cb = dm_get_stage_code_box();	
	SDL_Rect mb = dm_get_box_msg_wh();	
	SDL_Rect b;
	b.w = mb.w;
	b.h = mb.h;
	b.x = cb.x + (cb.w - mb.w)/2;
	b.y = (cb.y + cb.h) - mb.h/2;
	return b;
}

/* Function: dm_get_text_box_code
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_text_box_code()
{
	SDL_Rect cb = dm_get_stage_code_box();	
	SDL_Rect mb = dm_get_box_msg_wh();	
	SDL_Rect b;
	b.w = mb.w;
	b.h = mb.h;
	b.x = cb.x + (cb.w - mb.w)/2;
	b.y = cb.y + cb.h - mb.h/2;
	return b;
}
/* Function: dm_get_text_box_ins
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_text_box_ins()
{

	SDL_Rect ib = dm_get_stage_instruction_box();	
	SDL_Rect mb = dm_get_box_msg_wh();	
	SDL_Rect b;
	b.w = mb.w;
	b.h = mb.h;
	b.x = ib.x;
	b.y = ib.y + ib.h;
	return b;
}



/* Function: dm_get_text_box_center
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_text_box_center()
{
	SDL_Rect mb = dm_get_box_msg_wh();	
	SDL_Rect ib = dm_get_stage_input_buffer_box();	
	SDL_Rect b;
	b.w = mb.w;
	b.h = mb.h;
	b.x = ib.x - mb.w/2;
	b.y = g_screen_height/2 - mb.h/2;
	return b;
}

/* Function: dm_get_text_box_upper
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_text_box_upper()
{
	SDL_Rect mb = dm_get_box_msg_wh();	
	SDL_Rect b;
	b.w = mb.w;
	b.h = mb.h;
	b.x = g_screen_width/2 - mb.w/2;
	b.y = scale_to_resolution(TEXT_H_STAGE_ELEMENTS_TITLES);
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
	b.h = g_screen_height/20;
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
	b.h = g_screen_height/20;
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
	b.w = g_screen_width*3/5;
	b.h = g_screen_height/12;
	b.x = g_screen_width*4/6;
	b.y = g_screen_height/20;
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
	b.w = g_screen_width*3/5;
	b.h = g_screen_height/12;
	b.x = g_screen_width*4/6;
	b.y = g_screen_height - ib.h - b.h;
	return b;
}

/* Function: dm_get_stage_instruction_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object, x and y are initialize at 0
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
	b.w = (g_screen_width*2/6)/3;
	b.h = g_screen_height/4;
	b.x = 0;
	b.y = g_screen_height/2 - b.h;
	return b;
}

/* Function: dm_get_code_stage_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object, x and y are initialize at 0
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_stage_code_box()
{
	SDL_Rect ib = dm_get_stage_instruction_box();
	SDL_Rect b;

	b.w = ((g_screen_width*2/6)*2/3);
	b.h = (g_screen_height*2/3);
	b.x = ib.x + ib.w;
	b.y = g_screen_height/18;
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
	SDL_Rect vb = dm_get_vbox_wh();
	SDL_Rect rb = rg_get_register_box();
	SDL_Rect b;
	b.w = vb.w;
	b.h = vb.h;
	b.x = rb.x + rb.w - b.w - b.w/5;
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
	SDL_Rect vb = dm_get_vbox_wh();
	SDL_Rect rb = rg_get_register_box();
	SDL_Rect b;
	b.w = vb.w;
	b.h = vb.h;
	b.x = rb.x + rb.w - b.w - b.w/5;
	b.y = rb.y + rb.h + b.h/4;
	return b;
}

/* Function: dm_get_stage_registers_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object, x and y are initialize at 0
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_stage_registers_box()
{
	SDL_Rect cb = dm_get_stage_code_box();
	SDL_Rect b;
	b.w = g_screen_width/5;
	b.h = g_screen_height/4;
	b.x = cb.x + cb.w;
	b.y = 0;
	return b;
}

/* Function: dm_get_return_button_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object, x and y are initialize at 0
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_return_button_box()
{
	SDL_Rect b;
	b.w = scale_to_resolution(RET_BUTTON_W);
	b.h = scale_to_resolution(RET_BUTTON_H);
	b.x = SCREEN_BORDERS_OFS;
	b.y = g_screen_height - b.h - SCREEN_BORDERS_OFS;
	return b;
}

/* Function: dm_get_h_bottom_msg
 * -----------------------------------------------------------------------------
 * Returns the h value for the click anywhere message
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset for the sel level buttons
 */
int dm_get_h_bottom_msg()
{
	int h = scale_to_resolution(TEXT_H_BOTTOM_MSG);
	return h;
	
}

/* Function: dm_get_h_error_msg
 * -----------------------------------------------------------------------------
 * Returns the h value for the click anywhere message
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset for the sel level buttons
 */
int dm_get_h_error_msg()
{
	int h = scale_to_resolution(TEXT_H_ERROR_MSG);
	return h;
	
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
	int h = scale_to_resolution(TEXT_H_MSG);
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
int dm_get_h_line_number()
{
	int h = scale_to_resolution(TEXT_H_LINE_NUMBER);
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
	int h = scale_to_resolution(TEXT_H_BIG);
	return h;
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
int get_sel_level_offset_y()
{
	int y_offset =  scale_to_resolution(SEL_LEVEL_BUTTON_H + 
										0.5*SEL_LEVEL_BUTTON_H);
	return y_offset;
	
}

/* Function: dm_get_level_button_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_level_button_box()
{
	SDL_Rect b;
	b.w = scale_to_resolution(SEL_LEVEL_BUTTON_W);
	b.h = scale_to_resolution(SEL_LEVEL_BUTTON_H);
	b.x = (g_screen_width - (5*b.w))/6;
	b.y = g_screen_height/6;
	return b;
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
SDL_Rect dm_get_upper_title_box()
{
	SDL_Rect b;
	b.h = scale_to_resolution(TEXT_H_STAGE_TITLES);
	b.x = 0;
	b.y = g_screen_height/36;
	b.w = g_screen_width;
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
	b.w = scale_to_resolution(AVATAR_W);
	b.h = scale_to_resolution(AVATAR_H);
	b.x = 0;
	b.y = 0;
	return b;
}

/* Function: dm_get_stage_buttons
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object, x and y are initialize at 0
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_stage_buttons()
{
	SDL_Rect b;
	b.w = scale_to_resolution(STAGE_BUTTON_W);
	b.h = scale_to_resolution(STAGE_BUTTON_H);
	b.x = ((g_screen_width*2/6) + ((g_screen_width/6) - 4*b.w)/4);
	b.y = g_screen_height - g_screen_height/15;
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
	b.w = scale_to_resolution(CODE_BUTTON_W);
	b.h = scale_to_resolution(CODE_BUTTON_H);
	b.x = 0;
	b.y = 0;
	return b;
}

/* Function: dm_get_escape_menu_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_escape_menu_box()
{
	SDL_Rect b;
	b.w = scale_to_resolution(ESC_MENU_BOX_W);
	b.h = scale_to_resolution(ESC_MENU_BOX_H);
	b.x = (g_screen_width - b.w)/2;
	b.y = (g_screen_height - b.h)/2;
	return b;
}

/* Function: dm_get_escape_b1_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_escape_b1_box()
{
	SDL_Rect mb = dm_get_escape_menu_box();

	SDL_Rect b;
	b.w = scale_to_resolution(ESC_MENU_BUTTON_W);
	b.h = scale_to_resolution(ESC_MENU_BUTTON_H);
	b.x = mb.x + mb.w/2 - b.w/2;
	b.y = mb.y + (mb.h - 3*b.h)/4 ;
	return b;
}

/* Function: dm_get_escape_b1_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_escape_b2_box()
{
	SDL_Rect mb = dm_get_escape_menu_box();
	SDL_Rect b1 = dm_get_escape_b1_box();
	
	SDL_Rect b;
	b.w = scale_to_resolution(ESC_MENU_BUTTON_W);
	b.h = scale_to_resolution(ESC_MENU_BUTTON_H);
	b.x = b1.x;
	b.y = b1.y + (mb.h - 3*b.h)/4 + b.h;
	return b;
}

/* Function: dm_get_escape_b3_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_escape_b3_box()
{
	SDL_Rect mb = dm_get_escape_menu_box();
	SDL_Rect b2 = dm_get_escape_b2_box();
	
	SDL_Rect b;
	b.w = scale_to_resolution(ESC_MENU_BUTTON_W);
	b.h = scale_to_resolution(ESC_MENU_BUTTON_H);
	b.x = b2.x;
	b.y = b2.y + (mb.h - 3*b.h)/4 + b.h;
	return b;
}
/* Function: dm_get_p1_button_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_p1_button_box()
{
	SDL_Rect b;
	b.w = scale_to_resolution(P_BUTTON_W);
	b.h = scale_to_resolution(P_BUTTON_H);
	b.x = (g_screen_width - 3*b.w)/4;
	b.y = g_screen_height/3;
	return b;
}

/* Function: dm_get_p2_button_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_p2_button_box()
{
	SDL_Rect b;
	b.w = scale_to_resolution(P_BUTTON_W);
	b.h = scale_to_resolution(P_BUTTON_H);
	b.x = 2*(g_screen_width - 3*b.w)/4 + b.w;
	b.y = g_screen_height/3;
	return b;
}

/* Function: dm_get_p3_button_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_p3_button_box()
{
	SDL_Rect b;
	b.w = scale_to_resolution(P_BUTTON_W);
	b.h = scale_to_resolution(P_BUTTON_H);
	b.x = 3*(g_screen_width - 3*b.w)/4 + 2*b.w;
	b.y = g_screen_height/3;
	return b;
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
SDL_Rect dm_get_press_space_box()
{
	const int texture_w = 400;
	
	SDL_Rect b;
	b.w = scale_to_resolution(texture_w);
	b.x = (g_screen_width - b.w)/2;
	b.y = g_screen_height/2;
	b.h = 0;
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
	const int texture_w = 600;
	
	SDL_Rect b;
	b.w = scale_to_resolution(texture_w);
	b.x = (g_screen_width - b.w)/2;
	b.y = g_screen_height/3;
	b.h = 0;
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
SDL_Rect dm_get_game_title_box()
{
	const int texture_w = 1100;
	const int texture_y = 25;
	
	SDL_Rect b;
	b.w = scale_to_resolution(texture_w);
	b.x = (g_screen_width - b.w)/2;
	b.y = scale_to_resolution(texture_y);
	b.h = 0;
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
int scale_to_resolution(int dim)
{
	int scaled_dim;

	switch(g_res_id){
		case R1600X900:
			scaled_dim = dim*1;
			break;
		default:
			scaled_dim = dim*1;
			break;
	}
	return scaled_dim;
}


















