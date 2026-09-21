#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include "list.h"
#include "win_condition_wc.h"
#include "aux.h"
#include "buffers_bf.h"


//Strings for the win condition
#define STR_WIN1 "WIN1"
#define STR_WIN2 "WIN2"
#define STR_WIN3 "WIN3"
#define STR_WIN4 "WIN4"
#define STR_WIN5 "WIN5"

#define WIN_CONDITION_LENGTH 128

typedef enum wc_insert_position_t {
    WC_INSERT_BACK = 0,
    WC_INSERT_FRONT
} wc_insert_position_t;

static List *g_expected_output = NULL;

typedef enum wc_condition_t {
    WC_CONDITION_INVALID = 0,
    WC_CONDITION_TRANSFORMED_COPY,
    WC_CONDITION_GROUP_SUMS,
    WC_CONDITION_UNTIL_STOP,
    WC_CONDITION_COUNT_UNTIL_STOP,
    WC_CONDITION_DECREASING_OFFSET
} wc_condition_t;

typedef struct wc_config_t {
    wc_condition_t condition;

    int repetitions;
    int multiplier;
    int offset;
    bool reverse;

    int group_size;
    bool insert_between;
    int separator_value;

    bool filter_enabled;
    int target_value;
    int stop_value;

    int initial_offset;
} wc_config_t;

static wc_config_t g_active_config = {
    .condition = WC_CONDITION_INVALID
};

static bool g_has_active_config = false;

static bool wc_add_expected_value(
    int value,
    int type,
    wc_insert_position_t position
);
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
static void wc_invalidate_active_condition(void);



#define WC_TOKEN_DELIMITERS " \t\r\n"

static void wc_invalidate_active_condition(void)
{
    g_active_config = (wc_config_t) {
        .condition = WC_CONDITION_INVALID
    };

    g_has_active_config = false;
}

static bool wc_next_int(char **saveptr, int *result)
{
    assert(saveptr != NULL);
    assert(result != NULL);

    char *token = strtok_r(
        NULL,
        WC_TOKEN_DELIMITERS,
        saveptr
    );

    if (token == NULL) {
        return false;
    }

    errno = 0;

    char *end = NULL;
    long value = strtol(token, &end, 10);

    if (errno != 0 ||
        end == token ||
        *end != '\0' ||
        value < INT_MIN ||
        value > INT_MAX) {
        return false;
    }

    *result = (int)value;
    return true;
}

static bool wc_next_bool(char **saveptr, bool *result)
{
    assert(saveptr != NULL);
    assert(result != NULL);

    char *token = strtok_r(
        NULL,
        WC_TOKEN_DELIMITERS,
        saveptr
    );

    if (token == NULL) {
        return false;
    }

    if (strcmp(token, "true") == 0) {
        *result = true;
        return true;
    }

    if (strcmp(token, "false") == 0) {
        *result = false;
        return true;
    }

    return false;
}

static bool wc_parse_condition(
    const char *text,
    wc_config_t *config
)
{
    assert(text != NULL);
    assert(config != NULL);

    size_t text_length = strlen(text);

    if (text_length >= WIN_CONDITION_LENGTH) {
        return false;
    }

    char buffer[WIN_CONDITION_LENGTH];
    memcpy(buffer, text, text_length + 1);

    *config = (wc_config_t) {
        .condition = WC_CONDITION_INVALID
    };

    char *saveptr = NULL;

    char *condition = strtok_r(
        buffer,
        WC_TOKEN_DELIMITERS,
        &saveptr
    );

    if (condition == NULL) {
        return false;
    }

    if (strcmp(condition, STR_WIN1) == 0) {
        config->condition =
            WC_CONDITION_TRANSFORMED_COPY;

        return
            wc_next_int(
                &saveptr,
                &config->repetitions
            ) &&
            wc_next_int(
                &saveptr,
                &config->multiplier
            ) &&
            wc_next_int(
                &saveptr,
                &config->offset
            ) &&
            wc_next_bool(
                &saveptr,
                &config->reverse
            );
    }

    if (strcmp(condition, STR_WIN2) == 0) {
        config->condition =
            WC_CONDITION_GROUP_SUMS;

        return
            wc_next_int(
                &saveptr,
                &config->group_size
            ) &&
            wc_next_bool(
                &saveptr,
                &config->insert_between
            ) &&
            wc_next_int(
                &saveptr,
                &config->separator_value
            );
    }

    if (strcmp(condition, STR_WIN3) == 0) {
        config->condition =
            WC_CONDITION_UNTIL_STOP;

        int filter = 0;

        bool valid =
            wc_next_int(&saveptr, &filter) &&
            wc_next_int(
                &saveptr,
                &config->target_value
            ) &&
            wc_next_int(
                &saveptr,
                &config->stop_value
            );

        if (!valid || (filter != 0 && filter != 1)) {
            return false;
        }

        config->filter_enabled = filter == 1;
        return true;
    }

    if (strcmp(condition, STR_WIN4) == 0) {
        config->condition =
            WC_CONDITION_COUNT_UNTIL_STOP;

        return
            wc_next_int(
                &saveptr,
                &config->target_value
            ) &&
            wc_next_int(
                &saveptr,
                &config->stop_value
            );
    }

    if (strcmp(condition, STR_WIN5) == 0) {
        config->condition =
            WC_CONDITION_DECREASING_OFFSET;

        return wc_next_int(
            &saveptr,
            &config->initial_offset
        );
    }

    return false;
}


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

/*
 * Builds the expected output from a validated win-condition
 * configuration.
 *
 * Returns true when the expected output is built successfully.
 * Returns false for an invalid condition or builder failure.
 */
static bool wc_build_from_config(const wc_config_t *config)
{
    assert(config != NULL &&
           "Win-condition configuration is NULL");

    switch (config->condition) {
        case WC_CONDITION_TRANSFORMED_COPY:
            return wc_build_transformed_copy(
                config->repetitions,
                config->multiplier,
                config->offset,
                config->reverse
            );

        case WC_CONDITION_GROUP_SUMS:
            return wc_build_group_sums(
                config->group_size,
                config->insert_between,
                config->separator_value
            );

        case WC_CONDITION_UNTIL_STOP:
            return wc_build_until_stop(
                config->filter_enabled,
                config->target_value,
                config->stop_value
            );

        case WC_CONDITION_COUNT_UNTIL_STOP:
            return wc_build_count_until_stop(
                config->target_value,
                config->stop_value
            );

        case WC_CONDITION_DECREASING_OFFSET:
            return wc_build_decreasing_offset(
                config->initial_offset
            );

        case WC_CONDITION_INVALID:
        default:
            return false;
    }
}
/*
 * Parses and activates a win condition from its text definition.
 *
 * The expected-output list is recreated before building the new
 * condition. The active configuration is stored only after the
 * expected output is built successfully.
 *
 * Returns true on success and false when parsing, allocation,
 * or expected-output construction fails.
 */

bool wc_set_condition_from_text(const char *text)
{
    if (text == NULL) {
        return false;
    }

    wc_config_t parsed_config = {
        .condition = WC_CONDITION_INVALID
    };

    if (!wc_parse_condition(text, &parsed_config)) {
        return false;
    }

    if (!wc_reset_expected_output()) {
        wc_invalidate_active_condition();
        return false;
    }

    if (!wc_build_from_config(&parsed_config)) {
        wc_destroy_expected_output();
        wc_invalidate_active_condition();
        return false;
    }

    g_active_config = parsed_config;
    g_has_active_config = true;

    return true;
}



/*
 * Rebuilds the expected output using the active win-condition
 * configuration.
 *
 * Returns true on success. Returns false when no condition is
 * active, the expected-output list cannot be recreated, or the
 * builder fails.
 */
bool wc_reset_condition(void)
{
    if (!g_has_active_config) {
        return false;
    }

    if (!wc_reset_expected_output()) {
        wc_invalidate_active_condition();
        return false;
    }

    if (!wc_build_from_config(&g_active_config)) {
        wc_destroy_expected_output();
        wc_invalidate_active_condition();
        return false;
    }

    return true;
}
/*
 * Checks whether the current output matches the beginning of the
 * expected output.
 *
 * An empty output is considered valid. The function returns false
 * if the output is longer than the expected output or if any value
 * differs from the corresponding expected value.
 */
bool wc_matches_expected_prefix(void)
{
    List *output = bf_get_output_list();

    assert(output != NULL &&
           "Output list is NULL");

    assert(g_expected_output != NULL &&
           "Expected-output list is NULL");

    int output_count = List_count(output);
    int expected_count = List_count(g_expected_output);

    if (expected_count == 0) {
        return false;
    }

    if (output_count > expected_count) {
        return false;
    }

    /*
     * Producing no output has not violated the expected result yet.
     */
    if (output_count == 0) {
        return true;
    }

    ListNode *expected_node = g_expected_output->first;

    LIST_FOREACH(output, first, next, output_node) {
        assert(expected_node != NULL &&
               "Expected-output node is NULL");

        const value_box_t *actual =
            output_node->value;

        const value_box_t *expected =
            expected_node->value;

        assert(actual != NULL &&
               "Actual output value is NULL");

        assert(expected != NULL &&
               "Expected output value is NULL");

        if (actual->value != expected->value) {
            return false;
        }

        expected_node = expected_node->next;
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



/*
 * Returns true when the actual output exactly matches the
 * complete expected output.
 */
bool wc_is_satisfied(void)
{
    List *output = bf_get_output_list();

    assert(output != NULL &&
           "Output list is NULL");

    assert(g_expected_output != NULL &&
           "Expected-output list is NULL");

    int output_count = List_count(output);
    int expected_count = List_count(g_expected_output);

    if (output_count == 0 ||
        output_count != expected_count) {
        return false;
    }

    ListNode *expected_node =
        g_expected_output->first;

    LIST_FOREACH(output, first, next, output_node) {
        assert(expected_node != NULL &&
               "Expected-output node is NULL");

        const value_box_t *actual =
            output_node->value;

        const value_box_t *expected =
            expected_node->value;

        assert(actual != NULL &&
               "Actual output value is NULL");

        assert(expected != NULL &&
               "Expected output value is NULL");

        if (actual->value != expected->value) {
            return false;
        }

        expected_node = expected_node->next;
    }

    return true;
}
