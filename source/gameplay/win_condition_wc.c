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

typedef enum wc_insert_position_t {
    WC_INSERT_BACK = 0,
    WC_INSERT_FRONT
} wc_insert_position_t;

static List *g_expected_output = NULL;


static bool wc_add_expected_value(
    int value,
    int type,
    wc_insert_position_t position
);

//static bool wc_add_expected_value(int value, int type);
static bool wc_build_transformed_copy(
    int repetitions,
    int multiplier,
    int offset,
    bool reverse
);

static bool wc_build_group_sums(
    int group_size,
    bool insert_between,
    int separator_value
);
static bool wc_build_until_stop(
    bool filter_enabled,
    int target_value,
    int stop_value
);
static bool wc_build_count_until_stop(
    int target_value,
    int stop_value
);

static bool wc_build_decreasing_offset(int initial_offset);
static void set_win_condition(char *win_condition);
static char level_win_condition[WIN_CONDITION_LENGTH];



/* Adds one value to the expected output. */
static bool wc_add_expected_value(
    int value,
    int type,
    wc_insert_position_t position
)
{
    assert(g_expected_output != NULL &&
           "Expected-output list is NULL");

    value_box_t *expected = malloc(sizeof(*expected));

    if (expected == NULL) {
        return false;
    }

    expected->value = value;
    expected->type = type;

    if (position == WC_INSERT_FRONT) {
        List_unshift(g_expected_output, expected);
    } else {
        List_push(g_expected_output, expected);
    }

    return true;
}



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
 * WIN1: wc_build_transfromed_copy(rep, mul, rev) 
 * WIN2: wx_build_group_sums(group_size)
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
		wc_build_transformed_copy(rep, mul, sum, rev);
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
		wc_build_group_sums(group_size, between, in_val);
	} else if (strstr(win_cond, STR_WIN3) != NULL){
		char *target_text = strtok_r(NULL, delim, &saveptr1);
		int target = atoi(target_text);
		char *tval_text = strtok_r(NULL, delim, &saveptr1);
		int tval = atoi(tval_text);
		char *stop_text = strtok_r(NULL, delim, &saveptr1);
		int stop = atoi(stop_text);
		wc_build_until_stop(target, tval, stop);
	} else if (strstr(win_cond, STR_WIN4) != NULL){
		char *element_text = strtok_r(NULL, delim, &saveptr1);
		int element = atoi(element_text);
		char *stop_text = strtok_r(NULL, delim, &saveptr1);
		int stop = atoi(stop_text);
		wc_build_count_until_stop(element, stop);
	} else if (strstr(win_cond, STR_WIN5) != NULL){
		char *dec_text = strtok_r(NULL, delim, &saveptr1);
		int dec = atoi(dec_text);
		wc_build_decreasing_offset(dec);
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

/*
 * Builds an expected output by transforming and copying every
 * value from the input buffer.
 *
 * Each input value is transformed as:
 *
 *     multiplier * input_value + offset
 *
 * The transformed value is inserted `repetitions` times.
 * When `reverse` is true, the resulting sequence is reversed.
 *
 * Returns true on success and false on allocation failure.
 */
static bool wc_build_transformed_copy(
    int repetitions,
    int multiplier,
    int offset,
    bool reverse
)
{
    List *input = get_input_list();

    assert(repetitions > 0 &&
           "Repetition count must be greater than zero");

    assert(multiplier > 0 &&
           "Multiplier must be greater than zero");

    assert(input != NULL &&
           "Input list is NULL");

    assert(g_expected_output != NULL &&
           "Expected-output list is NULL");

    assert(List_count(input) > 0 &&
           "Input list is empty");

    assert(List_count(g_expected_output) == 0 &&
           "Expected-output list is not empty");

    wc_insert_position_t position = reverse
        ? WC_INSERT_FRONT
        : WC_INSERT_BACK;

    LIST_FOREACH(input, first, next, node) {
        const value_box_t *input_value = node->value;

        assert(input_value != NULL &&
               "Input value is NULL");

        int transformed_value =
            multiplier * input_value->value + offset;

        for (int i = 0; i < repetitions; ++i) {
            bool added = wc_add_expected_value(
                transformed_value,
                input_value->type,
                position
            );

            if (!added) {
                return false;
            }
        }
    }

    return true;
}
/*
 * Builds the expected output from input values encountered before
 * the stop value.
 *
 * When filter_enabled is true, only values matching target_value
 * are copied. The stop value itself is not copied.
 *
 * Returns true on success and false on allocation failure.
 */
static bool wc_build_until_stop(
    bool filter_enabled,
    int target_value,
    int stop_value
)
{
    List *input = get_input_list();

    assert(input != NULL &&
           "Input list is NULL");

    assert(g_expected_output != NULL &&
           "Expected-output list is NULL");

    assert(List_count(input) > 0 &&
           "Input list is empty");

    assert(List_count(g_expected_output) == 0 &&
           "Expected-output list is not empty");

    LIST_FOREACH(input, first, next, node) {
        const value_box_t *input_value = node->value;

        assert(input_value != NULL &&
               "Input value is NULL");

        if (input_value->value == stop_value) {
            break;
        }

        if (filter_enabled &&
            input_value->value != target_value) {
            continue;
        }

        if (!wc_add_expected_value(
                input_value->value,
                input_value->type,
                WC_INSERT_BACK)) {
            return false;
        }
    }

    return true;
}

/*
 * Counts occurrences of target_value before stop_value is reached,
 * then adds the count to the expected output.
 *
 * The expected value inherits the type of the stop element.
 *
 * Returns false if allocation fails or stop_value is not found.
 */
static bool wc_build_count_until_stop(
    int target_value,
    int stop_value
)
{
    List *input = get_input_list();

    assert(input != NULL &&
           "Input list is NULL");

    assert(g_expected_output != NULL &&
           "Expected-output list is NULL");

    assert(List_count(input) > 0 &&
           "Input list is empty");

    assert(List_count(g_expected_output) == 0 &&
           "Expected-output list is not empty");

    int count = 0;

    LIST_FOREACH(input, first, next, node) {
        const value_box_t *input_value = node->value;

        assert(input_value != NULL &&
               "Input value is NULL");

        /*
         * Stop takes precedence if target_value and stop_value
         * happen to contain the same value.
         */
        if (input_value->value == stop_value) {
            return wc_add_expected_value(
                count,
                input_value->type,
                WC_INSERT_BACK
            );
        }

        if (input_value->value == target_value) {
            ++count;
        }
    }

    /*
     * The input ended without the required stop value.
     */
    return false;
}

/*
 * Builds the expected output by adding a decreasing offset
 * to each input value.
 *
 * The first value receives initial_offset, and the offset
 * decreases by one for every following value.
 *
 * Returns true on success and false on allocation failure.
 */
static bool wc_build_decreasing_offset(int initial_offset)
{
    List *input = get_input_list();

    assert(input != NULL &&
           "Input list is NULL");

    assert(g_expected_output != NULL &&
           "Expected-output list is NULL");

    assert(List_count(input) > 0 &&
           "Input list is empty");

    assert(List_count(g_expected_output) == 0 &&
           "Expected-output list is not empty");

    int offset = initial_offset;

    LIST_FOREACH(input, first, next, node) {
        const value_box_t *input_value = node->value;

        assert(input_value != NULL &&
               "Input value is NULL");

        int expected_value =
            input_value->value + offset;

        if (!wc_add_expected_value(
                expected_value,
                input_value->type,
                WC_INSERT_BACK)) {
            return false;
        }

        --offset;
    }

    return true;
}

/*
 * Builds the expected output by adding input values in fixed-size groups.
 *
 * When insert_between is true, separator_value is inserted between
 * consecutive group sums.
 *
 * Returns true on success and false on allocation failure.
 */
static bool wc_build_group_sums(
    int group_size,
    bool insert_between,
    int separator_value
)
{
    List *input = get_input_list();

    assert(group_size > 0 &&
           "Group size must be greater than zero");

    assert(input != NULL &&
           "Input list is NULL");

    assert(g_expected_output != NULL &&
           "Expected-output list is NULL");

    int input_count = List_count(input);

    assert(input_count > 0 &&
           "Input list is empty");

    assert(input_count % group_size == 0 &&
           "Input size must be a multiple of group size");

    assert(List_count(g_expected_output) == 0 &&
           "Expected-output list is not empty");

    int group_sum = 0;
    int value_index = 0;

    LIST_FOREACH(input, first, next, node) {
        const value_box_t *input_value = node->value;

        assert(input_value != NULL &&
               "Input value is NULL");

        group_sum += input_value->value;
        ++value_index;

        bool group_complete =
            value_index % group_size == 0;

        if (!group_complete) {
            continue;
        }

        if (!wc_add_expected_value(
                group_sum,
                input_value->type,
                WC_INSERT_BACK)) {
            return false;
        }

        bool more_groups_remain =
            value_index < input_count;

        if (insert_between && more_groups_remain) {
            if (!wc_add_expected_value(
                    separator_value,
                    input_value->type,
                    WC_INSERT_BACK)) {
                return false;
            }
        }

        group_sum = 0;
    }

    return true;
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


/* Adds one value to the expected output. 
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
}*/



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
