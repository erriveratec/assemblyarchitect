#include <assert.h>
#include <stdbool.h>
#include "levels_lv.h"
#include "list.h"
#include "aux.h"
#include "dbg.h"
#include "buffers_bf.h"
#include "dimensions.h"
#include "registers_rg.h"

#define TUT_TEXT_SELECT_INSTRUCTION "Select and drag an instruction from the" \
									"instruction box"

#define TUT_TEXT_X INS_BOX_X
#define TUT_TEXT_Y SCREEN_HEIGHT/2

#define TUT_BOX_W INS_BOX_W
#define TUT_BOX_H SCREEN_HEIGHT/8
#define TUT_BOX_X INS_BOX_X
#define TUT_BOX_Y SCREEN_HEIGHT/2

#define LEVEL_LINE_W 4

// At the end of the code execution, the output buffer must be compared 
// against this list.

static List *win_list = NULL;

static int level_instructions_limit;

List *get_win_list();
void add_to_win_list(int value, int type);
static void set_level_1_win_list();
static void set_level_2_win_list();
static void set_level_3_win_list();
static void set_level_4_win_list();
static void set_level_5_win_list();
static void set_level_6_win_list();

/* Function: lv_level_1_tutorial
 * -----------------------------------------------------------------------------
 * This function handles the tutorial part of level 1
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
int lv_level_1_tutorial()
{
	dw_draw_rectangle(TUT_BOX_X, TUT_BOX_Y, TUT_BOX_W, TUT_BOX_H, COLOR_WHITE);
	return 0;
}

/* Function: lv_get_level_instructions_limit
 * -----------------------------------------------------------------------------
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	The limit number of instructions for the level
 */
int lv_get_level_instructions_limit()
{
	return level_instructions_limit;
}

/* Function: lv_set_level_instructions_limit
 * -----------------------------------------------------------------------------
 * Arguments:
 * 	Limit: The quantity of the instructions that the level will allow
 *	
 * Return:
 *	void
 */
void lv_set_level_instructions_limit(int limit)
{
	level_instructions_limit = limit;
}


/* Function: lv_generate_win_condition_list
 * -----------------------------------------------------------------------------
 * Arguments:
 * 	level: The level of the win condition that will be generated.
 *	
 * Return:
 *	void
 */
void lv_generate_win_condition_list(int level)
{
	switch (level){
		case LV_LEVEL_1:
			set_level_1_win_list();
			break;
		case LV_LEVEL_2:
			set_level_2_win_list();
			break;
		case LV_LEVEL_3:
			set_level_3_win_list();
			break;
		case LV_LEVEL_4:
			set_level_4_win_list();
			break;
		case LV_LEVEL_5:
			set_level_5_win_list();
			break;
		case LV_LEVEL_6:
			set_level_6_win_list();
			break;
		default:
			break;
	}

}
/* Function: add_to_win_list
 * -----------------------------------------------------------------------------
 * Arguments:
 * 	value: The input value to be added.
 *	type: The type of input to be added.
 *	
 * Return:
 *	void
 */
void add_to_win_list(int value, int type)
{
	List *win = get_win_list();

	assert(win != NULL && "win pointer is NULL");
	
	value_box_t *new_input = malloc(sizeof(value_box_t));
	check_mem(new_input);

	new_input->value = value;
	new_input->type = type;

	List_push(win, new_input);

error:
	return;
}

/* Function: lv_destroy_win_list
 *------------------------------------------------------------------------------
 * Destroys the win list when finishing a level.
 *
 * Arguments:
 *	None.
 *
 * Return:
 *	Void.
 *
 */
void lv_destroy_win_list()
{
	List_clear_destroy(win_list);
	win_list = NULL;
}


/* Function: lv_create_win_list
 *------------------------------------------------------------------------------
 * This function creates and initializes the input list.
 *
 * Arguments:
 *	None.
 *
 * Return:
 *	Void.
 *
 */
void lv_create_win_list()
{
	assert(win_list == NULL && "The win list is not NULL");
	win_list = List_create();
	check_mem(win_list);

error:
	return;
}

/* Function: lv_reset_win_list
 *------------------------------------------------------------------------------
 * Arguments:
 *	void.
 *
 * Return:
 *	void.
 */
void lv_reset_win_list()
{
	List_clear_destroy(win_list);
	win_list = NULL;
	lv_create_win_list();
}

/* Function: get_win_list
 *------------------------------------------------------------------------------
 * Arguments:
 *	None.
 *
 * Return:
 *	Pointer to input list.
 */
List *get_win_list()
{
	return win_list;
}

/* Function: lv_print_win_list
 *------------------------------------------------------------------------------
 * Arguments:
 *	None.
 *
 * Return:
 *	
 */
void lv_print_win_list()
{
	List *win_list = get_win_list();

	assert(win_list != NULL && "Win list pointer is NULL");

	int win_list_size = List_count(win_list);

	printf("The size of the list is %d\n", win_list_size);	

	LIST_FOREACH(win_list, first, next, cur){
		value_box_t *cur_input = cur->value;
		printf("List value: %d\n", cur_input->value);
	}
}

/* Function: set_level_1_win_list
 *------------------------------------------------------------------------------
 * Arguments:
 *	None.
 *
 * Return:
 *	Pointer to the object that will be assigned as a default reg operand.
 */
static void set_level_1_win_list()
{
	List *input_list = get_input_list();
	List *win_list = get_win_list();

	assert(input_list != NULL && "Input list pointer is NULL");
	assert(win_list != NULL && "Win list pointer is NULL");

	int input_list_size = List_count(input_list);
	int win_list_size = List_count(win_list);

	assert(input_list_size > 0 && "The size of the input list is incorrect");
	assert(win_list_size == 0 && "The win list has elements");

	LIST_FOREACH(input_list, first, next, cur){
		value_box_t *cur_input = cur->value;
		value_box_t *new_win = malloc(sizeof(value_box_t));
		new_win->value = cur_input->value;
		new_win->type = cur_input->type;
		List_push(win_list, new_win);
	}
}

/* Function: set_level_2_win_list
 *------------------------------------------------------------------------------
 * Arguments:
 *	None.
 *
 * Return:
 *	Pointer to the object that will be assigned as a default reg operand.
 */
static void set_level_2_win_list()
{
	List *input_list = get_input_list();
	List *win_list = get_win_list();

	assert(input_list != NULL && "Input list pointer is NULL");
	assert(win_list != NULL && "Win list pointer is NULL");

	int input_list_size = List_count(input_list);
	int win_list_size = List_count(win_list);

	assert(input_list_size > 0 && "The size of the input list is incorrect");
	assert(win_list_size == 0 && "The win list has elements");

	LIST_FOREACH(input_list, first, next, cur){
		value_box_t *cur_input = cur->value;
		value_box_t *new_win = malloc(sizeof(value_box_t));
		new_win->value = cur_input->value;
		new_win->type = cur_input->type;
		List_unshift(win_list, new_win);
	}
}

/* Function: set_level_3_win_list
 *------------------------------------------------------------------------------
 * Arguments:
 *	None.
 *
 * Return:
 *	Pointer to the object that will be assigned as a default reg operand.
 */
static void set_level_3_win_list()
{
	List *input_list = get_input_list();
	List *win_list = get_win_list();

	assert(input_list != NULL && "Input list pointer is NULL");
	assert(win_list != NULL && "Win list pointer is NULL");

	int input_list_size = List_count(input_list);
	assert(input_list_size % 2 == 0 && "For this challenge the list size must\
		   be an even number");
	
	int win_list_size = List_count(win_list);
	assert(input_list_size > 0 && "The size of the input list is incorrect");
	assert(win_list_size == 0 && "The win list has elements");

	int i = 1;
	int prev_val = 0;
	LIST_FOREACH(input_list, first, next, cur){
		value_box_t *cur_input = cur->value;
		
		if (i % 2 == 0){
			value_box_t *new_win = malloc(sizeof(value_box_t));
			new_win->value = cur_input->value + prev_val;
			new_win->type = cur_input->type;
			List_push(win_list, new_win);
		}
		prev_val = cur_input->value;
		i++;
	}
}

/* Function: set_level_4_win_list
 *------------------------------------------------------------------------------
 * Arguments:
 *	None.
 *
 * Return:
 *	Void.
 */
static void set_level_4_win_list()
{
	List *input_list = get_input_list();
	List *win_list = get_win_list();

	assert(input_list != NULL && "Input list pointer is NULL");
	assert(win_list != NULL && "Win list pointer is NULL");

	int input_list_size = List_count(input_list);
	
	int win_list_size = List_count(win_list);
	assert(input_list_size > 0 && "The size of the input list is incorrect");
	assert(win_list_size == 0 && "The win list has elements");

	
	LIST_FOREACH(input_list, first, next, cur){
		value_box_t *cur_input = cur->value;
		
		value_box_t *new_win = malloc(sizeof(value_box_t));
		new_win->value = 5*cur_input->value;
		new_win->type = cur_input->type;
		List_push(win_list, new_win);
	}
}

/* Function: set_level_5_win_list
 *------------------------------------------------------------------------------
 * Arguments:
 *	None.
 *
 * Return:
 *	Void.
 */
static void set_level_5_win_list()
{
	List *input_list = get_input_list();
	List *win_list = get_win_list();

	assert(input_list != NULL && "Input list pointer is NULL");
	assert(win_list != NULL && "Win list pointer is NULL");

	int input_list_size = List_count(input_list);
	
	int win_list_size = List_count(win_list);
	assert(input_list_size > 0 && "The size of the input list is incorrect");
	assert(win_list_size == 0 && "The win list has elements");

	
	LIST_FOREACH(input_list, first, next, cur){
		value_box_t *cur_input = cur->value;
		
		value_box_t *new_win = malloc(sizeof(value_box_t));
		new_win->value = 8*cur_input->value;
		new_win->type = cur_input->type;
		List_push(win_list, new_win);
	}
}

/* Function: set_level_6_win_list
 *------------------------------------------------------------------------------
 * Arguments:
 *	None.
 *
 * Return:
 *	Void.
 */
static void set_level_6_win_list()
{
	List *input_list = get_input_list();
	List *win_list = get_win_list();

	assert(input_list != NULL && "Input list pointer is NULL");
	assert(win_list != NULL && "Win list pointer is NULL");

	int input_list_size = List_count(input_list);
	
	int win_list_size = List_count(win_list);
	assert(input_list_size > 0 && "The size of the input list is incorrect");
	assert(win_list_size == 0 && "The win list has elements");

	
	LIST_FOREACH(input_list, first, next, cur){
		value_box_t *cur_input = cur->value;
		value_box_t *new_win = malloc(sizeof(value_box_t));
		new_win->value = cur_input->value;
		new_win->type = cur_input->type;
		List_push(win_list, new_win);
	}
}

/* Function: lv_evaluate_output_correctness
 *------------------------------------------------------------------------------
 * Evaluates the correctness of the output as values are being added
 *
 * Arguments:
 *	None.
 *
 * Return:
 *	boolean stating if the output buffer contents are correct
 */
bool lv_evaluate_output_correctness()
{
	List *output_list = get_output_list();
	List *win_list = get_win_list();

	assert(output_list != NULL && "Output list pointer is NULL");
	assert(win_list != NULL && "Win list pointer is NULL");

	int output_list_size = List_count(output_list);
	int win_list_size = List_count(win_list);

	assert(win_list_size > 0 && "The win list has no elements");

	if (output_list_size == 0){
		return true;
	}

	ListNode *win_node = win_list->first;
	int i = 0;
	LIST_FOREACH(output_list, first, next, cur){
		if (i == output_list_size){
			break;
		}
		value_box_t *output = cur->value;
		value_box_t *win_val = win_node->value;
		if (output->value != win_val->value){
			return false;
		}
		win_node = win_node->next;
		i++;
	}
	return true;
}

/* Function: lv_level_drawings
 *------------------------------------------------------------------------------
 * This functions draws the specific characteristics of a level, like where
 * each of the avatars are able to reack.
 *
 * Arguments:
 *	level: The spefici level that is goin to have the drawing.
 *
 * Return:
 *	Void.
 */
void lv_level_drawings(int level)
{
	assert(level < LV_LEVEL_MAX && level > LV_LEVEL_MIN && 
			          								"Invalid level value");

	int x = REG_BOX_X + rg_get_register_box_member(MEMBER_W);
	int y = SCREEN_HEIGHT/2 - LEVEL_LINE_W/2; 
	int w = SCREEN_WIDTH - x;
	int h = LEVEL_LINE_W;
	
	dw_draw_filled_rectangle(x, y, w, h, COLOR_GREY, COLOR_GREY);
}


/* Function: lv_check_if_win
 *------------------------------------------------------------------------------
 * Evaluates the whole output list against the win list to verify if the 
 * result is correct. Does not verifies if there are still elements in the
 * output list. 
 *
 * Arguments:
 *	None.
 *
 * Return:
 *	True if the win condition is met, false if otherwise
 */
bool lv_check_if_win()
{
	List *output_list = get_output_list();
	List *win_list = get_win_list();

	assert(output_list != NULL && "Output list pointer is NULL");
	assert(win_list != NULL && "Win list pointer is NULL");

	int output_list_size = List_count(output_list);
	int win_list_size = List_count(win_list);

	assert(win_list_size > 0 && "The win list has no elements");

	if (output_list_size != win_list_size || output_list_size == 0){
		return false;
	}

	ListNode *win_node = win_list->first;

	LIST_FOREACH(output_list, first, next, cur){
		value_box_t *output = cur->value;
		value_box_t *win_val = win_node->value;

		if (output->value != win_val->value){
			return false;
		}
		win_node = win_node->next;
	}
	return true;
}
