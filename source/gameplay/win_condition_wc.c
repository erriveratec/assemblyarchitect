#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "list.h"
#include "win_condition_wc.h"
#include "buffers_bf.h"
#include "aux.h"
#include "buffers_bf.h"


//Strings for the win condition
#define STR_WIN1 "WIN1"
#define STR_WIN2 "WIN2"
#define STR_WIN3 "WIN3"
#define STR_WIN4 "WIN4"
#define STR_WIN5 "WIN5"

#define WIN_CONDITION_LENGTH 30

static List *g_expected_output = NULL;

static bool wc_add_expected_value(int value, int type);
static void win1_move_input_to_output(int rep, int mul,int sum, bool reversed);
static void win2_add_inputs_in_groups(int grp_size, bool between, int in_val);
static void win3_move_input_to_output_stop(bool target, int tval, int stop);
static void win4_count_values_till_stop(int element, int stop);
static void win5_move_input_to_output_add_dec_ofs(int dec);
static void set_win_condition(char *win_condition);
static char level_win_condition[WIN_CONDITION_LENGTH];

/* Function: lv_reset_win_condition
 * -----------------------------------------------------------------------------
 * This function resets the win condition when the player resets the level
 *  
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
void lv_reset_level_win_condition()
{
	set_win_condition(level_win_condition);
}

/* Function: lv_level_win_condition
 * -----------------------------------------------------------------------------
 * Copies the win condition to the level variable and creates the win condition
 *  
 * Arguments:
 * 	Void.
 *	
 * Return:
 *	Void.
 */
void lv_set_level_win_condition_text(char *win_condition)
{
	strcpy(level_win_condition, win_condition);
}

/* Function: set_win_condition
 * -----------------------------------------------------------------------------
 * This function creates the win condition according to what is in the levels
 * file
 * Nomeclature of the win conditions
 * WIN1: win1_move_input_to_output(rep, mul, rev) 
 * WIN2: win2_add_inputs_in_groups(group_size)
 *
 * Arguments:
 * 	text: The text with the description of the win condition.
 *	
 * Return:
 *	void
 */
static void set_win_condition(char *win_condition)
{
	assert(win_condition != NULL && "NULL win condition text");
	char win_condition_cpy[WIN_CONDITION_LENGTH];
	strcpy(win_condition_cpy, win_condition);
	char *saveptr1;
	char *delim = ax_char_space;
	char *win_cond;
	
	win_cond =  strtok_r(win_condition_cpy, delim, &saveptr1);
	if (strstr(win_cond, STR_WIN1) != NULL){
		char *rep_text = strtok_r(NULL, delim, &saveptr1);
		int rep = atoi(rep_text);
		char *mul_text = strtok_r(NULL, delim, &saveptr1);
		int mul = atoi(mul_text);
		char *sum_text = strtok_r(NULL, delim, &saveptr1);
		int sum = atoi(sum_text);
		char *reversed = strtok_r(NULL, delim, &saveptr1);
		bool rev;
		if (strstr(reversed, "true") != NULL){
			rev = true;
		} else if (strstr(reversed, "false") != NULL){
			rev = false;
		}
		win1_move_input_to_output(rep, mul, sum, rev);
	} else if (strstr(win_cond, STR_WIN2) != NULL){
		char *group_size_text = strtok_r(NULL, delim, &saveptr1);
		int group_size = atoi(group_size_text);
		char *bet_text = strtok_r(NULL, delim, &saveptr1);
		bool between;
		if (strstr(bet_text, "true") != NULL){
			between = true;
		} else if (strstr(bet_text, "false") != NULL){
			between = false;
		}
		char *in_val_text = strtok_r(NULL, delim, &saveptr1);
		int in_val = atoi(in_val_text);
		win2_add_inputs_in_groups(group_size, between, in_val);
	} else if (strstr(win_cond, STR_WIN3) != NULL){
		char *target_text = strtok_r(NULL, delim, &saveptr1);
		int target = atoi(target_text);
		char *tval_text = strtok_r(NULL, delim, &saveptr1);
		int tval = atoi(tval_text);
		char *stop_text = strtok_r(NULL, delim, &saveptr1);
		int stop = atoi(stop_text);
		win3_move_input_to_output_stop(target, tval, stop);
	} else if (strstr(win_cond, STR_WIN4) != NULL){
		char *element_text = strtok_r(NULL, delim, &saveptr1);
		int element = atoi(element_text);
		char *stop_text = strtok_r(NULL, delim, &saveptr1);
		int stop = atoi(stop_text);
		win4_count_values_till_stop(element, stop);
	} else if (strstr(win_cond, STR_WIN5) != NULL){
		char *dec_text = strtok_r(NULL, delim, &saveptr1);
		int dec = atoi(dec_text);
		win5_move_input_to_output_add_dec_ofs(dec);
	}

	return;
}


/* Function: lv_chk_correct_output
 *------------------------------------------------------------------------------
 * Evaluates the correctness of the output as values are being added
 *
 * Arguments:
 *	None.
 *
 * Return:
 *	boolean stating if the output buffer contents are correct
 */
bool lv_chk_correct_output()
{
	List *output_list = bf_get_output_list();
	List *win_list = wc_get_expected_output();

	assert(output_list != NULL && "Output list pointer is NULL");
	assert(win_list != NULL && "Win list pointer is NULL");

	int output_list_size = List_count(output_list);
	int win_list_size = List_count(win_list);

	assert(win_list_size > 0 && "The win list has no elements");

	if (output_list_size > win_list_size){
		return false;
	}
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

/* Function: win1_move_input_to_output
 *------------------------------------------------------------------------------
 * Generates a win condition that is achieved by moving the elements from the
 * input buffer to the output buffer in order. Can apply repetitions and 
 * transformations to the output list if needed as its implementation is easy.
 *
 * Arguments:
 *	rep: number of times a element of the IB will be copied to the OB.
 *  mul: multiplication transformation to the output buffer.
 *	sum: value that will be added to the output
 *  reversed: if the order of the inputs is reversed
 *
 * Return:
 *	Void.
 */
static void win1_move_input_to_output(int rep, int mul, int sum, bool rev)
{
	List *input_list = get_input_list();
	List *win_list = wc_get_expected_output();

	assert(rep > 0 && "The number of repetitions is less than 1");
	assert(mul > 0 && "The mul factor es less than 1");
	assert(input_list != NULL && "Input list pointer is NULL");
	assert(win_list != NULL && "Win list pointer is NULL");

	int input_list_size = List_count(input_list);
	int win_list_size = List_count(win_list);

	assert(input_list_size > 0 && "The size of the input list is incorrect");
	assert(win_list_size == 0 && "The win list has elements");

	LIST_FOREACH(input_list, first, next, cur){
		value_box_t *cur_input = cur->value;
		value_box_t *new_win; 
		for (int i = 0; i < rep; i++){
			new_win = malloc(sizeof(value_box_t));
			new_win->value = mul*cur_input->value + sum;
			new_win->type = cur_input->type;
			if (rev == false){
				List_push(win_list, new_win);
			} else if (rev == true){
				List_unshift(win_list, new_win);
			}
		}
	}
}

/* Function: win3_move_input_to_output_stop
 *------------------------------------------------------------------------------
 * Generates a win condition that is achieved by moving the elements from the
 * input buffer to the output buffer up to a stop element
 *
 * Arguments:
 *  target: if activated, only specific values should be move the OB
 * 	target_val: 
 *	stop: stop element that will be used to stop the movement
 *
 * Return:
 *	Void.
 */
static void win3_move_input_to_output_stop(bool target, int tval, int stop)
{
	List *input_list = get_input_list();
	List *win_list = wc_get_expected_output();

	int input_list_size = List_count(input_list);
	int win_list_size = List_count(win_list);

	assert(input_list_size > 0 && "The size of the input list is incorrect");
	assert(win_list_size == 0 && "The win list has elements");

	LIST_FOREACH(input_list, first, next, cur){
		value_box_t *cur_input = cur->value;
		value_box_t *new_win; 
		if (cur_input->value == stop){
			break;
		} else if (target == true && cur_input->value == tval){
			new_win = malloc(sizeof(value_box_t));
			new_win->value = cur_input->value;
			new_win->type = cur_input->type;
			List_push(win_list, new_win);
		} else if (target == false){
			new_win = malloc(sizeof(value_box_t));
			new_win->value = cur_input->value;
			new_win->type = cur_input->type;
			List_push(win_list, new_win);
		} 
	}
}

/* Function: win4_count_values_till_stop
 *------------------------------------------------------------------------------
 * Generates a win condition that is achieved by counting the number of 
 * appearances of an element until a stop  condition
 *
 * Arguments:
 *	element: element that will be counted
 *	stop: stop element that will be used to stop the movement
 *
 * Return:
 *	Void.
 */
static void win4_count_values_till_stop(int element, int stop)
{
	List *input_list = get_input_list();
	List *win_list = wc_get_expected_output();

	int input_list_size = List_count(input_list);
	int win_list_size = List_count(win_list);

	assert(input_list_size > 0 && "The size of the input list is incorrect");
	assert(win_list_size == 0 && "The win list has elements");

	value_box_t *new_win = malloc(sizeof(value_box_t));
	LIST_FOREACH(input_list, first, next, cur){
		value_box_t *cur_input = cur->value;
		if (cur_input->value == element){
			new_win->value++;
		} else if (cur_input->value == stop){
			new_win->type = cur_input->type;
			List_push(win_list, new_win);
			break;
		}
	}
}

/* Function: win5_move_input_to_output_add_dec_ofs
 *------------------------------------------------------------------------------
 * Generates a win condition that is generating moving the inputs to the outputs
 * and adding a decreasing offset
 *
 * Arguments:
 *	dec: starting point of the dreceasing offset
 *
 * Return:
 *	Void.
 */
static void win5_move_input_to_output_add_dec_ofs(int dec)
{
	List *input_list = get_input_list();
	List *win_list = wc_get_expected_output();

	int input_list_size = List_count(input_list);
	int win_list_size = List_count(win_list);

	assert(input_list_size > 0 && "The size of the input list is incorrect");
	assert(win_list_size == 0 && "The win list has elements");

	LIST_FOREACH(input_list, first, next, cur){
		value_box_t *cur_input = cur->value;
		value_box_t *new_win; 
		new_win = malloc(sizeof(value_box_t));
		new_win->value = cur_input->value + dec;
		new_win->type = cur_input->type;
		List_push(win_list, new_win);
		dec--;
	}
}

/* Function: win2_add_inputs_in_groups
 *------------------------------------------------------------------------------
 * The solution of the challenge will be achieved is the player adds the inputs
 * in groups
 *
 * Arguments:
 *	grp_size: The size of the input group tha will be added
 *  between: If active the function inserts a value between each output
 *	in_value: Value that will be inserted between each output
 *
 * Return:
 *	Void.
 */
static void win2_add_inputs_in_groups(int grp_size, bool between, int in_val)
{
	List *input_list = get_input_list();
	List *win_list = wc_get_expected_output();

	assert(input_list != NULL && "Input list pointer is NULL");
	assert(win_list != NULL && "Win list pointer is NULL");

	int input_list_size = List_count(input_list);
	assert(input_list_size % grp_size == 0 && 
					"The input size must be a multiple of group size");
	
	int win_list_size = List_count(win_list);
	assert(input_list_size > 0 && "The size of the input list is incorrect");
	assert(win_list_size == 0 && "The win list has elements");

	int res = 1;
	int array_index = 0;
	int *values = malloc(sizeof(int)*input_list_size);
			
	LIST_FOREACH(input_list, first, next, cur){
		value_box_t *cur_input = cur->value;
		values[array_index] = cur_input->value;
		if (res % grp_size == 0){
			value_box_t *new_win = malloc(sizeof(value_box_t));
			int val = 0;
			for (int i = 0; i < grp_size; i++){
				val += values[array_index - i];
			}
			new_win->value = val;
			new_win->type = cur_input->type;
			List_push(win_list, new_win);
			if (between == true && array_index != (input_list_size - 1)){
				value_box_t *inserted = malloc(sizeof(value_box_t));
				inserted->value = in_val;
				inserted->type = cur_input->type;
				List_push(win_list, inserted);
			}
		}
		res++;
		array_index++;
	}
}

/* Prints the expected output for debugging. */
void wc_print_expected_output(void)
{
    if (g_expected_output == NULL) {
        printf("Expected-output list is not initialized.\n");
        return;
    }

    printf(
        "Expected-output size: %d\n",
        List_count(g_expected_output)
    );

    LIST_FOREACH(
        g_expected_output,
        first,
        next,
        node
    ) {
        const value_box_t *expected = node->value;

        if (expected == NULL) {
            printf("Expected value: NULL\n");
            continue;
        }

        printf(
            "Expected value: %d, type: %d\n",
            expected->value,
            expected->type
        );
    }
}


/* Recreates an empty expected-output list. */
bool wc_reset_expected_output(void)
{
    wc_destroy_expected_output();
    return wc_create_expected_output();
}


/* Creates the expected-output list. */
bool wc_create_expected_output()
{
    assert(g_expected_output == NULL &&
           "Expected-output list already exists");

    g_expected_output = List_create();

    if (g_expected_output == NULL) {
        return false;
    }

    return true;
}



/* Destroys the expected-output list. */
void wc_destroy_expected_output(void)
{
    if (g_expected_output == NULL) {
        return;
    }

    List_clear_destroy(g_expected_output);
    g_expected_output = NULL;
}



int wc_get_expected_output_size(void)
{
    if (g_expected_output == NULL) {
        return 0;
    }

    return List_count(g_expected_output);
}


/* Adds one value to the expected output. */
static bool wc_add_expected_value(int value, int type)
{
    assert(g_expected_output != NULL &&
           "Expected-output list is NULL");

    value_box_t *expected = malloc(sizeof(*expected));
    if (expected == NULL) {
        return false;
    }

    expected->value = value;
    expected->type = type;

    List_push(g_expected_output, expected);
    return true;
}



/* Function: wc_get_expected_output
 *------------------------------------------------------------------------------
 * Arguments:
 *	None.
 *
 * Return:
 *	Pointer to input list.
 */
List *wc_get_expected_output()
{
	return g_expected_output;
}


/* Function: lv_check_if_win
 *------------------------------------------------------------------------------
 * Evaluates the whole output list against the win list to verify if the 
 * result is correct. Does not verifies if there are still elements in the
 * input list. 
 *
 * Arguments:
 *	None.
 *
 * Return:
 *	True if the win condition is met, false if otherwise
 */
bool lv_check_if_win()
{
	List *output = bf_get_output_list();
	List *expected = wc_get_expected_output();

	assert(output != NULL && "Output list pointer is NULL");
	assert(expected != NULL && "Expected output list pointer is NULL");

	int output_count = List_count(output);
	int expected_count = List_count(expected);

	if (output_count == 0){
		return false;
	}	
	if (output_count != expected_count) {
		return false;
	}

	ListNode *expected_node = expected->first;

	LIST_FOREACH(output, first, next, cur){
		value_box_t *actual_value = cur->value;
		value_box_t *expected_value = expected_node->value;

		if (actual_value->value != expected_value->value){
			return false;
		}
		expected_node = expected_node->next;
	}
	return true;
}
