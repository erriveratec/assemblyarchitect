#include "code_window_cw.h"
#include "code_window_cw_internal.h"
#include "code_layout_cly.h"
#include "code_program_cp.h"
#include "code_edit_ce.h"
#include "control_flow_cf.h"
#include "code_renderer_cr.h"
#include "code_program_cp.h"
#include "aux.h"

#include <SDL.h>
#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include "instruction_layout_il.h"
#include "mouse_ms.h"
#include "dbg.h"
#include "dimensions_dm.h"
#include "buffers_bf.h"
#include "registers_rg.h"
#include "draw_dw.h"
#include "text_tx.h"
#include "gameplay/highlight_animation_ha.h"
#include "immediates_im.h"
#include "stage_buttons_sb.h"
#include "ui/escape_menu_em.h"
#include "ui/ui_metrics_um.h"
#include "ui/typography_ty.h"


#define NOT_FOUND -1
#define CODE_LINES_SIZE 5
#define CODE_BOX_HIGHLIGHT_GROWTH_MULTIPLIER 12.0f
static const int MISSING_OPERAND_WIDTH = 3;
static const int CODE_CONTENT_X_ADJUSTMENT = -8;

typedef struct button_animation_claim_t {
	btn_t *button;
	bool previous_animated;
	bool previous_anim_dir;
	int previous_anim_state;
	bool active;
} button_animation_claim_t;

typedef enum jump_highlight_component_t {
	JUMP_HIGHLIGHT_OPERAND,
	JUMP_HIGHLIGHT_LABEL_INSTRUCTION,
	JUMP_HIGHLIGHT_LABEL_OPERAND,
	JUMP_HIGHLIGHT_COMPONENT_COUNT
} jump_highlight_component_t;

typedef struct jump_target_label_buttons_t {
	btn_t *instruction_button;
	btn_t *operand_button;
} jump_target_label_buttons_t;

typedef struct code_window_state_t {
	code_program_t *program;
	ce_session_t edit_session;
	cr_assets_t renderer_assets;
	tx_styled_text_t *challenge_text;
	texture_t *stage_name;
	uint64_t next_label_identity;
	Uint32 code_box_width;
	Uint32 code_box_y;
	int pending_scroll_y;
	SDL_Rect code_box;
	SDL_Rect scroll_box;
	SDL_Rect text_box;
	bool challenge_highlight;
	bool code_box_highlight;
	float challenge_highlight_value;
	float code_box_highlight_value;
	button_animation_claim_t jump_claims[JUMP_HIGHLIGHT_COMPONENT_COUNT];
} code_window_state_t;

static code_window_state_t g_code_window = {
	.code_box_width = 355,
	.code_box_y = 50
};

#define CODE_BOX_W (g_code_window.code_box_width)
#define CODE_BOX_Y (g_code_window.code_box_y)
#define g_code_box (g_code_window.code_box)
#define g_scroll_box (g_code_window.scroll_box)
#define g_text_box (g_code_window.text_box)
#define g_challenge_highlight (g_code_window.challenge_highlight)
#define g_code_box_highlight (g_code_window.code_box_highlight)
#define g_challenge_highlight_value (g_code_window.challenge_highlight_value)
#define g_code_box_highlight_value (g_code_window.code_box_highlight_value)
#define g_active_jump_claims (g_code_window.jump_claims)
#define g_challenge_text (g_code_window.challenge_text)
#define g_stage_name (g_code_window.stage_name)

/* Snapshot claims preserve prior state; they cannot arbitrate concurrent writers to btn_t. */

static tx_styled_text_t *cw_create_challenge_text(
	int width, int text_height, const char *text,
	const tx_styled_text_options_t *options)
{
	return tx_create_styled_text_for_width(width, text_height, text, options);
}

#ifdef CW_CHALLENGE_TEXT_TESTING
static cw_challenge_text_factory_t g_challenge_text_factory =
	cw_create_challenge_text;

void cw_set_challenge_text_factory(cw_challenge_text_factory_t factory)
{
	g_challenge_text_factory = factory != NULL ? factory :
		cw_create_challenge_text;
}

void cw_reset_challenge_text_factory(void)
{
	g_challenge_text_factory = cw_create_challenge_text;
}

tx_styled_text_t *cw_get_challenge_text_for_testing(void)
{
	return g_challenge_text;
}

static tx_styled_text_t *cw_invoke_challenge_text_factory(
	int width, int text_height, const char *text,
	const tx_styled_text_options_t *options)
{
	return g_challenge_text_factory(width, text_height, text, options);
}
#else
static tx_styled_text_t *cw_invoke_challenge_text_factory(
	int width, int text_height, const char *text,
	const tx_styled_text_options_t *options)
{
	return cw_create_challenge_text(width, text_height, text, options);
}
#endif

int get_code_line_position(int y);
static bool in_code_window(void);
static bool chk_sel_line_in_pos(code_line_t *line);
static void set_text_box(int x, int y, int w, int h);
static void add_code_line(code_line_t *line);
static void add_code_line_last_pos(code_line_t *line);
static void set_code_box_member(int value, int member);
static void set_text_box_member(int value, int member);
static void code_box_height_adjust(void);
static void adjust_code_box_position(void);
static int get_label_operand_value(code_line_t *line);
static operand_t *create_label_operand(code_line_t *line);
static operand_t *create_saved_label_operand(int op1_id);
static int label_counter_up_to_index(int index);
static void update_label_instructions(void);
static void update_jump_instructions(void);
static operand_t *create_updated_jump_operand(code_line_t *jmp_addr);
static operand_t *create_saved_jump_operand(int op1_id);
static SDL_Rect get_scroll_box(void);
static void reset_button_animation(btn_t *button);
static int code_line_spacing(void);
static int get_code_line_x(int instruction_id);
static int get_code_line_y(int position);
static code_line_t *create_label_code_line(void);
static instruction_t *create_clone_instruction(
	const instruction_t *source, void *context);
static operand_t *create_clone_operand(
	const operand_t *source, bool is_jump, void *context);

static void release_button_animation_claim(button_animation_claim_t *claim)
{
	if (claim == NULL || !claim->active) {
		return;
	}
	if (claim->button != NULL) {
		claim->button->animated = claim->previous_animated;
		claim->button->anim_dir = claim->previous_anim_dir;
		claim->button->anim_state = claim->previous_anim_state;
	}
	*claim = (button_animation_claim_t){0};
}

static void claim_button_animation(button_animation_claim_t *claim,
	                               btn_t *button)
{
	if (claim == NULL) {
		return;
	}
	if (claim->active && claim->button == button) {
		return;
	}
	release_button_animation_claim(claim);
	if (button == NULL) {
		return;
	}
	claim->button = button;
	claim->previous_animated = button->animated;
	claim->previous_anim_dir = button->anim_dir;
	claim->previous_anim_state = button->anim_state;
	claim->active = true;
	button->animated = true;
	button->anim_dir = false;
	button->anim_state = 0;
}

static jump_target_label_buttons_t get_jump_target_label_buttons(
	const code_line_t *line)
{
	jump_target_label_buttons_t buttons = {0};
	if (line == NULL || line->ins == NULL ||
	    !cl_is_ins_jmp_type(line->ins->id) || line->op1 == NULL ||
	    line->op1->jptr == NULL || line->op1->jptr->ins == NULL ||
	    line->op1->jptr->ins->id != LABEL) {
		return buttons;
	}
	code_line_t *target = line->op1->jptr;
	buttons.instruction_button = target->ins->b;
	if (target->op1 != NULL) {
		buttons.operand_button = target->op1->b;
	}
	return buttons;
}

static void release_active_jump_replacement_claims(void)
{
	btn_t *released_buttons[JUMP_HIGHLIGHT_COMPONENT_COUNT] = {0};
	int released_count = 0;
	for (int index = JUMP_HIGHLIGHT_COMPONENT_COUNT - 1; index >= 0; index--) {
		button_animation_claim_t *claim = &g_active_jump_claims[index];
		if (claim->active && claim->button != NULL) {
			bool already_released = false;
			for (int earlier = 0; earlier < released_count; earlier++) {
				if (released_buttons[earlier] == claim->button) {
					already_released = true;
					break;
				}
			}
			if (already_released) {
				g_active_jump_claims[index] = (button_animation_claim_t){0};
				continue;
			}
			released_buttons[released_count++] = claim->button;
		}
		release_button_animation_claim(claim);
	}
}

static void start_active_jump_replacement_highlight(code_line_t *line)
{
	if (!cw_is_active_jump_operand_replacement(
		line, cw_get_code_line_pending_operand()) ||
	    line->op1 == NULL || line->op1->b == NULL) {
		return;
	}

	jump_target_label_buttons_t target_buttons =
		get_jump_target_label_buttons(line);
	btn_t *desired_buttons[JUMP_HIGHLIGHT_COMPONENT_COUNT] = {
		line->op1->b,
		target_buttons.instruction_button,
		target_buttons.operand_button
	};
	for (int index = 0; index < JUMP_HIGHLIGHT_COMPONENT_COUNT; index++) {
		if (desired_buttons[index] == NULL) {
			continue;
		}
		for (int earlier = 0; earlier < index; earlier++) {
			if (desired_buttons[earlier] == desired_buttons[index]) {
				desired_buttons[index] = NULL;
				break;
			}
		}
	}
	bool same_relationship = true;
	for (int index = 0; index < JUMP_HIGHLIGHT_COMPONENT_COUNT; index++) {
		button_animation_claim_t *claim = &g_active_jump_claims[index];
		if ((desired_buttons[index] != NULL) != claim->active ||
		    (claim->active && claim->button != desired_buttons[index])) {
			same_relationship = false;
			break;
		}
	}
	if (same_relationship) {
		return;
	}
	release_active_jump_replacement_claims();
	for (int index = 0; index < JUMP_HIGHLIGHT_COMPONENT_COUNT; index++) {
		claim_button_animation(&g_active_jump_claims[index], desired_buttons[index]);
	}
}

static cr_visual_state_t get_code_renderer_visual_state(void)
{
	cr_visual_state_t visual_state = {0};
	for (int index = 0; index < JUMP_HIGHLIGHT_COMPONENT_COUNT; index++) {
		if (g_active_jump_claims[index].active) {
			visual_state.jump_claim_buttons[index] =
				g_active_jump_claims[index].button;
		}
	}
	return visual_state;
}

static bool ensure_label_identity(code_line_t *line)
{
	return cf_ensure_label_identity(
		g_code_window.program, line, &g_code_window.next_label_identity) ==
		CF_VALID_TARGET;
}

static int code_line_spacing(void)
{
	return cly_code_line_spacing(
		cl_get_code_button_size().h, um_padding_vertical());
}

static int cw_get_code_text_width(void)
{
	return cw_get_stage_code_box().w - 2 * um_padding_horizontal_with_border();
}

static int cw_get_operand1_offset(void)
{
	return cly_operand_offset(cl_get_code_button_size().w, 0, 1);
}

static int cw_get_operand2_offset(void)
{
	return cly_operand_offset(cl_get_code_button_size().w,
		um_padding_horizontal(), 2);
}

static int cw_get_comma_offset(void)
{
	return cw_get_operand2_offset() - 15;
}

static int cw_get_missing_operand_width(void)
{
	return MISSING_OPERAND_WIDTH;
}

/* Function: cw_get_code_stage_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object, x and y are initialize at 0
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect cw_get_stage_code_box(void)
{
	SDL_Rect ib = il_get_initial_instruction_bounds();
	SDL_Rect sb = sb_get_stage_btns();
	return cly_calculate_stage_code_box(ib, sb, CODE_BOX_W, CODE_BOX_Y);
}

/* Function: cw_init_code_window_texture
 * -----------------------------------------------------------------------------
 * Inits the textures for the code windows
 * 
 * Arguments:
 *  Void.
 *
 * Return:
 * 	Void.
 */
void cw_init_code_window_texture(void)
{
	bool initialized = cr_assets_init(
		&g_code_window.renderer_assets, MAX_CODE_LINES + 1);
	assert(initialized && "Unable to initialize code renderer assets");
	if (!initialized) {
		return;
	}
}

/* Function: cw_update_saved_jump_instructions
 * -----------------------------------------------------------------------------
 * Traverses the whole code list and updates all the jump instructions
 * when all the saved instructions are completely loaded in the code.
 * 
 * Arguments:
 *  void.
 *
 * Return:
 * 	void.
 */
void cw_update_saved_jump_instructions(void)
{
	release_active_jump_replacement_claims();

	int op1_ofs = cw_get_operand1_offset();
	if (cf_translate_v1_targets(
		g_code_window.program, &g_code_window.next_label_identity) !=
	    CF_VALID_TARGET) {
		goto error;
	}
	for (int index = 0; index < cp_count(g_code_window.program); index++) {
		code_line_t *c = cp_get(g_code_window.program, index);
		if (cl_is_ins_jmp_type(c->ins->id) == true){
			assert(c->op1 != NULL && "op1 should't be NULL");
			code_line_t *jmp_addr = c->op1->jptr;
			cl_destroy_operand(c->op1);	
			assert(cp_contains(g_code_window.program, jmp_addr) && "Instruction"
				   "not present in code list");
			c->op1 = create_updated_jump_operand(jmp_addr);
			c->op1->target_label_identity = jmp_addr->label_identity;
			c->op1->b->r.x = c->ins->b->r.x + op1_ofs;
			c->op1->b->r.y = c->ins->b->r.y;
		}
	}
error:
	return;
}

/* Function: create_saved_jump_operand
*------------------------------------------------------------------------------
* Creates a placeholder jump operand in before the whole code loads
*
* Arguments:
* 	op1_id: the id of the operand what will be created
*	
* Return:
*	Pointer to the newly created operand
*
*/
static operand_t *create_saved_jump_operand(int op1_id)
{
   	operand_t *b = NULL;
	b = malloc(sizeof(operand_t));

	SDL_Rect cb = cl_get_code_button_size();

	char *line_text = ax_number_to_string_two_digits(op1_id);
	char *op_text = malloc(strlen(line_text) + strlen(ax_char_colon) + 1);
	strcpy(op_text, line_text);
	texture_t *t = dw_create_text_tex(op_text, C_WHITE);

	SDL_Rect r = {.x = 0, .y = 0, .w = 2*cb.w, .h = cb.h};
	b->b = bt_create_btn(r, t);

	b->id = op1_id;
	b->jptr = NULL;
	b->target_label_identity = 0;
	free(line_text);
	free(op_text);
	return b;
}

/* Function: create_updated_jump_operand
 * Creates the label operand for a resolved target line.
 */
static operand_t *create_updated_jump_operand(code_line_t *jmp_addr)
{
	assert(cp_contains(g_code_window.program, jmp_addr));
	operand_t *operand = malloc(sizeof(*operand));
	assert(operand != NULL);
	char *line_text = ax_number_to_string_two_digits(jmp_addr->op1->id);
	char *operand_text = malloc(strlen(label_text) + strlen(ax_char_space) +
	                             strlen(line_text) + 1);
	assert(operand_text != NULL);
	strcpy(operand_text, label_text);
	strcat(operand_text, ax_char_space);
	strcat(operand_text, line_text);
	texture_t *texture = dw_create_text_tex(operand_text, C_WHITE);
	SDL_Rect button_size = cl_get_code_button_size();
	SDL_Rect rect = {.x = 0, .y = 0, .w = 2 * button_size.w,
	                 .h = button_size.h};
	operand->b = bt_create_btn(rect, texture);
	operand->id = cp_index_of(g_code_window.program, jmp_addr);
	operand->jptr = jmp_addr;
	operand->target_label_identity = jmp_addr->label_identity;
	free(line_text);
	free(operand_text);
	return operand;
}

/* Function: update_jump_instructions
 * Traverses the program and rebuilds presentation operands for jump targets.
 */
static void update_jump_instructions(void)
{
	release_active_jump_replacement_claims();

	int op1_ofs = cw_get_operand1_offset();
	cf_repair_program_targets(
		g_code_window.program, &g_code_window.next_label_identity);
	for (int index = 0; index < cp_count(g_code_window.program); index++) {
		code_line_t *c = cp_get(g_code_window.program, index);
		if (cl_is_ins_jmp_type(c->ins->id) == true){
			if (c->op1 != NULL){
				code_line_t *jmp_addr = c->op1->jptr;
				if (jmp_addr != NULL &&
				    cp_contains(g_code_window.program, jmp_addr)){
					ensure_label_identity(jmp_addr);
					char *label_number = ax_number_to_string_two_digits(
						jmp_addr->op1->id);
					if (label_number != NULL && c->op1->b != NULL) {
						size_t text_size = strlen(label_text) +
							strlen(ax_char_space) + strlen(label_number) + 1;
						char *operand_text = malloc(text_size);
						if (operand_text != NULL) {
							strcpy(operand_text, label_text);
							strcat(operand_text, ax_char_space);
							strcat(operand_text, label_number);
							texture_t *texture = dw_create_text_tex(
								operand_text, C_WHITE);
							if (texture != NULL) {
								texture_t *previous = c->op1->b->t;
								c->op1->b->t = texture;
								dw_free_texture(previous);
							}
							free(operand_text);
						}
						free(label_number);
					}
					c->op1->id = cp_index_of(
						g_code_window.program, jmp_addr);
					c->op1->jptr = jmp_addr;
					c->op1->target_label_identity =
						jmp_addr->label_identity;
					c->op1->b->r.x = c->ins->b->r.x + op1_ofs;
					c->op1->b->r.y = c->ins->b->r.y;
				} else {
					cl_destroy_operand(c->op1);
					c->op1 = NULL;
					c->state = MISSING_OP1;
				}
			}
		}
	}
	return;
}

/* Creates a display operand for a validated in-program label target. */
static operand_t *create_jump_operand(code_line_t *addr)
{
	if (!cp_contains(g_code_window.program, addr) || addr->ins == NULL ||
	    addr->ins->id != LABEL || addr->op1 == NULL ||
	    !ensure_label_identity(addr)) {
		return NULL;
	}
	char *line_text = ax_number_to_string_two_digits(addr->op1->id);
	if (line_text == NULL) {
		return NULL;
	}
	size_t text_size = strlen(label_text) + strlen(ax_char_space) +
		strlen(line_text) + 1;
	char *op_text = malloc(text_size);
	if (op_text == NULL) {
		free(line_text);
		return NULL;
	}
	strcpy(op_text, label_text);
	strcat(op_text, ax_char_space);
	strcat(op_text, line_text);
	texture_t *t = dw_create_text_tex(op_text, C_WHITE);
	free(line_text);
	free(op_text);
	if (t == NULL) {
		return NULL;
	}

	SDL_Rect cb = cl_get_code_button_size();
	SDL_Rect r = {.x = 0, .y = 0, .w = 2*cb.w, .h = cb.h};
	btn_t *button = bt_create_btn(r, t);
	if (button == NULL) {
		dw_free_texture(t);
		return NULL;
	}
	operand_t *op = malloc(sizeof(*op));
	if (op == NULL) {
		bt_destroy_button(button);
		return NULL;
	}
	op->b = button;
	op->id = cp_index_of(g_code_window.program, addr);
	op->jptr = addr;
	op->target_label_identity = addr->label_identity;

	return op;
} 





/* Function: update_label_instructions
 * -----------------------------------------------------------------------------
 * Traverses the whole code list and updates all the label instructions
 * 
 * Arguments:
 *  void.
 *
 * Return:
 * 	void.
 */
static void update_label_instructions(void)
{
	release_active_jump_replacement_claims();
	
	int op1_ofs = cw_get_operand1_offset();
	for (int index = 0; index < cp_count(g_code_window.program); index++) {
		code_line_t *c = cp_get(g_code_window.program, index);
		if (c->ins->id == LABEL){
			if (c->op1 != NULL){
				cl_destroy_operand(c->op1);	
				c->op1 = create_label_operand(c);
				c->op1->b->r.x = c->ins->b->r.x + op1_ofs;
				c->op1->b->r.y = c->ins->b->r.y;
			}

		}
	}
	return;
}
/* Function: create_saved_label_operand
*------------------------------------------------------------------------------
* Creates the label operand with the corresponding number that will be shown
* in the code
*
* Arguments:
* 	None.
*	
* Return:
*	Pointer to the newly created operand
*
*/
static operand_t *create_saved_label_operand(int op1_id)
{
   	operand_t *b = NULL;

	b = malloc(sizeof(operand_t));

	char *line_text = ax_number_to_string_two_digits(op1_id);
	char *op_text = malloc(strlen(line_text) + strlen(":") + 1);
	strcpy(op_text, line_text);
	strcat(op_text, ax_char_colon);
	texture_t *t = dw_create_text_tex(op_text, C_WHITE);

	SDL_Rect r = cl_get_code_button_size();
	b->b = bt_create_btn(r, t);

	b->id = op1_id;
	b->jptr = NULL;
	b->target_label_identity = 0;

	free(line_text);
	free(op_text);

	return b;
}

/* Function: create_label_operand
*------------------------------------------------------------------------------
* Creates the label operand with the corresponding number that will be shown
* in the code
*
* Arguments:
* 	None.
*	
* Return:
*	Pointer to the newly created operand
*
*/
static operand_t *create_label_operand(code_line_t *line)
{
   	operand_t *b = NULL;

	b = malloc(sizeof(operand_t));

	int label = get_label_operand_value(line);

	char *line_text = ax_number_to_string_two_digits(label);
	char *op_text = malloc(strlen(line_text) + strlen(":") + 1);
	strcpy(op_text, line_text);
	strcat(op_text, ":");
	texture_t *t = dw_create_text_tex(op_text, C_WHITE);

	SDL_Rect r = cl_get_code_button_size();
	b->b = bt_create_btn(r, t);

	b->id = label;
	b->jptr = NULL;
	b->target_label_identity = 0;

	free(line_text);
	free(op_text);

	return b;
}

/* Function: cw_get_code_line_pos_by_ptr 
 * -----------------------------------------------------------------------------
 * This function traverses the list and determines the position in the list
 * of a given instruction
 *
 * Arguments:
 * 	line: Instruction that will be checked its position on the list
 *
 * Return:
 *	int with the position of the line in the code list
 */
/* Function: label_counter_up_to_index
 * -----------------------------------------------------------------------------
 * This function traverses the code up to an index and returns the value
 * of label instructions up to that point
 * 
 * Arguments:
 *  index: The index that the code list will be traversed up to
 *
 * Return:
 *	int with the number of label instructions
 */
static int label_counter_up_to_index(int index)
{
	int list_size = cp_count(g_code_window.program);
	assert(index < list_size && "index values is out of reach");

	int label_counter = 0;
	for (int position = 0; position <= index; position++) {
		code_line_t *line = cp_get(g_code_window.program, position);
		if (line != NULL && line->ins->id == LABEL) {
			label_counter++;
		}
	}
	return label_counter;
}

/* Function: get_label_operand_value
 * -----------------------------------------------------------------------------
 * Traverses the code list to return the correct label position value. The
 * instruction must be already added to the code list.
 * The 2 is added up to the value of label is because instruction 
 * position stars from 0, and the number of counted instructions from 1. The
 * extra one is because the i value of the instruction is counted.
 * 
 * Arguments:
 *  line: The label instruction that the operand will calculated.
 *
 * Return:
 *	int with the label value that will be assigned to the instruction
 */
static int get_label_operand_value(code_line_t *line)
{
	int label = NO_VALUE;
	check_mem(line);

	int pos = cp_index_of(g_code_window.program, line);
	label = pos;

	bool in_code_list = cp_contains(g_code_window.program, line);

	if (in_code_list == false){
		label = NO_VALUE;
	} else if (pos == 0){
		label = 1;
	 } else {
		label = pos - label_counter_up_to_index(pos) + 2;
	}
error:
	return label;
}

/* Function: cw_clone_rclicked_line
 * -----------------------------------------------------------------------------
 * Clones an instruction from the instruction window
 *
 * Arguments:
 * 	line: The line that will be cloned.
 *	
 * Return:
 *	The cloned code_line_object
 *
 */
static instruction_t *create_clone_instruction(
	const instruction_t *source, void *context)
{
	(void)context;
	texture_t *texture = cl_create_instruction_texture(source->id);
	if (texture == NULL) {
		return NULL;
	}
	btn_t *button = bt_create_btn(source->b->r, texture);
	if (button == NULL) {
		dw_free_texture(texture);
		return NULL;
	}
	instruction_t *instruction = cl_create_instruction(source->id, button);
	if (instruction == NULL) {
		bt_destroy_button(button);
		return NULL;
	}
	return instruction;
}

static operand_t *create_clone_operand(
	const operand_t *source, bool is_jump, void *context)
{
	(void)context;
	if (is_jump) {
		return create_jump_operand(source->jptr);
	}
	if (source->id > REG_MIN && source->id < REG_MAX) {
		return rg_create_register_operand_by_id(source->id);
	}
	if (source->id > BUF_MIN && source->id < BUF_MAX) {
		return bf_create_buffer_operand_by_id(source->id);
	}
	if (source->id > IMM_MIN && source->id < IMM_MAX) {
		return im_create_imm_op_by_id(source->id);
	}
	return NULL;
}

ce_result_t cw_clone_rclicked_line(code_line_t *line, code_line_t **clone)
{
	return ce_clone_line(line, create_clone_instruction, create_clone_operand,
	                     NULL, clone);
}


/* Function: cw_add_saved_line
 *------------------------------------------------------------------------------
 * Arguments:
 *	line_string: contains a string of the instruction that will be added
 *
 * Return:
 *	void.
 */
void cw_add_saved_line(char *line)
{
	assert(line != NULL && "The line value cannot be NULL");
	char *saveptr1;
	char *ins_text;
	char *op1_text;
	char *op2_text;
	char *delim = ax_char_space;
	int ins_id;
	int op1_id;
	int op2_id;

	ins_text = strtok_r(line, delim, &saveptr1);
	ins_id = cl_text_to_instruction_id(ins_text);
	
	int operand_quantity = cl_get_instruction_operand_quantity(ins_id);

	if (operand_quantity == ONE_OPERAND || operand_quantity == TWO_OPERANDS){
		if (ins_id == LABEL || cl_is_ins_jmp_type(ins_id) == true){
			op1_text =  strtok_r(NULL, delim, &saveptr1);
			op1_id = atoi(op1_text);
		} else {
			op1_text =  strtok_r(NULL, delim, &saveptr1);
			op1_id = op1_text == NULL || strcmp(op1_text, "_") == 0
				? NO_OPERAND : cl_text_to_operand_id(op1_text);
		}
	}
	
	if (operand_quantity == TWO_OPERANDS){
		op2_text =  strtok_r(NULL, delim, &saveptr1);
		op2_id = op2_text == NULL || strcmp(op2_text, "_") == 0
			? NO_OPERAND : cl_text_to_operand_id(op2_text);
	}
	
	int list_size = cp_count(g_code_window.program);
	ins_text = cl_get_instruction_text(ins_id);
	texture_t *instruction_tex = dw_create_text_tex(
						  		  ins_text, C_WHITE);
	
	int x = get_code_line_x(ins_id);
	int y = get_code_line_y(0);

	for (int i = 0; i <= list_size; i++){
		y += code_line_spacing();
	}
	
	SDL_Rect r = cl_get_code_button_size();
	r.x = x;
	r.y = y;
	btn_t *b = bt_create_btn(r, instruction_tex);
	check_mem(b);
	instruction_t *new_ins = cl_create_instruction(ins_id, b);
	code_line_t *new_line = cl_create_code_line(new_ins);
	
	if (operand_quantity == ONE_OPERAND || operand_quantity == TWO_OPERANDS){
		operand_t *op1 = NULL;
		if (cl_is_ins_jmp_type(ins_id) == true){
			op1 = create_saved_jump_operand(op1_id);
		} else if (ins_id == LABEL){
			op1 = create_saved_label_operand(op1_id);
		} else if (op1_id > REG_MIN && op1_id < REG_MAX){
			op1 = rg_create_register_operand_by_id(op1_id);
		} else if (op1_id > BUF_MIN && op1_id < BUF_MAX){
			op1 = bf_create_buffer_operand_by_id(op1_id);
		}
		new_line->op1 = op1;
	}
	if (operand_quantity == TWO_OPERANDS){
		operand_t *op2 = NULL;
		if (op2_id > REG_MIN && op2_id < REG_MAX){
			op2 = rg_create_register_operand_by_id(op2_id);
		} else if (op2_id > BUF_MIN && op2_id < BUF_MAX){
			op2 = bf_create_buffer_operand_by_id(op2_id);
		} else if (op2_id > IMM_MIN && op2_id < IMM_MAX){
			op2 = im_create_imm_op_by_id(op2_id);
		}
		new_line->op2 = op2;
	}

	new_line->state = COMPLETE;
	if (operand_quantity == TWO_OPERANDS) {
		new_line->state = new_line->op1 == NULL
			? (new_line->op2 == NULL ? MISSING_BOTH : MISSING_OP1)
			: (new_line->op2 == NULL ? MISSING_OP2 : COMPLETE);
	} else if (operand_quantity == ONE_OPERAND && new_line->op1 == NULL) {
		new_line->state = MISSING_OP1;
	}

	if (ce_insert_line(g_code_window.program, cp_count(g_code_window.program),
	                   new_line) != CE_OK) {
		cl_destroy_code_line(new_line);
	}
	
	error:
	return;
}


/* Function: cw_get_code_line_at_pos
 *------------------------------------------------------------------------------
 * Arguments:
 *	int: the position of the code_line that will be recovered.
 *
 * Return:
 *	The code line at a given position 
 */
/* Function: get_instruction_operand
 *------------------------------------------------------------------------------
 * Arguments:
 * instruction_pos: position of the instruction in the list.
 * operand_pos: the operand position.
 *
 * Return:
 *	id of the operand at given position for an instruction.
 */
/* Function: cw_get_instruction_at_code_pos
 *------------------------------------------------------------------------------
 * Arguments:
 *	int: the position of the instruction that wants to be recovered.
 *
 * Return:
 *	id of the instruction at a given position.
 */
/* Function: cw_get_code_list_size
 *------------------------------------------------------------------------------
 * Arguments:
 *	None.
 *
 * Return:
 *	Void.
 */
const code_program_t *cw_get_code_program(void)
{
	return g_code_window.program;
}

#ifdef CW_PROGRAM_TESTING
code_program_t *cw_get_program_for_testing(void)
{
	return g_code_window.program;
}
#endif

/* Function: cw_create_code_list
 *------------------------------------------------------------------------------
 * This function creates and initializes the code list.
 *
 * Arguments:
 *	None.
 *
 * Return:
 *	Void.
 *
 */
void cw_create_code_list(void)
{
	assert(g_code_window.program == NULL && "The code list is not NULL");
	g_code_window.program = cp_create(cl_destroy_code_line);
	assert(g_code_window.program != NULL && "Unable to create code program");
	g_code_window.next_label_identity = 1;
	g_code_window.edit_session = (ce_session_t){0};
	return;
}

/* Function: get_code_list
 *------------------------------------------------------------------------------
 * This function returns the pointer to the code list.
 *
 * Arguments:
 *	None.
 *
 * Return:
 *	Void.
 *
 */
/* Function: cw_get_clicked_code
 *------------------------------------------------------------------------------
 * This function returns a pointer to the selected code line
 *
 * Arguments:
 * 	none.
 *	
 * Return:
 *	the pointer to the specific button that has been clicked.
 *	NULL if not button was clicked
 *
 */
code_line_t *cw_get_clicked_code(void)
{
	code_line_t *clicked = NULL;
	for (int index = 0; index < cp_count(g_code_window.program); index++) {
		code_line_t *c = cp_get(g_code_window.program, index);
		if (true == bt_btn_clicked(c->ins->b)){
			clicked = c;
		} 
	}
	return clicked;
}

/* Function: cw_get_rclicked_code
 *------------------------------------------------------------------------------
 * This function returns a pointer to the selected code line with rclicked
 *
 * Arguments:
 * 	none.
 *	
 * Return:
 *	the pointer to the specific button that has been rclicked.
 *	NULL if not button was clicked
 *
 */
code_line_t *cw_get_rclicked_code(void)
{
	code_line_t *clicked = NULL;
	for (int index = 0; index < cp_count(g_code_window.program); index++) {
		code_line_t *c = cp_get(g_code_window.program, index);
		if (bt_btn_rclicked(c->ins->b) == true){
			clicked = c;
		} 
	}
	return clicked;
}

/* Function: cw_chk_rclick_code
 *------------------------------------------------------------------------------
 * This functions verifies if the user right clicked one of the code lines
 *
 * Arguments:
 * 	none.
 *	
 * Return:
 *	true if the user clicked over an code instruction, false if otherwise
 *
 */
bool cw_chk_rclick_code(void)
{
	bool clicked = false;

	if (sb_chk_rst_esc_menu_active() == false){
		for (int index = 0; index < cp_count(g_code_window.program); index++) {
			code_line_t *c = cp_get(g_code_window.program, index);
			if (bt_btn_rclicked(c->ins->b) == true){
				if (c->ins->id == LABEL){
					clicked = false;
				} else {
					clicked = true;
				}
				break;
			} 
		}
	}
	return clicked;
}

/* Function: cw_chk_click_code
 *------------------------------------------------------------------------------
 * This functions verifies if the user clicked one of the code developed by
 * the player
 *
 * Arguments:
 * 	none.
 *	
 * Return:
 *	true if the user clicked over an instructions, false if otherwise
 *
 */
bool cw_chk_click_code(void)
{
	bool clicked = false;
	
	if (sb_chk_rst_esc_menu_active() == false){
		if (ms_click_inside_rect(g_code_box) == true){
			for (int index = 0; index < cp_count(g_code_window.program); index++) {
				code_line_t *c = cp_get(g_code_window.program, index);
				if (true == bt_btn_clicked(c->ins->b)){
					clicked = true;
					break;
				} 
			}
		}
	}
	return clicked;
}

/* Function: cw_set_stage_name
 * -----------------------------------------------------------------------------
 * This function is called to set the string that will be displayed as the 
 * the stage name
 *
 * Arguments:
 * text: the text that will be set on the challenge text
 *
 * Return:
 *	void
 */
void cw_set_stage_name(char *text)
{
	assert(NULL != text && "The text pointer is NULL");

	dw_free_texture(g_stage_name);
	g_stage_name = NULL;
	g_stage_name = dw_create_text_tex(text, C_AMBER);
	check_mem(g_stage_name);
error:
	return;
}

/* Function: cw_clear_code_window_assets
 * -----------------------------------------------------------------------------
 * Clears the whole assets related to the code window when leaving a level
 *
 * Arguments:
 * 	None.
 *
 * Return:
 *	Void.
 */
void cw_destroy_code_window_assets(void)
{
	release_active_jump_replacement_claims();
	if (g_code_window.edit_session.state != CE_IDLE) {
		ce_cancel(&g_code_window.edit_session);
	}
	dw_free_texture(g_stage_name);
	g_stage_name = NULL;
	tx_free_styled_text(g_challenge_text);
	g_challenge_text = NULL;
	g_code_window.edit_session = (ce_session_t){0};
	cp_destroy(g_code_window.program);
	g_code_window.program = NULL;
	g_code_window.next_label_identity = 0;
	g_code_window.pending_scroll_y = 0;
}

void cw_destroy_renderer_assets(void)
{
	cr_assets_destroy(&g_code_window.renderer_assets);
}


/* Function: cw_get_code_box_member
 * -----------------------------------------------------------------------------
 * This function access and returns the members of the code box object.
 *
 * Arguments:
 * member: The specific member that is being requested.
 *
 * Return:
 *	The accessed member.
 */
static SDL_Rect get_scroll_box(void)
{
	return g_scroll_box;
}

/* Function: set_code_box_member
 * -----------------------------------------------------------------------------
 * This function modifies an specific member of the code box.
 *
 * Arguments:
 * value: the value that is going to be set.
 * member: The specific member that is gonna be modified.
 *
 * Return:
 *	void.
 */
static void set_code_box_member(int value, int member)
{
	assert(member > MEMBER_MIN && member < MEMBER_MAX && "Invalid member");

	switch (member){
		
		case MEMBER_X:
			g_scroll_box.x = value;
			break;
		case MEMBER_Y:
			g_scroll_box.y = value;
			break;
		case MEMBER_W:
			g_scroll_box.w = value;
			break;
		case MEMBER_H:
			g_scroll_box.h = value;
			break;
		default:
			perror("Error: defaulting on invalid case");
			break;
	}
	return;
}

/* Function: set_text_box_member
 * -----------------------------------------------------------------------------
 * This function modifies an specific member of the text box.
 *
 * Arguments:
 * value: the value that is going to be set.
 * member: The specific member that is gonna be modified.
 *
 * Return:
 *	void.
 */
static void set_text_box_member(int value, int member)
{
	assert(member >= MEMBER_X && member <= MEMBER_H &&  "Member is incorrect");

	switch (member){
		
		case MEMBER_X:
			g_text_box.x = value;
			break;
		case MEMBER_Y:
			g_text_box.y = value;
			break;
		case MEMBER_W:
			g_text_box.w = value;
			break;
		case MEMBER_H:
			g_text_box.h = value;
			break;
		default:
			perror("Error: defaulting on invalid case");
			break;
	}
	return;
}



/* Function: cw_get_text_box_rect
 * -----------------------------------------------------------------------------
 * Returns the rectangle of the text boox
 *
 * Arguments:
 * 	Void.
 *
 * Return:
 *	The rectangle of the text box
 */
SDL_Rect cw_get_text_box_rect(void)
{
	return g_text_box;
}

float cw_get_challenge_highlight_limit(void)
{
	int h = ty_message_height();
	if (g_text_box.w <= 0 || g_text_box.h <= 0) {
		return (float)dw_get_animation_limit();
	}

	float anim_limit = (float)dw_get_animation_limit();
	float left_space = g_text_box.x - g_code_box.x;
	float right_space = g_code_box.x + g_code_box.w -
	                    g_text_box.x - g_text_box.w;
	float horizontal_space = left_space < right_space ? left_space : right_space;
	float top_space = g_text_box.y - g_code_box.y;
	float bottom_space = g_code_box.y + g_code_box.h -
	                     g_text_box.y - g_text_box.h;
	float vertical_space = top_space < bottom_space ? top_space : bottom_space;
	float horizontal_limit =
	    2.0f * horizontal_space * h / g_text_box.w;
	float vertical_limit =
	    2.0f * vertical_space * h / g_text_box.h;
	if (horizontal_limit < anim_limit) {
		anim_limit = horizontal_limit > 0.0f ? horizontal_limit : 0.0f;
	}
	if (vertical_limit < anim_limit) {
		anim_limit = vertical_limit > 0.0f ? vertical_limit : 0.0f;
	}
	return anim_limit;
}

bool cw_get_code_box_highlight_progress(float *progress,
										float animation_limit)
{
	if (progress == NULL || !g_code_box_highlight ||
	    !isfinite(animation_limit) || animation_limit <= 0.0f) {
		return false;
	}
	*progress = ha_get_progress();
	return true;
}

bool cw_get_challenge_highlight_progress(float *progress,
										float animation_limit)
{
	if (progress == NULL || !g_challenge_highlight ||
	    !isfinite(animation_limit) || animation_limit <= 0.0f) {
		return false;
	}
	*progress = ha_get_progress();
	return true;
}

/* Function: in_code_window
 * -----------------------------------------------------------------------------
 * This function verifies if the mouse coordinates are inside of one of the 
 * spaces where a instruction can be assigned.
 *
 * Arguments:
 * mouse: the mouse object that will be evaluated.	
 *
 * Return:
 *	Void.
 */
static bool in_code_window(void){
	int mouse_x = ms_get_mouse_x();
	int mouse_y = ms_get_mouse_y();
	return cly_rect_contains_point(
		g_scroll_box, mouse_x, mouse_y);
}

bool cw_calculate_challenge_frame_rect(
	SDL_Rect text_bounds, SDL_Rect container, int allowance, SDL_Rect *frame)
{
	return cly_calculate_challenge_frame_rect(
		text_bounds, container, allowance, frame);
}

/* Function: cw_set_challenge_text
 * -----------------------------------------------------------------------------
 * This function is called to set the string that will be displayed as the 
 * challenge text.
 *
 * Arguments:
 * text: the text that will be set on the challenge text
 *
 * Return:
 *	void
 */
void cw_set_challenge_text(const char *text)
{
	assert(NULL != text && "The text pointer is NULL");
	tx_free_styled_text(g_challenge_text);
	g_challenge_text = NULL;
	if (text[0] == '\0') {
		return;
	}
	int w = cw_get_code_text_width();
	int h = ty_message_height();
	const tx_styled_text_options_t options = {
		.highlight_arch_tokens = false,
		.parse_authoring_prefixes = false,
		.body_color = C_SILVERGREY
	};
	g_challenge_text = cw_invoke_challenge_text_factory(w, h, text, &options);
	if (g_challenge_text == NULL) {
		fprintf(stderr, "Unable to create styled challenge text\n");
	}
}

void cw_set_challenge_highlight(bool enabled)
{
	g_challenge_highlight = enabled;
}

void cw_set_code_box_highlight(bool enabled)
{
	g_code_box_highlight = enabled;
}

/* Function: cw_set_scroll_box
 * -----------------------------------------------------------------------------
 * This function is called to set the value on the code box object
 *
 * Arguments:
 * x: x coordinate of the box
 * y: y coordinate of the box
 * w: width of the box
 * h: height of the box
 *
 * Return:
 *	void
 */
void cw_set_code_box(SDL_Rect r)
{
	g_code_box.x = r.x;
	g_code_box.y = r.y;
	g_code_box.w = r.w;
	g_code_box.h = r.h;

}

/* Function: cw_set_scroll_box
 * -----------------------------------------------------------------------------
 * This function is called to set the value on the code box object
 *
 * Arguments:
 * x: x coordinate of the box
 * y: y coordinate of the box
 * w: width of the box
 * h: height of the box
 *
 * Return:
 *	void
 */
void cw_set_scroll_box(SDL_Rect r)
{
	g_scroll_box.x = r.x;
	g_scroll_box.y = r.y;
	g_scroll_box.w = r.w;
	g_scroll_box.h = r.h;

	int w = cw_get_code_text_width();
	int h = ty_message_height();

	int text_h = ty_emphasis_height();
	int border_ofs = um_padding_horizontal_with_border();
	int text_box_height = g_challenge_text != NULL
	                          ? g_challenge_text->row_count * h
	                          : 0;
	set_text_box(r.x + border_ofs, 
				 r.y + border_ofs + text_h + border_ofs, 
				 w, 
				 text_box_height);
}

/* Function: set_text_box
 * -----------------------------------------------------------------------------
 * This function is called to set the value on the text box object
 *
 * Arguments:
 * x: x coordinate of the box
 * y: y coordinate of the box
 * w: width of the box
 * h: height of the box
 *
 * Return:
 *	void
 */
static void set_text_box(int x, int y, int w, int h)
{
	g_text_box.x = x;
	g_text_box.y = y;
	g_text_box.w = w;
	g_text_box.h = h;
}


/* Function: get_code_line_x
 * -----------------------------------------------------------------------------
 * This function calculates and returns the x position of the lines of code
 * that will be incorporated in the code box. IF it is a label instruction
 * it will provide the correct x value.
 *
 * Arguments:
 * 	none.
 *
 * Return:
 *	int with the coordinate of the x coordinates for the lines of code
 */
static int get_code_line_x(int instruction_id)
{
	int x;  
	int number_ofs = um_padding_horizontal_with_border() + CODE_CONTENT_X_ADJUSTMENT;
	SDL_Rect sb = cw_get_stage_code_box();
	int number_h = ty_code_height();
	int number_w = ax_get_texture_w_fit_h(
		number_h, g_code_window.renderer_assets.line_numbers[0]);
	if (instruction_id == LABEL){
		x = sb.x;
	} else {
		x = sb.x + number_ofs + number_w; 	
	}
	return x;
}


/* Function: code_box_height_adjust
 * -----------------------------------------------------------------------------
 * This function handles the length of the code box handler according to the 
 * number of code lines done by the player.
 *
 * Arguments:
 * 	none.
 *
 * Return:
 *	void.
 */
static void code_box_height_adjust(void)
{
	int list_size = cp_count(g_code_window.program);

	SDL_Rect codbox = get_scroll_box();
	SDL_Rect ogcodbox = cw_get_stage_code_box();
	
	int y = get_code_line_y(list_size - 1);
	if ( y > (ogcodbox.h + ogcodbox.y)){
		
		int inc = y + code_line_spacing() - (codbox.h + codbox.y);
		int new_bottom_border = codbox.y + codbox.h + inc;
		int cur_bottom_border = codbox.h + codbox.y;
		
		int mdelta = ax_get_cw_move_delta();
		int delta = ax_get_movement_delta(cur_bottom_border, 
									   	  new_bottom_border, 
									   	  mdelta);
		
		if (new_bottom_border > cur_bottom_border){
			cur_bottom_border += delta;
			set_code_box_member(cur_bottom_border - codbox.y, MEMBER_H);
		} else if (new_bottom_border < cur_bottom_border){
			cur_bottom_border -= delta;
			set_code_box_member(cur_bottom_border - codbox.y, MEMBER_H);
		} 
		
	}
}
/* Function: adjust_code_box_position
 * -----------------------------------------------------------------------------
 * This function handles the length position of the code box according to mouse
 * scroll.
 *
 * Arguments:
 * 	none.
 *
 * Return:
 *	void.
 */
static void adjust_code_box_position(void)
{
	g_code_window.pending_scroll_y += 3*ms_get_mouse_scroll_y();
	SDL_Rect cb = cw_get_stage_code_box();
	SDL_Rect codbox = get_scroll_box();

	int cur_y = codbox.y;
	int max_upper_border_y = cb.y;
	int min_lower_border_y = cb.y + cb.h;

	if (cur_y >= max_upper_border_y && g_code_window.pending_scroll_y > 0){
		g_code_window.pending_scroll_y = 0;
	} 
	if ((cur_y + codbox.h) <= min_lower_border_y &&
	    g_code_window.pending_scroll_y < 0){
		g_code_window.pending_scroll_y = 0;
	}

	int mdelta = ax_get_cw_move_delta();
	int delta = ax_get_movement_delta(
		0, g_code_window.pending_scroll_y, mdelta);

	if (g_code_window.pending_scroll_y != 0){
		SDL_Rect tb = cw_get_text_box_rect();
		int text_box_y = tb.y;
		
		if (g_code_window.pending_scroll_y < 0){
			cur_y -= delta;
			set_code_box_member(cur_y, MEMBER_Y);
			text_box_y -= delta;
			set_text_box_member(text_box_y, MEMBER_Y);
			g_code_window.pending_scroll_y += delta;
		} else if (g_code_window.pending_scroll_y > 0){
			cur_y += delta;
			set_code_box_member(cur_y, MEMBER_Y);
			text_box_y += delta;
			set_text_box_member(text_box_y, MEMBER_Y);
			g_code_window.pending_scroll_y -= delta;
		}
	}
}

/* Function: cw_draw_code_window
 * -----------------------------------------------------------------------------
 * This function is called to draw all the elements related to the code window
 *
 * Arguments:
 * 	none.
 *
 * Return:
 *	void.
 */
void cw_draw_code_window(void)
{
	code_box_height_adjust();
	if (in_code_window()) {
		adjust_code_box_position();
	}
	float animation_limit = cw_get_challenge_highlight_limit();
	cr_visual_state_t visual_state = get_code_renderer_visual_state();
	SDL_Rect stage_code_box = cw_get_stage_code_box();
	cr_draw_code_window_presentation(
		g_code_window.program, &g_code_window.renderer_assets, &visual_state,
		cw_get_code_line_pending_operand(), cw_get_comma_offset(),
		sb_chk_rst_esc_menu_active(), g_code_box, g_scroll_box, g_text_box,
		g_stage_name, g_challenge_text, g_code_box_highlight,
		g_challenge_highlight, animation_limit,
		&g_code_box_highlight_value, &g_challenge_highlight_value,
		stage_code_box.x, get_code_line_y(0), code_line_spacing(),
		ty_code_height(), ty_message_height(), ty_emphasis_height(),
		um_border_width(), um_padding_horizontal_with_border(),
		CODE_CONTENT_X_ADJUSTMENT);
	}
void cw_sort_code(void)
{
	int list_size = cp_count(g_code_window.program);
	
	if (0 == list_size){
		return;
	}
	
	int op1_ofs = cw_get_operand1_offset();
	int op2_ofs = cw_get_operand2_offset();
	
	int y = get_code_line_y(0);

	int mdelta = ax_get_cw_move_delta();
	for (int index = 0; index < list_size; index++) {
		code_line_t *line = cp_get(g_code_window.program, index);
		int x = get_code_line_x(line->ins->id);
		int delta = ax_get_movement_delta(line->ins->b->r.x, x, mdelta);

		if (line->ins->b->r.x < x){
			line->ins->b->r.x += delta;
		} else if (line->ins->b->r.x > x){
			line->ins->b->r.x -= delta;
		}
		delta = ax_get_movement_delta(line->ins->b->r.y, y, mdelta);

		if (line->ins->b->r.y < y){
			line->ins->b->r.y += delta;
		} else if (line->ins->b->r.y > y){
			line->ins->b->r.y -= delta;
		}
		if (line->op1 != NULL){
				line->op1->b->r.x = line->ins->b->r.x + op1_ofs;
				line->op1->b->r.y = line->ins->b->r.y;
		}
		if (line->op2 != NULL){
				line->op2->b->r.x = line->ins->b->r.x + op2_ofs;
				line->op2->b->r.y = line->ins->b->r.y;
		}
		y += code_line_spacing();

	}
	return ;
}

/* Function: get_list_node_by_value
 * -----------------------------------------------------------------------------
 * This function returns the corresponding node of a give value for that node 
 * it assumes that each node has a unique value pointer
 *
 * Arguments:
 * 	line: Line that is going to be verified if already in list.
 *
 * Return:
 *	pointer to the node, NULL if node is not found
 */

/* Function: cw_check_if_in_code_list
 * -----------------------------------------------------------------------------
 * This function verifies if a selected element of the screen is already in
 * a list, to avoide adding the same element multiple times.
 *
 *
 * Arguments:
 * 	lin: Node that is going to be verified if already in list.
 *
 * Return:
 *	true if already present, false if otherwise.
 */


/* Function: get_code_line_position
 * ----------------------------------------------------------------------------
 * This function receives a y coordinate and determines the position in the 
 * linked list of the instruction corresponding to that coordinate.
 *
 * Arguments:
 * y: the y coordinate used to determine the code line position.
 *
 * Return:
 *	The position of the code line according, -1 if the coordinate do not 
 * 	corresponds to any code line.
 */
int get_code_line_position(int y)
{
	return cly_code_line_position_at_y(
		get_code_line_y(0), code_line_spacing(),
		cp_count(g_code_window.program), y);
}


/* Function: add_code_line_last_pos
 * ----------------------------------------------------------------------------
 * This function adds a code line at the last position of the codelist
 *
 * Arguments:
 * instruction: the instruction to be added to the list
 *
 * Return:
 *	Void.
 */
static void add_code_line_last_pos(code_line_t *line)
{
	assert(NULL != line &&  "The code line object is NULL");

		
	assert(ce_insert_line(g_code_window.program,
	                      cp_count(g_code_window.program), line) == CE_OK);
	return;
}

/* Function: add_code_line
 * ----------------------------------------------------------------------------
 * This function adds a code line to the respective code list, in the correct
 * position according to mouse coordinates.
 *
 * Arguments:
 * instruction: the instruction to be added to the list
 *
 * Return:
 *	Void.
 */
static void add_code_line(code_line_t *line)
{
	assert(NULL != line &&  "The code line object is NULL");

	bool already_present = cp_contains(g_code_window.program, line);
	int list_count = cp_count(g_code_window.program) - (already_present ? 1 : 0);
	int destination = list_count;
		
	if (0 == list_count){
		destination = 0;
	} else {
		int mouse_y = ms_get_mouse_y();
		code_line_t *first = cp_get(g_code_window.program, 0);
		if (already_present && first == line) {
			first = cp_get(g_code_window.program, 1);
		}
		int y = first->ins->b->r.y;
		if (mouse_y < y) {
			destination = 0;
		} else {
			code_line_t *last = cp_get(g_code_window.program,
			                           cp_count(g_code_window.program) - 1);
			if (already_present && last == line) {
				last = cp_get(g_code_window.program,
				              cp_count(g_code_window.program) - 2);
			}
			y = last->ins->b->r.y;
			int height = last->ins->b->r.h;
			if (mouse_y < y + height) {
				int row_y = get_code_line_y(0);
				int spacing = code_line_spacing();
				destination = NOT_FOUND;
				for (int index = 0; index < list_count; index++) {
					if (mouse_y >= row_y && mouse_y < row_y + spacing) {
						destination = index;
						break;
					}
					row_y += spacing;
				}
				if (destination == NOT_FOUND) {
					destination = list_count;
				}
			}
		}
	} 
	if (already_present) {
		ce_begin(&g_code_window.edit_session, line, CE_MOVING_LINE);
		assert(ce_move_line(
			g_code_window.program, line, destination) == CE_OK);
		ce_cancel(&g_code_window.edit_session);
	} else {
		assert(ce_insert_line(
			g_code_window.program, destination, line) == CE_OK);
	}
}

/* Function: chk_sel_line_in_pos
 * -----------------------------------------------------------------------------
 * This function verifies if the code line is in it's correct line coordinate
 * according to it's position in the list. If the line is not in it's y
 * coordinate range, it is removed. The position is measured using mouse
 * coordinates, so this function works if the line is being selected with the
 * mouse.
 *
 * Arguments:
 * line: the instruction that the mouse object is carrying
 *
 * Return:
 *	Void.
 */
static bool chk_sel_line_in_pos(code_line_t *line)
{
	assert(NULL != line && "The evaluated line is NULL");
	
	int list_size = cp_count(g_code_window.program);
	
	if (0 == list_size){
		return false;
	}
	
	int y = get_code_line_y(0);

	int mouse_y = ms_get_mouse_y();
	bool ret = false;
	SDL_Rect cb = cl_get_code_button_size();
	int h = cb.h;
	if (mouse_y < y || mouse_y >= y + list_size*h){
		ret = false;
	} else {
		for (int index = 0; index < list_size; index++) {
			if (cp_get(g_code_window.program, index) == line){
				if (mouse_y >= y && mouse_y < y + h){
					ret =  true;	
				}
			}
			y += code_line_spacing();
		}
	}
	return ret;
}

/* Function: cw_clear_code_list
 * -----------------------------------------------------------------------------
 * This function removes all the instructions contained in the code list
 *
 * Arguments:
 *  Void.
 *
 * Return:
 *	Void.
 */
void cw_clear_code_list(void)
{
	release_active_jump_replacement_claims();
	if (g_code_window.edit_session.state != CE_IDLE) {
		ce_cancel(&g_code_window.edit_session);
	}
	cp_clear(g_code_window.program);
}


/* Function: cw_player_holding_instruction
 * -----------------------------------------------------------------------------
 * This function contains the window were the programmed code is going to be
 * it will contain the list of instructions to be executed and it will high
 * where the instruction can be located. Also, determines if a instruction
 * can be rearranged according to others.
 *
 * Arguments:
 * 	instruction: the instruction that the mouse object is carrying
 *  arrange: boolean indicating if the instructions can be arrangeable.
 *	del: boolean indicating if the code can be deleted
 *
 * Return:
 *	Void.
 */
void cw_player_holding_instruction(code_line_t *line, bool arng, bool del)
{
	assert(g_code_window.program != NULL && "Code list is NULL");
	assert(line != NULL &&  "The code line object is NULL");
	cw_cancel_pending_operand_change();
	
	if (in_code_window() == true || line->ins->id == LABEL || del == false){
		if (cp_contains(g_code_window.program, line)){
			if (chk_sel_line_in_pos(line) == false && arng == true){
				add_code_line(line);
				update_label_instructions();
				update_jump_instructions();
			}
		} else if (!cp_contains(g_code_window.program, line) && arng == true){
			add_code_line(line);
			update_label_instructions();
			update_jump_instructions();
		} else if (!cp_contains(g_code_window.program, line) && arng == false){
			add_code_line_last_pos(line);
			update_label_instructions();
			update_jump_instructions();
		}
		if (line->ins->id == LABEL){
			ensure_label_identity(line);
			if (line->op1 == NULL){
				line->op1 = create_label_operand(line);
				line->state = COMPLETE;
			} else {
				cl_destroy_operand(line->op1);	
				line->op1 = create_label_operand(line);
				line->state = COMPLETE;
			}
		}
	} else {
		if (cp_contains(g_code_window.program, line)){
			if (cl_is_ins_jmp_type(line->ins->id) == true && line->op1 != NULL){
				code_line_t *addr = line->op1->jptr;
				int index = cp_index_of(g_code_window.program, addr);
				if (index >= 0) {
					code_line_t *removed = NULL;
					assert(ce_remove_line(
						g_code_window.program, index, &removed) == CE_OK);
					cl_destroy_code_line(removed);
				}
			}
			int index = cp_index_of(g_code_window.program, line);
			code_line_t *removed = NULL;
			assert(ce_remove_line(
				g_code_window.program, index, &removed) == CE_OK);
			assert(removed == line);
			update_label_instructions();
			update_jump_instructions();
		}
	}

	line->ins->b->r.x = ms_get_mouse_x() - line->ins->b->r.w/2;
	line->ins->b->r.y = ms_get_mouse_y() - line->ins->b->r.h/2;
	int op1_ofs = cw_get_operand1_offset();
	int op2_ofs = cw_get_operand2_offset();

	if (line->op1 != NULL){
		line->op1->b->r.x = line->ins->b->r.x + op1_ofs;
		line->op1->b->r.y = line->ins->b->r.y;
	}
	if (line->op2 != NULL){
		line->op2->b->r.x = line->ins->b->r.x + op2_ofs;
		line->op2->b->r.y = line->ins->b->r.y;
	}
	cr_visual_state_t visual_state = get_code_renderer_visual_state();
	cr_draw_code_line(
		line, cw_get_code_line_pending_operand(), &visual_state,
		em_get_escape_state());
}

/* Function: get_line_pending_operand
 * -----------------------------------------------------------------------------
 * Returns the first changing line, or the first missing line if none is changing.
 *
 * Arguments:
 * 	none.
 *	
 * Return:
 * 	pointer to the active pending code line, or NULL if none is found.
 *
 */
code_line_t *cw_get_code_line_pending_operand(void)
{
	if (g_code_window.program == NULL) {
		return NULL;
	}

	for (int index = 0; index < cp_count(g_code_window.program); index++) {
		code_line_t *c = cp_get(g_code_window.program, index);
		int s = c->state;
		if (CHANGING_OP1 == s || CHANGING_OP2 == s){
			return c;
		}
	}
	for (int index = 0; index < cp_count(g_code_window.program); index++) {
		code_line_t *c = cp_get(g_code_window.program, index);
		int s = c->state;
		if (MISSING_BOTH == s || MISSING_OP1 == s || MISSING_OP2 == s){
			return c;
		}
	}
	return NULL;
}

static bool get_pending_operand_slot_rect(const code_line_t *line,
										 SDL_Rect *slot_rect)
{
	if (line == NULL || line->ins == NULL || line->ins->b == NULL ||
	    slot_rect == NULL || line->ins->b->r.w <= 0 ||
	    line->ins->b->r.h <= 0) {
		return false;
	}

	int operand_offset;
	switch (line->state) {
	case MISSING_BOTH:
	case MISSING_OP1:
	case CHANGING_OP1:
		operand_offset = cw_get_operand1_offset();
		break;
	case MISSING_OP2:
	case CHANGING_OP2:
		operand_offset = cw_get_operand2_offset();
		break;
	default:
		return false;
	}

	*slot_rect = (SDL_Rect){
		.x = line->ins->b->r.x + operand_offset,
		.y = line->ins->b->r.y,
		.w = line->ins->b->r.w,
		.h = line->ins->b->r.h
	};
	return true;
}

static bool get_animated_pending_operand_rect(float normalized_progress,
											 SDL_Rect *animated_operand_rect)
{
	SDL_Rect base_operand_rect;
	if (!get_pending_operand_slot_rect(cw_get_code_line_pending_operand(),
	                                  &base_operand_rect)) {
		return false;
	}
	return cw_calculate_pending_operand_rect(
		base_operand_rect, normalized_progress, dw_get_animation_limit(),
		animated_operand_rect);
}


/* Function: cw_highlight_code_pending_operand
 * -----------------------------------------------------------------------------
 * Draw the pending or replacing operand slot using the shared frame phase.
 * The primary gameplay render path owns ha_begin_frame().
 *
 */
void cw_highlight_code_pending_operand(void)
{
	code_line_t *pending_line = cw_get_code_line_pending_operand();
	if (!cw_should_draw_pending_operand_rect(pending_line, pending_line)) {
		return;
	}
	SDL_Rect animated_operand_rect;
	if (get_animated_pending_operand_rect(ha_get_progress(),
	                                      &animated_operand_rect)) {
		dw_draw_thick_rect(animated_operand_rect,
		                   cw_get_missing_operand_width(), C_WHITE);
	}
}

/* Function: cw_check_code_sorted
 * -----------------------------------------------------------------------------
 * This function verifies if all the lines of code are in their position
 *
 * Arguments:
 * 	code: the list of code programmed by the player
 *	
 * Return:
 * 	true if all code is in position, false if otherwise.
 *
 */
bool cw_check_code_sorted(void)
{
	int list_size = cp_count(g_code_window.program);
	if (0 == list_size){
		return true;
	}
	
	int y = get_code_line_y(0);

	int retval = true;
	for (int index = 0; index < list_size; index++) {
		code_line_t *value = cp_get(g_code_window.program, index);
		btn_t *b = value->ins->b;
		int x = get_code_line_x(value->ins->id);

		if (b->r.x < x || b->r.x > x || b->r.y < y || b->r.y > y){
			retval = false;
			break;
		} 
		y += code_line_spacing();
	}	
	return retval;
}

/* Function: cw_chk_click_code_op2
 * -----------------------------------------------------------------------------
 * Verifies if the player clicked the operand 2 of the code
 *
 * Arguments:
 * 	code_line_pos: The position of the codeline that will be verified
 *	
 * Return:
 * 	true if player clicked on operand 2.
 *	false if otherwise.
 *
 */
bool cw_chk_click_code_op(void)
{
	bool clicked = false;
	if (!sb_chk_rst_esc_menu_active()) {
		for (int index = 0; index < cp_count(g_code_window.program); index++) {
			code_line_t *line = cp_get(g_code_window.program, index);
			if ((line->op1 != NULL && bt_chk_rel_btn(line->op1->b, NULL)) ||
			    (line->op2 != NULL && bt_chk_rel_btn(line->op2->b, NULL))) {
				clicked = true;
				break;
			}
		}
	}
	return clicked;
}
static void reset_button_animation(btn_t *button)
{
	if (button == NULL) {
		return;
	}
	button->animated = false;
	button->anim_dir = false;
	button->anim_state = 0;
}

void cw_change_clicked_code_line_state(void)
{
	code_line_t *selected_line = NULL;
	int selected_operand = NO_OPERAND;
	int code_size = cp_count(g_code_window.program);
	for (int index = 0; index < code_size; index++) {
		code_line_t *c = cp_get(g_code_window.program, index);
		if (c == NULL || c->ins == NULL || c->ins->id == LABEL){
			continue;
		}
		if (c->op1 != NULL && c->op1->b != NULL &&
			bt_chk_rel_btn(c->op1->b, NULL) == true){
			selected_line = c;
			selected_operand = OP1;
			break;
		}
		if (cl_is_ins_jmp_type(c->ins->id) == false &&
			c->op2 != NULL && c->op2->b != NULL &&
			bt_chk_rel_btn(c->op2->b, NULL) == true){
			selected_line = c;
			selected_operand = OP2;
			break;
		}
	}
	if (selected_line == NULL){
		return;
	}
	code_line_t *pending_line = cw_get_code_line_pending_operand();
	bool same_active_jump = selected_operand == OP1 &&
		cw_is_active_jump_operand_replacement(selected_line, pending_line);
	if (!same_active_jump) {
		release_active_jump_replacement_claims();
	}

	for (int index = 0; index < code_size; index++) {
		code_line_t *c = cp_get(g_code_window.program, index);
		if (c == NULL || c->ins == NULL ||
			(c->state != CHANGING_OP1 && c->state != CHANGING_OP2)){
			continue;
		}
		if (!cl_is_ins_jmp_type(c->ins->id)) {
			if (c->op1 != NULL && c->op1->b != NULL){
				reset_button_animation(c->op1->b);
			}
			if (c->op2 != NULL && c->op2->b != NULL){
				reset_button_animation(c->op2->b);
			}
		}
	}

	btn_t *selected_button = selected_operand == OP1 ?
		selected_line->op1->b : selected_line->op2->b;
	ce_result_t selection_result = ce_select_operand(
		g_code_window.program, &g_code_window.edit_session, selected_line,
		selected_operand == OP1 ? CE_OPERAND_SLOT_1 : CE_OPERAND_SLOT_2);
	assert(selection_result == CE_OK);
	if (selection_result != CE_OK) {
		return;
	}
	bool selected_jump_replacement = cw_is_active_jump_operand_replacement(
		selected_line, cw_get_code_line_pending_operand());
	if (!selected_jump_replacement) {
		reset_button_animation(selected_button);
	}
	if (selected_jump_replacement) {
		start_active_jump_replacement_highlight(selected_line);
	}
}

void cw_cancel_pending_operand_change(void)
{
	release_active_jump_replacement_claims();
	code_line_t *line = g_code_window.edit_session.line;
	if (g_code_window.edit_session.state == CE_IDLE || line == NULL) {
		line = cw_get_code_line_pending_operand();
		if (line == NULL || line->ins == NULL ||
		    (line->state != CHANGING_OP1 && line->state != CHANGING_OP2)) {
			return;
		}
		ce_session_state_t state = line->state == CHANGING_OP1
			? CE_REPLACING_OPERAND_1 : CE_REPLACING_OPERAND_2;
		ce_begin(&g_code_window.edit_session, line, state);
	}
	if (ce_cancel(&g_code_window.edit_session) != CE_OK) {
		return;
	}

	int quantity = cl_get_instruction_operand_quantity(line->ins->id);
	if (!cl_is_ins_jmp_type(line->ins->id)) {
		reset_button_animation(line->op1 != NULL ? line->op1->b : NULL);
	}
	if (quantity == TWO_OPERANDS) {
		reset_button_animation(line->op2 != NULL ? line->op2->b : NULL);
	}
}


/* Function: cw_assign_op_to_line
 * -----------------------------------------------------------------------------
 * This function assigns an operand to a line that has a state where the 
 * operand is missing, generates error if called to a line that is not in a
 * state of a missing operand.
 *
 * Arguments:
 *	operand: the operand that will be assigned.
 * 	line: the line where the operand will be assigned.
 *	
 * Return:
 *	void.
 *
 */
static bool try_assign_op_to_line(operand_t *op, code_line_t *line)
{
	if (op == NULL || line == NULL || line->ins == NULL ||
	    !cp_contains(g_code_window.program, line)) {
		return false;
	}
	if (cl_is_ins_jmp_type(line->ins->id) &&
	    (!cp_contains(g_code_window.program, op->jptr) ||
	     !ensure_label_identity(op->jptr))) {
		return false;
	}
	release_active_jump_replacement_claims();

	ce_session_state_t state;
	switch (line->state) {
	case MISSING_BOTH:
	case MISSING_OP1:
	case CHANGING_OP1:
		state = CE_REPLACING_OPERAND_1;
		break;
	case MISSING_OP2:
	case CHANGING_OP2:
		state = CE_REPLACING_OPERAND_2;
		break;
	default:
		return false;
	}
	if (g_code_window.edit_session.line != line ||
	    g_code_window.edit_session.state != state) {
		ce_begin(&g_code_window.edit_session, line, state);
	}
	operand_t *replaced = NULL;
	if (ce_replace_operand(&g_code_window.edit_session, op, &replaced) != CE_OK) {
		return false;
	}
	int offset = state == CE_REPLACING_OPERAND_1
		? cw_get_operand1_offset() : cw_get_operand2_offset();
	if (op->b != NULL) {
		op->b->r.x = line->ins->b->r.x + offset;
		op->b->r.y = line->ins->b->r.y;
		reset_button_animation(op->b);
	}
	if (replaced != NULL) {
		cl_destroy_operand(replaced);
	}
	return true;
}

void cw_assign_op_to_line(operand_t *op, code_line_t *line)
{
	assert(NULL != op && "The operand cannot be NULL");
	assert(NULL != line && "The line cannot be NULL");
	if (!try_assign_op_to_line(op, line)) {
		cl_destroy_operand(op);
	}
}

code_line_t *cw_create_automatic_jump_label(code_line_t *jump)
{
	if (!cp_contains(g_code_window.program, jump) || jump->ins == NULL ||
	    !cl_is_ins_jmp_type(jump->ins->id) || jump->state != MISSING_OP1 ||
	    jump->op1 != NULL) {
		return NULL;
	}
	code_line_t *label = create_label_code_line();
	if (label == NULL) {
		return NULL;
	}
	operand_t *operand = NULL;
	if (ce_insert_line(g_code_window.program,
	                   cp_count(g_code_window.program), label) != CE_OK) {
		goto rollback;
	}
	update_label_instructions();
	update_jump_instructions();
	cw_player_holding_instruction(label, false, true);
	operand = create_jump_operand(label);
	if (operand != NULL && try_assign_op_to_line(operand, jump)) {
		return label;
	}

rollback:
	if (operand != NULL) {
		cl_destroy_operand(operand);
	}
	int index = cp_index_of(g_code_window.program, label);
	if (index >= 0) {
		code_line_t *removed = NULL;
		cp_result_t result = cp_remove(g_code_window.program, index, &removed);
		assert(result == CP_OK && removed == label);
	}
	cl_destroy_code_line(label);
	return NULL;
}

/* Function: cw_reset_code_exectution
*------------------------------------------------------------------------------
* Arguments:
* 	None.
*	
* Return:
*	Void.
*/
SDL_Rect cw_get_code_line_coord_at_pos(int code_line_element, int pos)
{
	int op1_ofs = cw_get_operand1_offset();
	int op2_ofs = cw_get_operand2_offset();
	int x = get_code_line_x(MOV);
	int y = get_code_line_y(0);
	SDL_Rect bwh = cl_get_code_button_size();
	SDL_Rect base = {.x = x, .y = y, .w = bwh.w, .h = bwh.h};
	int list_size = cp_count(g_code_window.program);
	int x_offset = code_line_element == CW_OP1 ? op1_ofs :
		code_line_element == CW_OP2 ? op2_ofs : 0;
	return cly_code_line_rect(
		base, x_offset, code_line_spacing(), pos, list_size);
}

static int get_code_line_y(int position)
{
	int list_size = cp_count(g_code_window.program);
	int border_offset = um_padding_vertical_with_border() +
	                    um_padding_vertical();
	int base_y = g_text_box.y + g_text_box.h + border_offset;
	return cly_code_line_y(
		base_y, code_line_spacing(), position, list_size);
}

void cw_reset_code_execution(void)
{
	release_active_jump_replacement_claims();
	cf_reset_execution(g_code_window.program);
}

int cw_get_instruction_y_coord(int instruction_position)
{
	assert(instruction_position >= 0 &&
	       instruction_position < cp_count(g_code_window.program));
	code_line_t *line = cp_get(g_code_window.program, instruction_position);
	return line->ins->b->r.y;
}

bool cw_chk_click_code_op2(int code_line_pos)
{
	int line_position = code_line_pos - 1;
	code_line_t *line = cp_get(g_code_window.program, line_position);
	return line != NULL && line->op2 != NULL &&
		bt_chk_rel_btn(line->op2->b, NULL);
}

static code_line_t *create_label_code_line(void)
{
	char *text = cl_get_instruction_text(LABEL);
	texture_t *texture = dw_create_text_tex(text, C_WHITE);
	SDL_Rect rect = cl_get_code_button_size();
	rect.x = ms_get_mouse_x() - rect.w / 2;
	rect.y = ms_get_mouse_y() - rect.h / 2;
	btn_t *button = bt_create_btn(rect, texture);
	if (button == NULL) {
		return NULL;
	}
	instruction_t *instruction = cl_create_instruction(LABEL, button);
	return cl_create_code_line(instruction);
}
