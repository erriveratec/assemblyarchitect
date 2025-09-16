#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include"text_tx.h"
#include"draw_dw.h"
#include"instruction_window_iw.h"
#include"code_window_cw.h"
#include"dimensions_dm.h"


#define MSG_CLICKANY "Click anywhere to continue"
#define MSG_PRESSPLAY "Press the play button"
#define MSG_PRESSBACK "Press the back button to continue"
#define MSG_PRESSCONT "Press the continue button"

#define L1_WELCOME "Welcome to Level 01"
#define L1_DESCRIPTION "First, read the challenge description"
#define L1_OBJ1 "Objective: move items from the Input Buffer [IB]"
#define L1_OBJ2 "To the Output Buffer [OB]"
#define L1_OBJ3 "Values recovered from the Input Buffer [IB], must"\
" go to a register first"
#define L1_SELINS1 "Select and drag the \"mov\" instruction from"\
" the instruction box"
#define L1_DROPINS1 "Drop the instruction in the code box"
#define L1_SELOP1 "Select \"rax\" as the destination operand"\
" (where the value will be stored)"
#define L1_SELOP2 "Select the Input Buffer [IB] as the source"\
" operand (from where the value is recovered)"
#define L1_PLAYTUT "Press the play button to see what the instruction does"
#define L1_SELINS2 "Select and drag another \"mov\" instruction from the"\
" instruction box"
#define L1_ERROR "The value was moved from the Input Buffer [IB] to rax."\
" Now we need to move it from rax to the Output Buffer [OB]"
#define L1_DROPINS2 "Drop the instruction in the code box"\
" below the previous instruction"
#define L1_SELOP1_2 "Select the Output Buffer [OB] as the destination operand"
#define L1_SELOP2_2 "Select rax as the source operand"
#define L1_CONGRATS "Congratulations, you have beaten the first level"

#define L2_WELCOME "Welcome to Level 02"
#define L2_DESCRIPTION "We will learn how to rearrange, delete and"\
" change operands of the instruction"
#define L2_SELLAST "Select and drag the last instruction"\
" by clicking in the instruction name."	
#define L2_DELINS "Drag the instruction out of the code box to delete it"
#define L2_CHANGEOP "Click the source operand of the instruction to change it"
#define L2_SELIB "Select the Input Buffer [IB] as the source operand"
#define L2_MOVINS "Select and drag the second instruction and move in to"\
" the first position"

#define L3_WELCOME "Welcome to Level 03"
#define L3_DESCRIPTION1 "A value retrieved from the Input Buffer [IB]"\
" can only be read once"
#define L3_DESCRIPTION2 "A value stored on a register can be read multiple"\
" times"
#define L3_SELINS1 "Select and drag the \"mov\" instruction from"\
" the instruction box"
#define L3_DROPINS "Drop the instruction in the code box"
#define L3_AVAILOPS1 "All available operands for selection will be pointed"\
" with an arrow"
#define L3_SELRAX "Select \"rax\""
#define L3_AVAILOPS2 "Only valid operands combinations will be available for"\
" selection"
#define L3_SELIB "Select Input Buffer [IB]"
#define L3_SELINS2 "Use more \"mov\" instructions to solve the challenge"
#define L3_READ "Use\n\"mov [OB], rax\"\nseveral times to solve the challenge"

#define L4_WELCOME "Welcome to Level 04"
#define L4_DESCRIPTION1 "Solve the challenge by yourself applying what has"\
" been learned"\

#define L5_WELCOME "Welcome to Level 05" 
#define L5_DESCRIPTION1 "There are more"\
" registers available. Use them to solve the challenge."\

#define L6_WELCOME "Welcome to Level 06" 
#define L6_DESCRIPTION1 "Notice that there is"\
" a new instruction available: \"add\""
#define L6_DESCRIPTION2 "Usage:\n\"add OP1, OP2\"\ndoes OP1 + OP2"\
" and stores the result in OP1"
#define L6_SELINS "Solve the challenge using the"\
" \"mov\" and \"add\" instructions"

#define L7_WELCOME "Welcome to Level 07" 
#define L7_DESCRIPTION1 "Did you know that"\
" \"mov\" \"add\" can be used with both operands being registers?"
#define L7_DESCRIPTION2 "Example:\n\"mov rbx, rax\"\n moves the value from"\
" \"rax\" to \"rbx\""

#define L8_WELCOME "You are now in Level 08" 
#define L8_DESCRIPTION1 "Both operands of instructions \"mov\" \"add\" can"\
" be the same register"
#define L8_DESCRIPTION2 "Example:\n\"add rax, rax\"\n does rax + rax"\
" and stores the result in rax"

#define L9_WELCOME "You have arrived to level 09"
#define L9_DESCRIPTION1 "There is a new instruction: \"jmp\""
#define L9_DESCRIPTION1 "There is a new instruction: \"jmp\""

SDL_Rect g_big_box;
SDL_Rect g_error_box;
SDL_Rect g_upper_box;
SDL_Rect g_center_box;
SDL_Rect g_lower_box;
SDL_Rect g_ins_box;
SDL_Rect g_code_box;
SDL_Rect g_stagebutton_box;

int g_lvl_msgs_size;
texture_array_t **g_lvl_msgs = NULL;

int g_gbl_msgs_size;
texture_array_t **g_gbl_msgs = NULL;

static int get_box_member(SDL_Rect *box, int member);

/* Function: tx_init_level_9_texts
 * -----------------------------------------------------------------------------
 * Creates the requiered text textures for a level
 * 
 * Arguments:
 *	Void.
 *	
 * Return:
 * 	Void.	
 */
void tx_init_level_9_texts()
{
	g_lvl_msgs_size = TX_L9_MAX;
	g_lvl_msgs = malloc(sizeof(texture_array_t*)*g_lvl_msgs_size);

	SDL_Rect r = dm_get_text_box_big();
	int text_h = dm_get_h_big_text();
	g_lvl_msgs[TX_L9_WELCOME] = dw_new_text_texture_by_h(r.w, text_h, C_BLACK, 
																    L9_WELCOME);
	r = dm_get_box_msg_wh();
	text_h = dm_get_h_msg();
	g_lvl_msgs[TX_L9_DESCRIPTION1] = dw_new_text_texture_by_h(r.w, text_h, 
										 			  C_BLACK, L9_DESCRIPTION1);
}

/* Function: tx_init_level_8_texts
 * -----------------------------------------------------------------------------
 * Creates the requiered text textures for a level
 * 
 * Arguments:
 *	Void.
 *	
 * Return:
 * 	Void.	
 */
void tx_init_level_8_texts()
{
	g_lvl_msgs_size = TX_L8_MAX;
	g_lvl_msgs = malloc(sizeof(texture_array_t*)*g_lvl_msgs_size);

	SDL_Rect r = dm_get_text_box_big();
	int text_h = dm_get_h_big_text();
	g_lvl_msgs[TX_L8_WELCOME] = dw_new_text_texture_by_h(r.w, text_h, C_BLACK, 
																    L8_WELCOME);
	r = dm_get_box_msg_wh();
	text_h = dm_get_h_msg();
	g_lvl_msgs[TX_L8_DESCRIPTION1] = dw_new_text_texture_by_h(r.w, text_h, 
										 			  C_BLACK, L8_DESCRIPTION1);
	g_lvl_msgs[TX_L8_DESCRIPTION2] = dw_new_text_texture_by_h(r.w, text_h, 
										 			  C_BLACK, L8_DESCRIPTION2);
}

/* Function: tx_init_level_7_texts
 * -----------------------------------------------------------------------------
 * Creates the requiered text textures for a level
 * 
 * Arguments:
 *	Void.
 *	
 * Return:
 * 	Void.	
 */
void tx_init_level_7_texts()
{
	g_lvl_msgs_size = TX_L7_MAX;
	g_lvl_msgs = malloc(sizeof(texture_array_t*)*g_lvl_msgs_size);

	SDL_Rect r = dm_get_text_box_big();
	int text_h = dm_get_h_big_text();
	g_lvl_msgs[TX_L7_WELCOME] = dw_new_text_texture_by_h(r.w, text_h, C_BLACK, 
																    L7_WELCOME);
	r = dm_get_box_msg_wh();
	text_h = dm_get_h_msg();
	g_lvl_msgs[TX_L7_DESCRIPTION1] = dw_new_text_texture_by_h(r.w, text_h, 
										 			  C_BLACK, L7_DESCRIPTION1);
	g_lvl_msgs[TX_L7_DESCRIPTION2] = dw_new_text_texture_by_h(r.w, text_h, 
										 			  C_BLACK, L7_DESCRIPTION2);

}

/* Function: tx_init_level_6_texts
 * -----------------------------------------------------------------------------
 * Creates the requiered text textures for a level
 * 
 * Arguments:
 *	Void.
 *	
 * Return:
 * 	Void.	
 */
void tx_init_level_6_texts()
{
	g_lvl_msgs_size = TX_L6_MAX;
	g_lvl_msgs = malloc(sizeof(texture_array_t*)*g_lvl_msgs_size);

	SDL_Rect r = dm_get_text_box_big();
	int text_h = dm_get_h_big_text();
	g_lvl_msgs[TX_L6_WELCOME] = dw_new_text_texture_by_h(r.w, text_h, C_BLACK, 
																    L6_WELCOME);
	r = dm_get_box_msg_wh();
	text_h = dm_get_h_msg();
	g_lvl_msgs[TX_L6_DESCRIPTION1] = dw_new_text_texture_by_h(r.w, text_h, 
										 			  C_BLACK, L6_DESCRIPTION1);
	g_lvl_msgs[TX_L6_DESCRIPTION2] = dw_new_text_texture_by_h(r.w, text_h, 
										 			  C_BLACK, L6_DESCRIPTION2);
	g_lvl_msgs[TX_L6_SELINS] = dw_new_text_texture_by_h(r.w, text_h, 
										 			  C_BLACK, L6_SELINS);
}

/* Function: tx_init_level_5_texts
 * -----------------------------------------------------------------------------
 * Creates the requiered text textures for a level
 * 
 * Arguments:
 *	Void.
 *	
 * Return:
 * 	Void.	
 */
void tx_init_level_5_texts()
{
	g_lvl_msgs_size = TX_L5_MAX;
	g_lvl_msgs = malloc(sizeof(texture_array_t*)*g_lvl_msgs_size);

	SDL_Rect r = dm_get_text_box_big();
	int text_h = dm_get_h_big_text();
	g_lvl_msgs[TX_L5_WELCOME] = dw_new_text_texture_by_h(r.w, text_h, C_BLACK, 
																    L5_WELCOME);
	r = dm_get_box_msg_wh();
	text_h = dm_get_h_msg();
	g_lvl_msgs[TX_L5_DESCRIPTION1] = dw_new_text_texture_by_h(r.w, text_h, 
										 			  C_BLACK, L5_DESCRIPTION1);
}

/* Function: tx_init_level_4_texts
 * -----------------------------------------------------------------------------
 * Creates the requiered text textures for a level
 * 
 * Arguments:
 *	Void.
 *	
 * Return:
 * 	Void.	
 */
void tx_init_level_4_texts()
{
	g_lvl_msgs_size = TX_L4_MAX;
	g_lvl_msgs = malloc(sizeof(texture_array_t*)*g_lvl_msgs_size);

	SDL_Rect r = dm_get_text_box_big();
	int text_h = dm_get_h_big_text();
	g_lvl_msgs[TX_L4_WELCOME] = dw_new_text_texture_by_h(r.w, text_h, C_BLACK, 
																    L4_WELCOME);
	r = dm_get_box_msg_wh();
	text_h = dm_get_h_msg();
	g_lvl_msgs[TX_L4_DESCRIPTION1] = dw_new_text_texture_by_h(r.w, text_h, 
										 			  C_BLACK, L4_DESCRIPTION1);
}

/* Function: tx_init_level_3_texts
 * -----------------------------------------------------------------------------
 * Creates the requiered text textures for a level
 * 
 * Arguments:
 *	Void.
 *	
 * Return:
 * 	Void.	
 */
void tx_init_level_3_texts()
{
	g_lvl_msgs_size = TX_L3_MAX;
	g_lvl_msgs = malloc(sizeof(texture_array_t*)*g_lvl_msgs_size);

	SDL_Rect r = dm_get_text_box_big();
	int text_h = dm_get_h_big_text();
	g_lvl_msgs[TX_L3_WELCOME] = dw_new_text_texture_by_h(r.w, text_h, C_BLACK, 
																    L3_WELCOME);
	r = dm_get_box_msg_wh();
	text_h = dm_get_h_msg();
	g_lvl_msgs[TX_L3_DESCRIPTION1] = dw_new_text_texture_by_h(r.w, text_h, 
										 			  C_BLACK, L3_DESCRIPTION1);
	g_lvl_msgs[TX_L3_DESCRIPTION2] = dw_new_text_texture_by_h(r.w, text_h, 
										 			  C_BLACK, L3_DESCRIPTION2);
	g_lvl_msgs[TX_L3_SELINS1] = dw_new_text_texture_by_h(r.w, text_h, 
										 			  C_BLACK, L3_SELINS1);
	g_lvl_msgs[TX_L3_DROPINS] = dw_new_text_texture_by_h(r.w, text_h, 
										 			  C_BLACK, L3_DROPINS);
	g_lvl_msgs[TX_L3_AVAILOPS1] = dw_new_text_texture_by_h(r.w, text_h, 
										 			  C_BLACK, L3_AVAILOPS1);
	g_lvl_msgs[TX_L3_SELRAX] = dw_new_text_texture_by_h(r.w, text_h, 
										 			  C_BLACK, L3_SELRAX);
	g_lvl_msgs[TX_L3_AVAILOPS2] = dw_new_text_texture_by_h(r.w, text_h, 
										 			  C_BLACK, L3_AVAILOPS2);
	g_lvl_msgs[TX_L3_SELIB] = dw_new_text_texture_by_h(r.w, text_h, 
										 			  C_BLACK, L3_SELIB);
	g_lvl_msgs[TX_L3_SELINS2] = dw_new_text_texture_by_h(r.w, text_h, 
										 			  C_BLACK, L3_SELINS2);
	g_lvl_msgs[TX_L3_READ] = dw_new_text_texture_by_h(r.w, text_h, 
										 			  C_BLACK, L3_READ);
}

/* Function: tx_init_level_2_texts
 * -----------------------------------------------------------------------------
 * Creates the requiered text textures for a level
 * 
 * Arguments:
 *	Void.
 *	
 * Return:
 * 	Void.	
 */
void tx_init_level_2_texts()
{
	g_lvl_msgs_size = TX_L2_MAX;
	g_lvl_msgs = malloc(sizeof(texture_array_t*)*g_lvl_msgs_size);

	SDL_Rect r = dm_get_text_box_big();
	int text_h = dm_get_h_big_text();
	g_lvl_msgs[TX_L2_WELCOME] = dw_new_text_texture_by_h(r.w, text_h, C_BLACK, 
																    L2_WELCOME);
	r = dm_get_box_msg_wh();
	text_h = dm_get_h_msg();
	g_lvl_msgs[TX_L2_DESCRIPTION] = dw_new_text_texture_by_h(r.w, text_h, 
										 			   C_BLACK, L2_DESCRIPTION);
	g_lvl_msgs[TX_L2_SELLAST] = dw_new_text_texture_by_h(r.w, text_h, 
										 			   C_BLACK, L2_SELLAST);
	g_lvl_msgs[TX_L2_DELINS] = dw_new_text_texture_by_h(r.w, text_h, 
										 			   C_BLACK, L2_DELINS);
	g_lvl_msgs[TX_L2_CHANGEOP] = dw_new_text_texture_by_h(r.w, text_h, 
										 			   C_BLACK, L2_CHANGEOP);
	g_lvl_msgs[TX_L2_SELIB] = dw_new_text_texture_by_h(r.w, text_h, 
										 			   C_BLACK, L2_SELIB);
	g_lvl_msgs[TX_L2_MOVINS] = dw_new_text_texture_by_h(r.w, text_h, 
										 			   C_BLACK, L2_MOVINS);
	g_lvl_msgs[TX_L2_PRESSPLAY] = dw_new_text_texture_by_h(r.w, text_h, 
										 			   C_BLACK, MSG_PRESSPLAY);

}

/* Function: tx_init_level_1_texts
 * -----------------------------------------------------------------------------
 * Creates the requiered text textures for a level
 * 
 * Arguments:
 *	Void.
 *	
 * Return:
 * 	Void.	
 */
void tx_init_level_1_texts()
{
	g_lvl_msgs_size = TX_L1_MAX;
	g_lvl_msgs = malloc(sizeof(texture_array_t*)*g_lvl_msgs_size);

	SDL_Rect r = dm_get_text_box_big();
	int text_h = dm_get_h_big_text();
	g_lvl_msgs[TX_L1_WELCOME] = dw_new_text_texture_by_h(r.w, text_h, C_BLACK, 
																    L1_WELCOME);
	r = dm_get_box_msg_wh();
	text_h = dm_get_h_msg();
	g_lvl_msgs[TX_L1_DESCRIPTION] = dw_new_text_texture_by_h(r.w, text_h, 
										 			   C_BLACK, L1_DESCRIPTION);
	g_lvl_msgs[TX_L1_OBJ1] = dw_new_text_texture_by_h(r.w, text_h, 
										 				C_BLACK, L1_OBJ1);
	g_lvl_msgs[TX_L1_OBJ2] = dw_new_text_texture_by_h(r.w, text_h, 
										 				C_BLACK, L1_OBJ2);
	g_lvl_msgs[TX_L1_OBJ3] = dw_new_text_texture_by_h(r.w, text_h, 
										 				C_BLACK, L1_OBJ3);
	r = dm_get_text_box_ins();
	g_lvl_msgs[TX_L1_SELINS1] = dw_new_text_texture_by_h(r.w, text_h, 
										 				C_BLACK, L1_SELINS1);
	g_lvl_msgs[TX_L1_DROPINS1] = dw_new_text_texture_by_h(r.w, text_h, 
										 				C_BLACK, L1_DROPINS1);
	g_lvl_msgs[TX_L1_SELOP1] = dw_new_text_texture_by_h(r.w, text_h, 
										 				C_BLACK, L1_SELOP1);
	g_lvl_msgs[TX_L1_SELOP2] = dw_new_text_texture_by_h(r.w, text_h, 
										 				C_BLACK, L1_SELOP2);
	g_lvl_msgs[TX_L1_PLAYTUT] = dw_new_text_texture_by_h(r.w, text_h, 
										 				C_BLACK, L1_PLAYTUT);
	g_lvl_msgs[TX_L1_SELINS2] = dw_new_text_texture_by_h(r.w, text_h, 
										 				C_BLACK, L1_SELINS2);
	r = dm_get_text_box_error();
	g_lvl_msgs[TX_L1_ERROR] = dw_new_text_texture_by_h(r.w, text_h, 
										 				C_BLACK, L1_ERROR);
	r = dm_get_text_box_ins();
	g_lvl_msgs[TX_L1_DROPINS2] = dw_new_text_texture_by_h(r.w, text_h, 
										 				C_BLACK, L1_DROPINS2);
	g_lvl_msgs[TX_L1_SELOP1_2] = dw_new_text_texture_by_h(r.w, text_h, 
										 				C_BLACK, L1_SELOP1_2);
	g_lvl_msgs[TX_L1_SELOP2_2] = dw_new_text_texture_by_h(r.w, text_h, 
										 				C_BLACK, L1_SELOP2_2);
	g_lvl_msgs[TX_L1_PRESSPLAY] = dw_new_text_texture_by_h(r.w, text_h, 
										 			    C_BLACK, MSG_PRESSPLAY);
	g_lvl_msgs[TX_L1_CONGRATS] = dw_new_text_texture_by_h(r.w, text_h, 
										 			    C_BLACK, L1_CONGRATS);
}

/* Function: tx_init_global_msgs
 * -----------------------------------------------------------------------------
 * Creates the textures of global messages that are used across several levels.
 * 
 * Arguments:
 *	Void.
 *	
 * Return:
 *	Void.	
 */
void tx_init_global_msgs()
{
	g_gbl_msgs_size = 3;
	g_gbl_msgs = malloc(sizeof(texture_array_t*)*g_gbl_msgs_size);

	SDL_Rect r = dm_get_text_box_big();
	int text_h = dm_get_h_bottom_msg();
	
	g_gbl_msgs[TX_MSG_CLICKANY] = dw_new_text_texture_by_h(r.w, text_h, C_GREY, 
															MSG_CLICKANY);
	g_gbl_msgs[TX_MSG_PRESSPLAY] = dw_new_text_texture_by_h(r.w, text_h, C_GREY, 
															MSG_PRESSPLAY);
	g_gbl_msgs[TX_MSG_PRESSBACK] = dw_new_text_texture_by_h(r.w, text_h, C_GREY, 
															MSG_PRESSBACK);
}

/* Function: tx_free_level_text_texture
 * -----------------------------------------------------------------------------
 * Free the level textures when a level is finished
 * 
 * Arguments:
 *	Void.
 *	
 * Return:
 * 	Void.	
 */
void tx_free_level_text_textures()
{
	for (int i = 0; i < g_lvl_msgs_size; i++){
		dw_free_texture_array(g_lvl_msgs[i]);
	}
	g_lvl_msgs_size = 0;
}

/* Function: tx_init_text_boxes
 * -----------------------------------------------------------------------------
 * Set the properties of the text boxes of the game
 * 
 * Arguments:
 *	level_id: identifie of the level of which the textures will be created
 *	
 * Return:
 *	
 */
void tx_init_text_boxes()
{
	g_big_box = dm_get_text_box_big();
	g_error_box = dm_get_text_box_error();	
	g_upper_box = dm_get_text_box_upper();	
	g_center_box = dm_get_text_box_center();	
	g_lower_box = dm_get_text_box_lower();	
	g_ins_box = dm_get_text_box_ins();
	g_code_box = dm_get_text_box_code();
	g_stagebutton_box = dm_get_text_box_stagebutton();
}

/* Function: get_box_member
 * -----------------------------------------------------------------------------
 * Returns the property member of a given text box by a pointer to the bos
 * 
 * Arguments:
 *	box: pointer of the box that the member will be recovered
 *	member: the member of the box that will be recovered.
 *	
 * Return:
 *	member of the box recovered.
 */
static int get_box_member(SDL_Rect *box, int member){
	assert(box != NULL && "Box pointer is NULL");
	assert(member > MEMBER_MIN && member < MEMBER_MAX && 
		   "Invalid member");
	int retval;
	switch(member){
		case MEMBER_X:
			retval = box->x;	
			break;
		case MEMBER_Y:
			retval = box->y;
			break;
		case MEMBER_W:
			retval = box->w;
			break;
		case MEMBER_H:
			retval = box->h;
			break;
	}
	return retval;
}


/* Function: tx_get_text_box_member
 * -----------------------------------------------------------------------------
 * Returns the property member of a given text box
 * 
 * Arguments:
 *	Void.
 *	
 * Return:
 *	Void.
 */
int tx_get_text_box_member(int text_box_id, int member)
{
	assert(text_box_id > TX_BOX_MIN && text_box_id < TX_BOX_MAX && 
		   "Invalid text box");
	assert(member > MEMBER_MIN && member < MEMBER_MAX && 
		   "Invalid member");
	int retval;
	switch(text_box_id){
		case TX_INS_BOX:
			retval = get_box_member(&g_ins_box, member);
			break;
		case TX_UPPER_BOX:
			retval = get_box_member(&g_upper_box, member);
			break;
		case TX_LOWER_BOX:
			retval = get_box_member(&g_lower_box, member);
			break;
		case TX_CODE_BOX:
			retval = get_box_member(&g_code_box, member);
			break;
		case TX_STAGEBUTTON_BOX:
			retval = get_box_member(&g_stagebutton_box, member);
			break;
		case TX_BIG_BOX:
			retval = get_box_member(&g_big_box, member);
			break;
		case TX_ERROR_BOX:
			retval = get_box_member(&g_error_box, member);
			break;
		}
	return retval;
}



/* Function: tx_bottom_msg
 * -----------------------------------------------------------------------------
 * Display a message in the bottom of a give box
 *
 * Arguments:
 *	r: The rectangle box where the message will be shown
 *	msg_id: The messages that are displayed at the botttom are global
 *	
 * Return:
 *	Void.
 */
void tx_bottom_msg(int pos, int msg_id)
{
	assert(msg_id >= 0 && msg_id < g_gbl_msgs_size && "Invalid msg_id");
	SDL_Rect b;
	int text_h;
	SDL_Color color;
	texture_array_t *a = g_gbl_msgs[msg_id];
	switch(pos){
		case TX_INS_BOX:
			b = g_ins_box; 
			b.y = g_ins_box.y + g_ins_box.h*5/6;
			b.h = g_ins_box.h/6;
			text_h = dm_get_h_bottom_msg();
			break;
		case TX_UPPER_BOX:
			b = g_upper_box; 
			b.y = g_upper_box.y + g_upper_box.h*5/6;
			b.h = g_upper_box.h/6;
			text_h = dm_get_h_bottom_msg();
			break;
		case TX_CENTER_BOX:
			b = g_center_box; 
			b.y = g_center_box.y + g_center_box.h*5/6;
			b.h = g_center_box.h/6;
			text_h = dm_get_h_bottom_msg();
			break;
		case TX_LOWER_BOX:
			b = g_lower_box; 
			b.y = g_lower_box.y + g_lower_box.h*5/6;
			b.h = g_lower_box.h/6;
			text_h = dm_get_h_bottom_msg();
			break;
		case TX_CODE_BOX:
			b = g_code_box; 
			text_h = dm_get_h_msg();
			break;
		case TX_STAGEBUTTON_BOX:
			b = g_stagebutton_box; 
			text_h = dm_get_h_msg();
			break;
		case TX_BIG_BOX:
			b = g_big_box;
			b.y = g_big_box.y + g_big_box.h*5/6;
			b.h = g_big_box.h/6;
			text_h = dm_get_h_msg();
			break;
		case TX_ERROR_BOX:
			b = g_error_box; 
			b.y = g_error_box.y + g_error_box.h*5/6;
			b.h = g_error_box.h/6;
			text_h = dm_get_h_bottom_msg();
			break;
	}
	dw_draw_wrapped_texture_by_h(b, text_h, a);
}

/* Function: tx_text_box
 * -----------------------------------------------------------------------------
 * This function displays a message box in differents part of the screen 
 * according to an identifies
 *
 * Arguments:
 *	pos: The position id of where is gonna be displayed.
 *	msg_id: The id of the message that will be shown
 *  bootom_msg_id: id of the bottom message of the box if any
 *	
 * Return:
 *	Void.
 */
void tx_text_box(int pos, int msg_id)
{
	assert(msg_id >= 0 && msg_id < g_lvl_msgs_size && "Invalid msg_id");
	SDL_Rect b;
	int text_h;
	SDL_Color color;
	texture_array_t *a = g_lvl_msgs[msg_id];
	switch(pos){
		case TX_INS_BOX:
			b = g_ins_box; 
			text_h = dm_get_h_msg();
			break;
		case TX_UPPER_BOX:
			b = g_upper_box; 
			text_h = dm_get_h_msg();
			break;
		case TX_CENTER_BOX:
			b = g_center_box; 
			text_h = dm_get_h_msg();
			break;
		case TX_LOWER_BOX:
			b = g_lower_box; 
			text_h = dm_get_h_msg();
			break;
		case TX_CODE_BOX:
			b = g_code_box; 
			text_h = dm_get_h_msg();
			break;
		case TX_STAGEBUTTON_BOX:
			b = g_stagebutton_box; 
			text_h = dm_get_h_msg();
			break;
		case TX_BIG_BOX:
			b = g_big_box; 
			text_h = dm_get_h_big_text();
			break;
		case TX_ERROR_BOX:
			b = g_error_box; 
			text_h = dm_get_h_msg();
			break;
	}
	dw_draw_filled_rectangle(b, C_WHITE, C_WHITE);
	dw_draw_wrapped_texture_by_h(b, text_h, a);
}

