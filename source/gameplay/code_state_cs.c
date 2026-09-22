#include <stdio.h>
#include <stdlib.h>
#include "code_state_cs.h"
#include "code_window_cw.h"
#include "game_mechanics_mc.h"

static code_line_t *g_hold_line;

bool chk_player_holds_line();





/* Function: lv_set_hold_line
 * ----------------------------------------------------------------------------
 * This function has the pointer of the line being hold by the player.
 * Is NULL if the player is not currently holding any line
 *
 * Arguments:
 *	line: Pointer of the line being hold by the player
 *
 * Return:
 *	Void.
 */
void lv_set_hold_line(code_line_t *line)
{
	g_hold_line = line;
}


/* Function: get_hold_line
 * ----------------------------------------------------------------------------
 * Returns the poiinter of the line being hold by the player. 
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	Pointer of the line being hold by the player
 */
code_line_t *get_hold_line()
{
	return g_hold_line;
}


/* Function: chk_player_holds_line
 * ----------------------------------------------------------------------------
 * Verifies if the player is holding a line by analizing the pointer
 *
 * Arguments:
 *	Void
 *
 * Return:
 *	Void to determine if the player is holding a line;
 */
bool chk_player_holds_line()
{
	bool hold_line = (g_hold_line == NULL) ? false : true;
	return hold_line;
}

/* Function: cs_capture_context
 * -----------------------------------------------------------------------------
 * Captures the current context of the code state machine
 * 
 * Arguments:
 * 	None.
 *
 * Return:
 *	cs_context_t: Current context of the code state machine
 */
cs_context_t cs_capture_context(void)
{
    cs_context_t context = {0};

    context.code_size =
        cw_get_code_list_size();

    context.operation_flag =
        mc_get_operation_flag();

    context.holding_instruction =
        chk_player_holds_line();

    context.held_instruction_id = -1;

    code_line_t *held_line =
        get_hold_line();

    if (held_line != NULL &&
        held_line->ins != NULL) {
        context.held_instruction_id =
            held_line->ins->id;
    }

    context.code_sorted =
        cw_check_code_sorted();

    context.operand_pending =
        cw_is_operand_pending();

    context.operand_1_pending =
        cw_is_operand_1_pending();

    context.operand_2_pending =
        cw_is_operand_2_pending();

    context.playing =
        mc_is_executing();

    return context;
}

/* Function: cs_get_state
 * -----------------------------------------------------------------------------
 * Returns the current state of the code state machine
 * 
 * Arguments:
 * 	None.
 *
 * Return:
 *	cs_state_t: Current state of the code state machine
 */

cs_state_t cs_get_state(void) {
	cs_context_t context = cs_capture_context();
	cs_state_t state = CS_STATE_UNCLASSIFIED;
	//return cs_classify(&context);
	return state;
}

