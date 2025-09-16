#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include"arrow_ar.h"
#include"text_tx.h"
#include"draw_dw.h"
#include"instruction_window_iw.h"
#include"code_window_cw.h"
#include"stage_buttons_sb.h"
#include"registers_rg.h"

texture_t *g_lv_arrow;
texture_t *g_ib_arrow;
texture_t *g_ob_arrow;
texture_t *g_exec_arrow;
texture_t *g_reg_arrow;

static arrow_t g_arrow_ins; 
static arrow_t g_arrow_drop;
static arrow_t g_arrow_play;
static arrow_t g_arrow_code_line;
static arrow_t g_arrow_del;
static arrow_t g_arrow_op2;
static arrow_t g_arrow_error;
static arrow_t g_arrow_challenge;
static arrow_t g_arrow_ib;
static arrow_t g_arrow_ob;
static arrow_t g_arrow_exec;
static arrow_t g_arrow_regs;

static void initialize_ins_arrow();
static void initialize_drop_arrow();
static void initialize_play_arrow();
static void initialize_code_line_arrow();
static void initialize_del_arrow();
static void initialize_op2_arrow();
static void initialize_error_arrow();
static void initialize_challenge_arrow();
static void initialize_ib_arrow();
static void initialize_ob_arrow();
static void initialize_regs_arrow();
static void check_execution_arrow_in_place(int instruction_number);
bool ar_move_execution_arrow(int instruction_number);
static void display_arrow_registers();

/* Function: display_arrow_registers
 * -----------------------------------------------------------------------------
 * Animate with a moving arrow the registers available for selection
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void display_arrow_registers()
{
	List *registers = rg_get_register_list();
	assert(registers != NULL && "Invalid pointer");
	SDL_Rect cb = dm_get_code_button_wh();
	int i = 0;
	LIST_FOREACH(registers, first, next, cur){ 
		reg_t *c = cur->value;
		g_arrow_regs.box.y = c->b->r.y + (cb.h - g_arrow_regs.box.h)/2; 
		g_arrow_regs.travel = 0;
		ar_animate_arrow(&g_arrow_regs);
		i++;
	}
	g_arrow_regs.travel = g_arrow_regs.box.w;
	ar_animate_arrow(&g_arrow_regs);
}

/* Function: ar_move_execution_arrow
 * -----------------------------------------------------------------------------
 * Arguments:
 *	instruction_number: The number of the current instruction being executed
 *
 * Return:
 * 	Bool indicating if arrow is in final position
 */
bool ar_move_execution_arrow(int instruction_number)
{
	int code_size = cw_get_code_list_size();	
	assert(code_size > 0  && "Code size is invalid");
	assert(instruction_number <= code_size && 
		   "Instruction number is incorrect");
	
	bool in_final_position = false;

	SDL_Rect a = dm_get_arrow_wh();
	int y = cw_get_instruction_y_coord(instruction_number) + a.h/6;
	
	if (g_arrow_exec.box.y < y){
		int delta = get_movement_delta(g_arrow_exec.box.y, y, MOVEMENT_DELTA/3);
		g_arrow_exec.box.y += delta;
	} else if (g_arrow_exec.box.y > y){
		int delta = get_movement_delta(g_arrow_exec.box.y, y, MOVEMENT_DELTA/3);
		g_arrow_exec.box.y -= delta;
	}
	if (g_arrow_exec.box.y == y){
		in_final_position = true;
	} 
	return in_final_position;
}

/* Function: check_execution_arrow_in_place
 * -----------------------------------------------------------------------------
 * Arguments:
 *	Evaluates if the execution arrow is it it's final posiition
 *
 * Return:
 *	bool if the exectution arrow arrived to its final position
 */
static void check_execution_arrow_in_place(int instruction_number)
{
	int code_size = cw_get_code_list_size();	
	assert(code_size > 0  && "Code size is invalid");
	assert(instruction_number <= code_size && 
		   "Instruction number is incorrect");

	SDL_Rect a = dm_get_arrow_wh();
	bool in_final_position = false;
	
	int y = cw_get_instruction_y_coord(instruction_number) + a.h/6;
	
	if (g_arrow_exec.box.y == y){
		int delta = get_movement_delta(g_arrow_exec.box.y, y, MOVEMENT_DELTA/6);
		g_arrow_exec.box.y += delta;
	} 
}
/* Function: ar_reset_execution_arrow
 * -------------------------------------
 * Arguments:
 * 	void.
 *
 * Return:
 *	void.
 */
void ar_reset_execution_arrow()
{
	SDL_Rect cb = dm_get_stage_code_box();
	SDL_Rect a = dm_get_arrow_wh();
	
	g_arrow_exec.box.x = cb.x - a.w;
	g_arrow_exec.box.y = cw_get_instruction_y_coord(0) + a.h/6;
	g_arrow_exec.box.w = a.w;
	g_arrow_exec.box.h = a.h;
	g_arrow_exec.texture = g_exec_arrow;
	g_arrow_exec.in_place = true;
	g_arrow_exec.visible = true;
	g_arrow_exec.startx = g_arrow_exec.box.x;
	g_arrow_exec.starty = g_arrow_exec.box.y;	
	g_arrow_exec.dir = AR_RIGHT;
	g_arrow_exec.travel = 0;

}

/* Function: ar_hide_execution_arrow
 * -------------------------------------
 * Arguments:
 * 	void.
 *
 * Return:
 *	void.
 */
void ar_hide_execution_arrow()
{
	g_arrow_exec.visible = false;

}

/* Function: initialize_reg_arrow
 * -----------------------------------------------------------------------------
 * 
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void initialize_regs_arrow()
{
	List *registers = rg_get_register_list();
	reg_t *c = registers->first->value;
	SDL_Rect rb = rg_get_register_box();
	SDL_Rect a = dm_get_arrow_wh();
	
	g_arrow_regs.box.x = c->b->r.x + c->b->r.w + a.w;
	g_arrow_regs.box.w = a.w;
	g_arrow_regs.box.h = a.h;
	g_arrow_regs.travel = a.w;
	g_arrow_regs.startx = g_arrow_regs.box.x;
	g_arrow_regs.dir = AR_LEFT;
	g_arrow_regs.in_place = false;
	g_arrow_regs.texture = g_reg_arrow;
	g_arrow_regs.visible =  true;
	SDL_SetTextureColorMod(g_arrow_regs.texture->texture, 255, 255, 0);
}
/* Function: initialize_ins_arrow
 * -----------------------------------------------------------------------------
 * 
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void initialize_ins_arrow()
{
	int size = iw_get_instruction_list_size();
	SDL_Rect ir = iw_get_instruction_rect_by_pos(size - 1);
	SDL_Rect a = dm_get_arrow_wh();
	g_arrow_ins.box.x = ir.x + a.w/2; 
	g_arrow_ins.box.y = ir.y + ir.h + a.h;
	g_arrow_ins.box.w = a.w; 
	g_arrow_ins.box.h = a.h;
	g_arrow_ins.startx = g_arrow_ins.box.x;
	g_arrow_ins.starty = g_arrow_ins.box.y;
	g_arrow_ins.travel = a.h;			
	g_arrow_ins.dir = AR_UP;
	g_arrow_ins.in_place = false;
	g_arrow_ins.visible =  true;
	g_arrow_ins.texture =  g_lv_arrow;
}

/* Function: initialize_code_arrow
 * -----------------------------------------------------------------------------
 * 
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void initialize_drop_arrow()
{
	SDL_Rect a = dm_get_arrow_wh();
	SDL_Rect bi = dm_get_text_box_ins();
	SDL_Rect cb = dm_get_stage_code_box();
	
	g_arrow_drop.box.x = (cb.x - 2*a.w);
	g_arrow_drop.box.y = bi.y - a.h*3/2;
	g_arrow_drop.box.w = a.w;
	g_arrow_drop.box.h = a.h;
	g_arrow_drop.in_place = false;
	g_arrow_drop.visible = true;
	g_arrow_drop.startx = g_arrow_drop.box.x;
	g_arrow_drop.starty = g_arrow_drop.box.y;
	g_arrow_drop.travel = a.w; 
	g_arrow_drop.dir = AR_RIGHT;
	g_arrow_drop.texture = g_lv_arrow;
}

/* Function: initialize_play_arrow
 * -----------------------------------------------------------------------------
 * 
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void initialize_play_arrow()
{
	SDL_Rect sb = dm_get_stage_buttons();
	SDL_Rect a = dm_get_arrow_wh();
	g_arrow_play.box.w = a.w;	
	g_arrow_play.box.h = a.h;
	g_arrow_play.box.x = sb_get_play_button_member(MEMBER_X);
	g_arrow_play.box.y = sb.y - 2*a.h;
	g_arrow_play.in_place = false;
	g_arrow_play.visible = true;
	g_arrow_play.startx = sb.x;
	g_arrow_play.starty = g_arrow_play.box.y;	
	g_arrow_play.travel = a.h;	
	g_arrow_play.dir = AR_DOWN;
	g_arrow_play.texture = g_lv_arrow;
}

/* Function: initialize_code_line_arrow
 * -----------------------------------------------------------------------------
 * 
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void initialize_code_line_arrow()
{
	SDL_Rect a = dm_get_arrow_wh();
	int pos = cw_get_code_list_size() - 1;

	if (pos >= 0){
		SDL_Rect cl = cw_get_code_line_coord_at_pos(CW_INS, pos);
		g_arrow_code_line.box.x = cl.x + a.w/2;
		g_arrow_code_line.box.y = cl.y + cl.h;
	} else {
		g_arrow_code_line.box.x = 0;
		g_arrow_code_line.box.y = 0;
	}
	g_arrow_code_line.box.w = a.w;	
	g_arrow_code_line.box.h = a.h;
	g_arrow_code_line.in_place = false;
	g_arrow_code_line.visible = true;
	g_arrow_code_line.startx = g_arrow_code_line.box.x;
	g_arrow_code_line.starty = g_arrow_code_line.box.y;
	g_arrow_code_line.travel = a.h;	
	g_arrow_code_line.dir = AR_UP;
	g_arrow_code_line.texture = g_lv_arrow;

}

/* Function: initialize_del_arrow
 * -----------------------------------------------------------------------------
 * 
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void initialize_del_arrow()
{
	SDL_Rect a = dm_get_arrow_wh();
	SDL_Rect cb = dm_get_stage_code_box();
	SDL_Rect tb = cw_get_text_box_rect();
	g_arrow_del.box.x = cb.x + cb.w;
	g_arrow_del.box.y = tb.y + tb.h/2;
	g_arrow_del.box.w = a.w;
	g_arrow_del.box.h = a.h;
	g_arrow_del.in_place = false;
	g_arrow_del.visible = true;
	g_arrow_del.startx = g_arrow_del.box.x;
	g_arrow_del.starty = g_arrow_del.box.y;
	g_arrow_del.travel = a.w;
	g_arrow_del.dir = AR_RIGHT;
	g_arrow_del.texture = g_lv_arrow;
}

/* Function: initialize_op2_arrow
 * -----------------------------------------------------------------------------
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void initialize_op2_arrow()
{
	SDL_Rect a = dm_get_arrow_wh();
	
	int pos = 1;
	int size  = cw_get_code_list_size();
	if (pos <= (size-1)){
		SDL_Rect cl = cw_get_code_line_coord_at_pos(CW_OP2, pos);
		g_arrow_op2.box.x = cl.x + a.w/2;
		g_arrow_op2.box.y = cl.y + cl.h;
	} else{
		g_arrow_op2.box.x = 0;
		g_arrow_op2.box.y = 0;
	}
	g_arrow_op2.box.w = a.w;
	g_arrow_op2.box.h = a.h;
	g_arrow_op2.in_place = false;
	g_arrow_op2.visible = true;
	g_arrow_op2.startx = g_arrow_op2.box.x;
	g_arrow_op2.starty = g_arrow_op2.box.y;			
	g_arrow_op2.travel = a.h;
	g_arrow_op2.dir = AR_UP;
	g_arrow_op2.texture = g_lv_arrow;
}

/* Function: initialize_op2_arrow
 * -----------------------------------------------------------------------------
 * 
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void initialize_error_arrow()
{
	SDL_Rect a = dm_get_arrow_wh();
	SDL_Rect rb = dm_get_text_box_result();
	SDL_Rect eb = dm_get_text_box_error();
	SDL_Rect back_but =  dm_get_text_box_result_but1();
	g_arrow_error.box.x = rb.x + rb.w/2 - a.w/2;
	g_arrow_error.box.y = eb.y - a.h;
	g_arrow_error.box.w = a.w;
	g_arrow_error.box.h = a.h;
	g_arrow_error.in_place = false;
	g_arrow_error.visible = true;
	g_arrow_error.startx = rb.x + rb.w/2 - back_but.w/2 - 2*a.w;			
	g_arrow_error.starty = eb.y - a.h;
	g_arrow_error.travel = a.h;
	g_arrow_error.dir = AR_UP;
	g_arrow_error.texture = g_lv_arrow;
}


/* Function: initialize_challenge_arrow
 * -----------------------------------------------------------------------------
 * 
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void initialize_challenge_arrow()
{
	SDL_Rect a = dm_get_arrow_wh();
	SDL_Rect tb = cw_get_text_box_rect();
	SDL_Rect cb = dm_get_stage_code_box();
	g_arrow_challenge.box.x = cb.x + cb.w + a.w;
	g_arrow_challenge.box.y = tb.y + tb.h/2 - a.h/2;
	g_arrow_challenge.box.w = a.w;
	g_arrow_challenge.box.h = a.h;
	g_arrow_challenge.in_place = false;
	g_arrow_challenge.visible = true;
	g_arrow_challenge.startx = g_arrow_challenge.box.x;
	g_arrow_challenge.starty = g_arrow_challenge.box.y;
	g_arrow_challenge.travel = a.w;
	g_arrow_challenge.dir = AR_LEFT;
	g_arrow_challenge.texture = g_lv_arrow;
}

/* Function: initialize_ib_arrow
 * -----------------------------------------------------------------------------
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void initialize_ib_arrow()
{ 	
	SDL_Rect b =  dm_get_stage_input_buffer_box();
	SDL_Rect dim = dm_get_arrow_wh();
	g_arrow_ib.box.x = b.x - 2*dim.w;
	g_arrow_ib.box.y = b.y + b.h/2 - dim.h/2;
	g_arrow_ib.box.w = dim.w;
	g_arrow_ib.box.h = dim.h;
	g_arrow_ib.startx = g_arrow_ib.box.x;
	g_arrow_ib.starty = g_arrow_ib.box.y;
	g_arrow_ib.dir = AR_RIGHT;
	g_arrow_ib.travel = b.x - g_arrow_ib.startx - dim.w;
	g_arrow_ib.in_place = false;
	g_arrow_ib.texture = g_ib_arrow;
	g_arrow_ib.visible = true;
	SDL_SetTextureColorMod(g_arrow_ib.texture->texture, 255, 0, 255);

}


/* Function: initialize_ob_arrow
 * -----------------------------------------------------------------------------
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
static void initialize_ob_arrow()
{ 	
	SDL_Rect b =  dm_get_stage_output_buffer_box();
	SDL_Rect dim = dm_get_arrow_wh();
	g_arrow_ob.box.x = b.x - 2*dim.w;
	g_arrow_ob.box.y = b.y + b.h/2 - dim.h/2;
	g_arrow_ob.box.w = dim.w;
	g_arrow_ob.box.h = dim.h;
	g_arrow_ob.startx = g_arrow_ib.box.x;
	g_arrow_ob.starty = g_arrow_ib.box.y;
	g_arrow_ob.dir = AR_RIGHT;
	g_arrow_ob.travel = b.x - g_arrow_ib.startx - dim.w;
	g_arrow_ob.in_place = false;
	g_arrow_ob.texture = g_ob_arrow;
	g_arrow_ob.visible = true;
	SDL_SetTextureColorMod(g_arrow_ob.texture->texture, 0, 255, 255);
}

/* Function: initialize_arrow
 * -----------------------------------------------------------------------------
 * Initialize all the corresponding arrows
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
void ar_initialize_arrows()
{
	SDL_SetTextureColorMod(g_lv_arrow->texture, 255, 0, 0);
	initialize_ins_arrow();
	initialize_drop_arrow();
	initialize_play_arrow();
	initialize_code_line_arrow();
	initialize_del_arrow();
	initialize_op2_arrow();
	initialize_error_arrow();
	initialize_challenge_arrow();
	initialize_ib_arrow();
	initialize_ob_arrow();
	initialize_regs_arrow();
}
/* Function: ar_init_arrow
 * -----------------------------------------------------------------------------
 * Animates the arrow that points to the first instruction
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
void ar_init_arrow(int arrow_id)
{
	switch(arrow_id){
		case AR_INS:
			initialize_ins_arrow();
			break;
		case AR_DROP:
			initialize_drop_arrow();
			break;
		case AR_PLAY:
			initialize_play_arrow();
			break;
		case AR_CODE:
			initialize_code_line_arrow();
			break;
		case AR_DEL:
			initialize_del_arrow();
			break;
		case AR_OP2:
			initialize_op2_arrow();
			break;
		case AR_ERROR:
			initialize_error_arrow();
			break;
		case AR_CHALLENGE:
			initialize_challenge_arrow();
			break;
		case AR_IB:
			initialize_ib_arrow();
			break;
		case AR_OB:
			initialize_ob_arrow();
			break;
		case AR_EXEC:
			ar_reset_execution_arrow();
			break;
		case AR_REG:
			initialize_regs_arrow();
			break;
		default:
			break;
	}
}

/* Function: display_arrow
 * -----------------------------------------------------------------------------
 * Animates the arrow that points to the first instruction
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
void ar_display_arrow(int arrow_id)
{
	int x;
	int y;
	int travel;
	int dir;
	arrow_t *aptr;
	switch(arrow_id){
		case AR_INS:
			aptr = &g_arrow_ins;
			break;
		case AR_DROP:
			aptr = &g_arrow_drop;
			break;
		case AR_PLAY:
			aptr = &g_arrow_play;
			break;
		case AR_CODE:
			aptr = &g_arrow_code_line;
			break;
		case AR_DEL:
			aptr = &g_arrow_del;
			break;
		case AR_OP2:
			aptr = &g_arrow_op2;
			break;
		case AR_ERROR:
			aptr = &g_arrow_error;
			break;
		case AR_CHALLENGE:
			aptr = &g_arrow_challenge;
			break;
		case AR_IB:
			aptr = &g_arrow_ib;
			break;
		case AR_OB:
			aptr = &g_arrow_ob;
			break;
		case AR_EXEC:
			aptr = &g_arrow_exec;
			break;
		case AR_REG:
			aptr = &g_arrow_regs;
			display_arrow_registers();
			break;
		default:
			break;
	}
	if (aptr->visible == true && arrow_id != AR_REG){
		ar_animate_arrow(aptr);
	}
}


/* Function: ar_animate_arrow
 * -----------------------------------------------------------------------------
 * Animates the movement of an arrow object in a given direction
 *
 * Arguments:
 *	x: starting x position.
 *	y: starting y position.
 *	arrow: arrow object that will be animated
 *  dir: direction of the movement of the arrow
 *  travel: how long the animation will move on the screen
 *
 * Return:
 *	Void.
 */
void ar_animate_arrow(arrow_t *arrow) 
{
	assert(arrow !=NULL && "arrow object is NULL");
	
	switch(arrow->dir){
		case AR_UP:
			if (arrow->travel != 0){
				if (arrow->box.y <= arrow->starty - arrow->travel){
					arrow->in_place = true;	
				} else if (arrow->box.y >= arrow->starty){
					arrow->in_place = false;
				}
				if (arrow->in_place == false){
					arrow->box.y--;
				}else if (arrow->in_place == true){
					arrow->box.y++;
				}
			}
			dw_draw_rotated_texture_fits_h(arrow->box.x, arrow->box.y, 
	     								   arrow->box.h, -90.0, arrow->texture);
			break;

		case AR_DOWN:
			if (arrow->travel != 0){
				if (arrow->box.y >= arrow->starty + arrow->travel){
					arrow->in_place = true;	
				} else if (arrow->box.y <= arrow->starty){
					arrow->in_place = false;
				}
				if (arrow->in_place == false){
					arrow->box.y++;
				}else if (arrow->in_place == true){
					arrow->box.y--;
				}
			}
			dw_draw_rotated_texture_fits_h(arrow->box.x, arrow->box.y, 
	     								   arrow->box.h, 90.0, arrow->texture);
			break;

		case AR_RIGHT:
			if (arrow->travel != 0){
				if (arrow->box.x >= arrow->startx + arrow->travel){
					arrow->in_place = true;	
				} else if (arrow->box.x <= arrow->startx){
					arrow->in_place = false;
				}
				if (arrow->in_place == false){
					arrow->box.x++;
				}else if (arrow->in_place == true){
					arrow->box.x--;
				}
			}
			dw_draw_rotated_texture_fits_h(arrow->box.x, arrow->box.y, 
										     arrow->box.h, 0.0, arrow->texture);
			break;
		case AR_LEFT:
			if (arrow->travel != 0){
				if (arrow->box.x <= arrow->startx - arrow->travel){
					arrow->in_place = true;	
				} else if (arrow->box.x >= arrow->startx){
					arrow->in_place = false;
				}
				if (arrow->in_place == false){
					arrow->box.x--;
				}else if (arrow->in_place == true){
					arrow->box.x++;
				}
			}
			dw_draw_rotated_texture_fits_h(arrow->box.x, arrow->box.y, 
										  arrow->box.h, 180.00, arrow->texture);
			break;
		default:
			break;
	}
	return;
}

