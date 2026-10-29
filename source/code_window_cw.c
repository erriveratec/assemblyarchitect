#include "code_window_cw.h"
#include "code_window_cw_internal.h"
#include "aux.h"

#include <SDL.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "instruction_layout_il.h"
#include "list.h"
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
static Uint32 CODE_BOX_W = 350;
static Uint32 CODE_BOX_Y = 50;
static const int CODE_COMMA_OFFSET = 15;
static const int MISSING_OPERAND_WIDTH = 3;

static List *code_list = NULL;


static SDL_Rect g_code_box;
static SDL_Rect g_scroll_box;
static SDL_Rect g_text_box;
static bool g_challenge_highlight;
static bool g_code_box_highlight;
static float g_challenge_highlight_value;
static float g_code_box_highlight_value;
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

/* Snapshot claims preserve prior state; they cannot arbitrate concurrent writers to btn_t. */
static button_animation_claim_t g_active_jump_claims[JUMP_HIGHLIGHT_COMPONENT_COUNT];

tx_styled_text_t *g_challenge_text;
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

texture_t *g_stage_name;
texture_t *g_comma_tex;
texture_t **g_numbers;

void display_line_number(void);
int get_code_line_position(int y);
ListNode *get_list_node_by_value(code_line_t *line);
static bool in_code_window(void);
static bool chk_sel_line_in_pos(code_line_t *line);
static void set_text_box(int x, int y, int w, int h);
static void display_player_code(void);
static void add_code_line(code_line_t *line);
static void add_code_line_last_pos(code_line_t *line);
static void set_code_box_member(int value, int member);
static void set_text_box_member(int value, int member);
static void code_box_height_adjust(void);
static void adjust_code_box_position(void);
static int get_label_operand_value(code_line_t *line);
List *get_code_list(void);
static operand_t *create_label_operand(code_line_t *line);
static operand_t *create_saved_label_operand(int op1_id);
static int label_counter_up_to_index(int index);
static void update_label_instructions(void);
static void update_jump_instructions(void);
static operand_t *create_updated_jump_operand(code_line_t *jmp_addr);
static operand_t *create_saved_jump_operand(int op1_id);
static SDL_Rect get_scroll_box(void);
static void reset_button_animation(btn_t *button);

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

static bool button_has_active_jump_claim(const btn_t *button)
{
	if (button == NULL) {
		return false;
	}
	for (int index = 0; index < JUMP_HIGHLIGHT_COMPONENT_COUNT; index++) {
		if (g_active_jump_claims[index].active &&
		    g_active_jump_claims[index].button == button) {
			return true;
		}
	}
	return false;
}

static bool get_active_jump_target_label_group(
	const code_line_t *line,
	jump_target_label_buttons_t *buttons)
{
	if (buttons == NULL) {
		return false;
	}
	*buttons = (jump_target_label_buttons_t){0};
	code_line_t *pending_line = cw_get_code_line_pending_operand();
	if (!cw_is_active_jump_operand_replacement(pending_line, pending_line) ||
	    pending_line->op1 == NULL || pending_line->op1->b == NULL ||
	    pending_line->op1->jptr != line) {
		return false;
	}
	jump_target_label_buttons_t target_buttons =
		get_jump_target_label_buttons(pending_line);
	button_animation_claim_t *instruction_claim =
		&g_active_jump_claims[JUMP_HIGHLIGHT_LABEL_INSTRUCTION];
	button_animation_claim_t *operand_claim =
		&g_active_jump_claims[JUMP_HIGHLIGHT_LABEL_OPERAND];
	if (target_buttons.instruction_button != NULL &&
	    instruction_claim->active &&
	    instruction_claim->button == target_buttons.instruction_button) {
		buttons->instruction_button = target_buttons.instruction_button;
	}
	if (target_buttons.operand_button != NULL &&
	    operand_claim->active &&
	    operand_claim->button == target_buttons.operand_button) {
		buttons->operand_button = target_buttons.operand_button;
	}
	return buttons->instruction_button != NULL || buttons->operand_button != NULL;
}

int cw_get_code_line_spacing(void)
{
	return cl_get_code_button_size().h + um_padding_vertical();
}

static int cw_get_code_text_width(void)
{
	return cw_get_stage_code_box().w - 2 * um_padding_horizontal_with_border();
}

static int cw_get_operand1_offset(void)
{
	return cl_get_code_button_size().w;
}

static int cw_get_operand2_offset(void)
{
	return 2 * cl_get_code_button_size().w + um_padding_horizontal();
}

static int cw_get_comma_offset(void)
{
	return cw_get_operand2_offset() - dm_scale_to_res(CODE_COMMA_OFFSET);
}

static int cw_get_missing_operand_width(void)
{
	return dm_scale_to_res(MISSING_OPERAND_WIDTH);
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
	SDL_Rect b;
	
	b.x = ib.x + ib.w;
	b.y = dm_scale_to_res(CODE_BOX_Y);
	b.w = dm_scale_to_res(CODE_BOX_W);
	b.h = sb.y - 2*b.y;
	return b;
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
	g_comma_tex = dw_create_text_tex(",", C_WHITE);
	g_numbers = malloc(sizeof(texture_t*) * (MAX_CODE_LINES + 1));
	for (int i = 0; i <= MAX_CODE_LINES; i++){
		char *number = ax_number_to_string_two_digits(i);
		g_numbers[i] = dw_create_text_tex(number, C_GREY);
		free(number);
	}
}

/* Function: cw_operate_jump_instruction
 * -----------------------------------------------------------------------------
 * Changes the status of the code lines between the jump and the destiny label,
 * if the destiny lable is previous to the jump instruction, the status is 
 * change to COMPLETE, if the destiny label is after the jump instruction.
 * all the instructions until that label are changed to EXECUTED
 * 
 * Arguments:
 *  void.
 *
 * Return:
 * 	void.
 */
void cw_operate_jump_instruction(code_line_t *line)
{
	assert(cl_is_ins_jmp_type(line->ins->id) == true
	&& "Invalid instruction for jump");

	List *code = get_code_list();
	check_mem(code);
	
	int jmp_pos = cw_get_code_line_pos_by_ptr(line);
	int dst_pos = cw_get_code_line_pos_by_ptr(line->op1->jptr);

	int i = 0;
	code_line_t *c;
	if (dst_pos < jmp_pos){
		LIST_FOREACH(code, first, next, cur){ 
			c = cur->value;
			if (i >= dst_pos && i <= jmp_pos){
				c->state = COMPLETE;
			}
			i++;
		}
	} else if (dst_pos > jmp_pos){
		LIST_FOREACH(code, first, next, cur){ 
			c = cur->value;
			if (i < dst_pos && i >= jmp_pos){
				c->state = EXECUTED;
			}
			i++;
		}	
	}
error:
	return;
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
	List *code = get_code_list();
	check_mem(code);

	int op1_ofs = cw_get_operand1_offset();
	code_line_t *c;
	LIST_FOREACH(code, first, next, cur){ 
		c = cur->value;
		if (cl_is_ins_jmp_type(c->ins->id) == true){
			assert(c->op1 != NULL && "op1 should't be NULL");
			code_line_t  *jmp_addr = cw_get_code_line_at_pos(c->op1->id);
			cl_destroy_operand(c->op1);	
			assert(cw_check_if_in_code_list(jmp_addr) == true && "Instruction"
				   "not present in code list");
			c->op1 = create_updated_jump_operand(jmp_addr);
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
	char *op_text = malloc(sizeof(char)*(strlen(line_text)+1));
	strcpy(op_text, line_text);
	texture_t *t = dw_create_text_tex(op_text, C_WHITE);

	SDL_Rect r = {.x = 0, .y = 0, .w = 2*cb.w, .h = cb.h};
	b->b = bt_create_btn(r, t);

	b->id = op1_id;
	free(line_text);
	free(op_text);
	return b;
}

/* Function: create_updated_jump_operand
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
static operand_t *create_updated_jump_operand(code_line_t *jmp_addr)
{
	assert(cw_check_if_in_code_list(jmp_addr) == true && "The code line is not"
	"in the list and is probably destroyed");

   	operand_t *op = NULL;
	op = malloc(sizeof(operand_t));

	int addr_label_id = jmp_addr->op1->id;
	char *line_text = ax_number_to_string_two_digits(addr_label_id);
	char *op_text = malloc(sizeof(char)*(strlen(label_text) + 
						   strlen(ax_char_space)+ strlen(line_text)));
	
	strcpy(op_text, label_text);
	strcat(op_text, ax_char_space);
	strcat(op_text, line_text);
	texture_t *t = dw_create_text_tex(op_text, C_WHITE);

	SDL_Rect cb = cl_get_code_button_size();
	SDL_Rect r = {.x = 0, .y = 0, .w = 2*cb.w, .h = cb.h};
	op->b = bt_create_btn(r, t);
	op->id = cw_get_code_line_pos_by_ptr(jmp_addr);
	op->jptr = jmp_addr;

	free(line_text);
	free(op_text);

	return op;
}

/* Function: update_jump_instructions
 * -----------------------------------------------------------------------------
 * Traverses the whole code list and updates all the jump instructions
 * to match the labels correctly
 * 
 * Arguments:
 *  void.
 *
 * Return:
 * 	void.
 */
static void update_jump_instructions(void)
{
	release_active_jump_replacement_claims();
	List *code = get_code_list();
	check_mem(code);

	int op1_ofs = cw_get_operand1_offset();
	code_line_t *c;
	LIST_FOREACH(code, first, next, cur){ 
		c = cur->value;
		if (cl_is_ins_jmp_type(c->ins->id) == true){
			if (c->op1 != NULL){
				code_line_t *jmp_addr = c->op1->jptr;
				cl_destroy_operand(c->op1);	
				if (cw_check_if_in_code_list(jmp_addr) == true){
					c->op1 = create_updated_jump_operand(jmp_addr);
					c->op1->b->r.x = c->ins->b->r.x + op1_ofs;
					c->op1->b->r.y = c->ins->b->r.y;
				} else {
					c->op1 = NULL;
					c->state = MISSING_OP1;
				}
			}
		}
	}
error:
	return;
}

/* Function: cw_create_jmp_op
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
operand_t *cw_create_jmp_op(code_line_t *addr)
{
   	operand_t *op = NULL;

	op = malloc(sizeof(operand_t));

	//code_line_t *addr = get_clicked_label_code_line();
	check_mem(addr);

	int addr_label_id = addr->op1->id;
	char *line_text = ax_number_to_string_two_digits(addr_label_id);
	char *op_text = malloc(sizeof(char)*(strlen(label_text) + 
						   strlen(ax_char_space)+ strlen(line_text)));
	
	strcpy(op_text, label_text);
	strcat(op_text, ax_char_space);
	strcat(op_text, line_text);
	texture_t *t = dw_create_text_tex(op_text, C_WHITE);

	SDL_Rect cb = cl_get_code_button_size();
	SDL_Rect r = {.x = 0, .y = 0, .w = 2*cb.w, .h = cb.h};
	op->b = bt_create_btn(r, t);
	op->id = cw_get_code_line_pos_by_ptr(addr);
	op->jptr = addr;

	free(line_text);
	free(op_text);

error:
	return op;
} 





/* Function: cw_ms_rel_in_label
*------------------------------------------------------------------------------
* This function verifies if a mouse click was released on a label instruction
* present in the code
*
* Arguments:
* 	Void.
*	
* Return:
*	boolean with the result if a mouse click was performed
*
*/
bool cw_ms_rel_in_label(void)
{
	bool selected = false;
	List *code = get_code_list();
	check_mem(code);

	code_line_t *c;
	LIST_FOREACH(code, first, next, cur){ 
		c = cur->value;
		if (c->ins->id == LABEL){
			if (bt_chk_rel_btn(c->ins->b, NULL) == true ||
				bt_chk_rel_btn(c->op1->b, NULL) == true){
				selected = true;
				break;
			}
		}
	}
error:
	return selected;
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
	List *code = get_code_list();
	check_mem(code);
	
	int op1_ofs = cw_get_operand1_offset();
	code_line_t *c;
	LIST_FOREACH(code, first, next, cur){ 
		c = cur->value;
		if (c->ins->id == LABEL){
			if (c->op1 != NULL){
				cl_destroy_operand(c->op1);	
				c->op1 = create_label_operand(c);
				c->op1->b->r.x = c->ins->b->r.x + op1_ofs;
				c->op1->b->r.y = c->ins->b->r.y;
			}

		}
	}
error:
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
	char *op_text = malloc(sizeof(char)*(strlen(line_text)+1));
	strcpy(op_text, line_text);
	strcat(op_text, ax_char_colon);
	texture_t *t = dw_create_text_tex(op_text, C_WHITE);

	SDL_Rect r = cl_get_code_button_size();
	b->b = bt_create_btn(r, t);

	b->id = op1_id;

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
	char *op_text = malloc(sizeof(char)*(strlen(line_text)+1));
	strcpy(op_text, line_text);
	strcat(op_text, ":");
	texture_t *t = dw_create_text_tex(op_text, C_WHITE);

	SDL_Rect r = cl_get_code_button_size();
	b->b = bt_create_btn(r, t);

	b->id = label;

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
int cw_get_code_line_pos_by_ptr(code_line_t *line)
{
	List *code = get_code_list();
	assert(NULL != code && "The code pointer is NULL");
	assert(NULL != line && "The line pointer is NULL");

	bool in_code_list = cw_check_if_in_code_list(line);
	int line_position = 0;

	if (in_code_list == false){
		line_position = NO_VALUE;
	} else {
		LIST_FOREACH(code, first, next, cur){
			if (cur->value == line){
				break;
			}
			line_position++;
		} 
	}
	return line_position;
}

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
	int list_size = cw_get_code_list_size();
	assert(index < list_size && "index values is out of reach");

	int label_counter = 0;
	List *code = get_code_list();
	check_mem(code);
	int i = 0;
	code_line_t *c;
	
	LIST_FOREACH(code, first, next, cur){ 
		c = cur->value;
			if (c->ins->id == LABEL){
				label_counter++;	
			}
		if (i == index){
			break;
		}
		i++;
	}
error:
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
	List *code = get_code_list();
	check_mem(code);
	check_mem(line);

	int pos = cw_get_code_line_pos_by_ptr(line);
	label = pos;

	bool in_code_list = cw_check_if_in_code_list(line);

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
code_line_t *cw_clone_rclicked_line(code_line_t *line)
{
	texture_t *t = cl_create_instruction_texture(line->ins->id);	
	btn_t *b = bt_create_btn(line->ins->b->r, t);

	instruction_t *i = cl_create_instruction(line->ins->id, b);
	
	code_line_t *new = cl_create_code_line(i);

	int operand_quantity = cl_get_instruction_operand_quantity(line->ins->id);
	
	if (operand_quantity == ONE_OPERAND || operand_quantity == TWO_OPERANDS){
		operand_t *op1;
		if (cl_is_ins_jmp_type(line->ins->id) == true){
		} else if (line->op1->id > REG_MIN && line->op1->id < REG_MAX){
			op1 = rg_create_register_operand_by_id(line->op1->id);
			cw_assign_op_to_line(op1, new);
		} else if (line->op1->id > BUF_MIN && line->op1->id < BUF_MAX){
			op1 = bf_create_buffer_operand_by_id(line->op1->id);
			cw_assign_op_to_line(op1, new);
		} else {
			printf("Error: no operand 1 determined\n");
		}
	}
	if (operand_quantity == TWO_OPERANDS){
		operand_t *op2;
		if (line->op2->id > REG_MIN && line->op2->id < REG_MAX){
			op2 = rg_create_register_operand_by_id(line->op2->id);
			cw_assign_op_to_line(op2, new);
		} else if (line->op2->id > BUF_MIN && line->op2->id < BUF_MAX){
			op2 = bf_create_buffer_operand_by_id(line->op2->id);
			cw_assign_op_to_line(op2, new);
		} else if (line->op2->id > IMM_MIN && line->op2->id < IMM_MAX){
			op2 = im_create_imm_op_by_id(line->op2->id);
			cw_assign_op_to_line(op2, new);
		} else {
			printf("Error: no operand 2 determined\n");
		}
	}
	return new;
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
	
	int list_size = cw_get_code_list_size();
	ins_text = cl_get_instruction_text(ins_id);
	texture_t *instruction_tex = dw_create_text_tex(
						  		  ins_text, C_WHITE);
	
	int x = cw_get_code_line_x(ins_id);
	int y = cw_get_code_line_y(0);

	for (int i = 0; i <= list_size; i++){
		y += cw_get_code_line_spacing();
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

	List *code = get_code_list();
	List_push(code, new_line);
	
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
code_line_t *cw_get_code_line_at_pos(int pos)
{
	int code_size = cw_get_code_list_size();
	assert(pos >= 0 && pos < code_size && "Incorrect position value");

	List *code = get_code_list();
	code_line_t *c;
	
	int i = 0;
	LIST_FOREACH(code, first, next, cur){ 
		c = cur->value;
		if (i == pos){
			break;
		}
		i++;
	}
	return c;
}

/* Function: get_instruction_operand
 *------------------------------------------------------------------------------
 * Arguments:
 * instruction_pos: position of the instruction in the list.
 * operand_pos: the operand position.
 *
 * Return:
 *	id of the operand at given position for an instruction.
 */
int cw_get_instruction_operand(int position, int operand_pos)
{
	assert(position >= 0 && "Invalid instruction position");
	assert(position < cw_get_code_list_size() && "The instruction position is"
		   "invalid");
	assert(operand_pos >= ZERO_OPERANDS && 
		   operand_pos <= TWO_OPERANDS && "The operand position is invalid");

	int operand = NO_OPERAND;
	List *code = get_code_list();
	code_line_t *c;
	
	int i = 0;
	LIST_FOREACH(code, first, next, cur){ 
		c = cur->value;
		
		if (i == position){
			break;
		}
		i++;
	}
	
	int operand_quantity = cl_get_instruction_operand_quantity(c->ins->id);
	
	if (operand_quantity == ZERO_OPERANDS){
		operand = NO_OPERAND;
	} else if ((operand_quantity == ONE_OPERAND || operand_quantity == TWO_OPERANDS)
	           && operand_pos == FIRST_OP){
		operand = c->op1 == NULL ? NO_OPERAND : c->op1->id;
	} else if (operand_quantity == TWO_OPERANDS && operand_pos == SECOND_OP){
		operand = c->op2 == NULL ? NO_OPERAND : c->op2->id;
	}
	return operand;
}

/* Function: cw_get_instruction_at_code_pos
 *------------------------------------------------------------------------------
 * Arguments:
 *	int: the position of the instruction that wants to be recovered.
 *
 * Return:
 *	id of the instruction at a given position.
 */
int cw_get_instruction_at_code_pos(int position)
{
	assert(position >= 0 && "Invalid instruction position");
	assert(position < cw_get_code_list_size() && "The instruction position is"
		   "invalid");

	int i = 0;

	List *code = get_code_list();
	code_line_t *c;
	
	LIST_FOREACH(code, first, next, cur){ 
		c = cur->value;
		
		if (i == position){
			break;
		}
		i++;
	}
	return c->ins->id;
}

/* Function: cw_get_code_list_size
 *------------------------------------------------------------------------------
 * Arguments:
 *	None.
 *
 * Return:
 *	Void.
 */
int cw_get_code_list_size(void)
{
	assert(code_list != NULL && "The code list is NULL");
	return List_count(code_list);
}

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
	assert(code_list == NULL && "The register list is not NULL");
	code_list = List_create();
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
List *get_code_list(void)
{
	return code_list;
}

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
	List *code = get_code_list();
	assert(NULL != code && "The code pointer is NULL");
	
	code_line_t *clicked = NULL;
	LIST_FOREACH(code, first, next, cur){ 
		
		code_line_t *c = cur->value;

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
	List *code = get_code_list();
	assert(NULL != code && "The code pointer is NULL");
	
	code_line_t *clicked = NULL;
	LIST_FOREACH(code, first, next, cur){ 
		
		code_line_t *c = cur->value;

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
	List *code = get_code_list();
	bool clicked = false;

	if (sb_chk_rst_esc_menu_active() == false){
		LIST_FOREACH(code, first, next, cur){ 
			code_line_t *c = cur->value;
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
	List *code = get_code_list();
	bool clicked = false;
	
	if (sb_chk_rst_esc_menu_active() == false){
		if (ms_click_inside_rect(g_code_box) == true){
			LIST_FOREACH(code, first, next, cur){ 
				code_line_t *c = cur->value;
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
	dw_free_texture(g_stage_name);
	tx_free_styled_text(g_challenge_text);
	g_challenge_text = NULL;
	List *code = get_code_list();
	LIST_FOREACH(code, first, next, cur){
		code_line_t *line = cur->value;	
		cl_destroy_code_line(line);
	}
	List_destroy(code_list);
	code_list = NULL;
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

	if (mouse_x > g_scroll_box.x && mouse_x < (g_scroll_box.x + g_scroll_box.w) &&
		mouse_y > g_scroll_box.y && mouse_y < (g_scroll_box.y + g_scroll_box.h)) {
		return true;
	} else {
		return false;
	}
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


/* Function: cw_get_code_line_x
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
int cw_get_code_line_x(int instruction_id)
{
	int x;  
	int number_ofs = um_padding_horizontal_with_border();
	SDL_Rect sb = cw_get_stage_code_box();
	int number_h = ty_code_height();
	int number_w = ax_get_texture_w_fit_h(number_h, g_numbers[0]);
	if (instruction_id == LABEL){
		x = sb.x;
	} else {
		x = sb.x + number_ofs + number_w; 	
	}
	return x;
}


/*
static void code_box_height_adjust()
{
	List *code = get_code_list();
	assert(NULL != code && "The code list cannot be NULL");

	int list_size = List_count(code);

	SDL_Rect cbut = cl_get_code_button_size();
	SDL_Rect codbox = get_code_box();
	SDL_Rect ogcodbox = cw_get_stage_code_box();
	if (CODE_LINES_SIZE < list_size){
		
		int increase_size = (list_size - CODE_LINES_SIZE)*cbut.h;
		int new_bottom_border = codbox.y + ogcodbox.h + increase_size;
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
}*/

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
	List *code = get_code_list();
	assert(NULL != code && "The code list cannot be NULL");

	int list_size = List_count(code);

	SDL_Rect codbox = get_scroll_box();
	SDL_Rect ogcodbox = cw_get_stage_code_box();
	
	int y = cw_get_code_line_y(list_size - 1);
	if ( y > (ogcodbox.h + ogcodbox.y)){
		
		int inc = y + cw_get_code_line_spacing() - (codbox.h + codbox.y);
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
	static int y_pending_scroll = 0;
	
	y_pending_scroll += 3*ms_get_mouse_scroll_y();
	SDL_Rect cb = cw_get_stage_code_box();
	SDL_Rect codbox = get_scroll_box();

	int cur_y = codbox.y;
	int max_upper_border_y = cb.y;
	int min_lower_border_y = cb.y + cb.h;

	if (cur_y >= max_upper_border_y && y_pending_scroll > 0){
		y_pending_scroll = 0;
	} 
	if ((cur_y + codbox.h) <= min_lower_border_y && y_pending_scroll < 0){
		y_pending_scroll = 0;
	}

	int mdelta = ax_get_cw_move_delta();
	int delta = ax_get_movement_delta(0, y_pending_scroll, mdelta);	

	if (y_pending_scroll != 0){
		SDL_Rect tb = cw_get_text_box_rect();
		int text_box_y = tb.y;
		
		if (y_pending_scroll < 0){
			cur_y -= delta;
			set_code_box_member(cur_y, MEMBER_Y);
			text_box_y -= delta;
			set_text_box_member(text_box_y, MEMBER_Y);
			y_pending_scroll += delta;
		} else if (y_pending_scroll > 0){
			cur_y += delta;
			set_code_box_member(cur_y, MEMBER_Y);
			text_box_y += delta;
			set_text_box_member(text_box_y, MEMBER_Y);
			y_pending_scroll -= delta;
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

	// Adjust the height of the code box
	code_box_height_adjust();

	if (in_code_window() == true){
		adjust_code_box_position();
	}
	float code_box_frame_growth = 0.0f;
	float code_box_top = g_code_box.y;
	float code_box_bottom = g_code_box.y + g_code_box.h;
	float anim_limit = cw_get_challenge_highlight_limit();
	g_code_box_highlight_value = g_code_box_highlight ?
		ha_get_value(anim_limit) : 0.0f;
	if (g_code_box_highlight) {
		code_box_frame_growth =
		    g_code_box_highlight_value * CODE_BOX_HIGHLIGHT_GROWTH_MULTIPLIER;
		code_box_top -= code_box_frame_growth / 2.0f;
		code_box_bottom += code_box_frame_growth / 2.0f;
	}
	
	
	if (g_code_box_highlight) {
		SDL_FRect outer = {
			.x = g_code_box.x,
			.y = g_code_box.y,
			.w = g_code_box.w,
			.h = g_code_box.h
		};
		int border = um_border_width();
		SDL_Rect inner_rect = {
			.x = g_code_box.x + border,
			.y = g_code_box.y + border,
			.w = g_code_box.w - 2 * border,
			.h = g_code_box.h - 2 * border
		};
		SDL_FRect inner = {
			.x = inner_rect.x,
			.y = inner_rect.y,
			.w = inner_rect.w,
			.h = inner_rect.h
		};
		dw_draw_filled_rectangle_f(
		    dw_grow_rect_height(outer, code_box_frame_growth),
		                          C_GREY, C_GREY);
		dw_draw_filled_rectangle_f(
		    dw_grow_rect_height(inner, code_box_frame_growth),
		                          C_BLACK, C_BLACK);
	} else {
		dw_draw_thick_rect(g_code_box, um_border_width(), C_GREY);
	}
//	dw_draw_thick_rect(g_scroll_box, um_border_width(), C_GREY);

	display_player_code();
	display_line_number();

	int h = ty_message_height();
	if (g_challenge_text != NULL && !g_challenge_highlight) {
		tx_draw_styled_text(g_text_box, h, g_challenge_text, 0);
		dw_draw_rectangle(g_text_box, C_GREY);
	} else if (g_challenge_text != NULL) {
		int line_count = g_challenge_text->row_count;
		g_challenge_highlight_value = ha_get_value(anim_limit);
		float challenge_pulse = g_challenge_highlight_value;

		SDL_FRect base_challenge_box = {
			.x = g_text_box.x,
			.y = g_text_box.y,
			.w = g_text_box.w,
			.h = g_text_box.h
		};
		SDL_FRect challenge_box = dw_grow_rect_height(
		    base_challenge_box, challenge_pulse * line_count);
		tx_draw_styled_text_pulsed(g_text_box, h, challenge_pulse,
		                           g_challenge_text);
		dw_draw_rectangle_f(challenge_box, C_GREY);
	}
	if (!g_challenge_highlight) {
		g_challenge_highlight_value = 0.0f;
	}
	
	
	// Text of the level

	int border_ofs = um_padding_horizontal_with_border();
	int text_h = ty_emphasis_height();
	SDL_Rect b = {.x = g_scroll_box.x + border_ofs, 
				  .y = g_scroll_box.y + border_ofs,
				  .h = text_h};
	dw_draw_texture_fit_h(b, g_stage_name);

	SDL_Rect upbox = {.x = g_code_box.x,
					  .y = 0,
					  .w = g_code_box.w,
					  .h = code_box_top > 0.0f ? (int)code_box_top : 0};
	dw_draw_filled_rectangle(upbox, C_BLACK, C_BLACK);
	SDL_Rect downbox = {.x = g_code_box.x,
					  .y = (int)ceilf(code_box_bottom),
			 			.w = g_code_box.w,
		     			.h = g_code_box.h};
	dw_draw_filled_rectangle(downbox, C_BLACK, C_BLACK);


}

static void draw_code_button(btn_t *button, bool blocked, bool hover)
{
	if (button == NULL) {
		return;
	}
	if (button_has_active_jump_claim(button)) {
		bt_draw_btn_with_highlight_progress(button, blocked, hover,
		                                   ha_get_progress());
	} else {
		bt_draw_btn(button, blocked, hover);
	}
}

static void draw_operand_button(code_line_t *line, operand_t *operand,
	                            int changing_state, bool blocked)
{
	code_line_t *pending_line = cw_get_code_line_pending_operand();
	if (operand == NULL || operand->b == NULL ||
	    !cw_should_draw_replacement_operand(
		    line, changing_state, pending_line)) {
		return;
	}
	draw_code_button(operand->b, blocked, false);
}

static bool button_is_in_label_group(const btn_t *button,
	                                 jump_target_label_buttons_t buttons)
{
	return button != NULL &&
		(button == buttons.instruction_button || button == buttons.operand_button);
}

static void draw_code_line_buttons(code_line_t *line, bool blocked)
{
	jump_target_label_buttons_t label_buttons;
	bool grouped_label = get_active_jump_target_label_group(line, &label_buttons);
	if (grouped_label) {
		btn_t *reference_button =
			g_active_jump_claims[JUMP_HIGHLIGHT_OPERAND].button;
		float reference_scale;
		bt_group_draw_item_t items[] = {
			{label_buttons.instruction_button, blocked, false},
			{label_buttons.operand_button, blocked, false}
		};
		if (bt_calculate_button_highlight_scale(
			reference_button, ha_get_progress(), &reference_scale)) {
			if (!bt_draw_btn_group_with_highlight_scale(
				items, sizeof(items) / sizeof(items[0]), reference_scale,
				BT_HIGHLIGHT_ANCHOR_LEFT)) {
				draw_code_button(label_buttons.instruction_button, blocked, false);
				draw_code_button(label_buttons.operand_button, blocked, false);
			}
		} else {
			draw_code_button(label_buttons.instruction_button, blocked, false);
			draw_code_button(label_buttons.operand_button, blocked, false);
		}
	}

	if (!grouped_label ||
	    !button_is_in_label_group(line->ins != NULL ? line->ins->b : NULL,
	                              label_buttons)) {
		draw_code_button(line->ins != NULL ? line->ins->b : NULL, blocked, false);
	}
	if (line->op1 != NULL &&
	    (!grouped_label || !button_is_in_label_group(line->op1->b, label_buttons))) {
		draw_operand_button(line, line->op1, CHANGING_OP1, blocked);
	}
	if (line->op2 != NULL) {
		draw_operand_button(line, line->op2, CHANGING_OP2, blocked);
	}
}

/* Function: display_player_code
 * -----------------------------------------------------------------------------
 * This function displays the code that is contained in the list of code
 *
 * Arguments:
 * 	code_list: a list with the code that will be executed
 *
 * Return:
 *	void
 */
static void display_player_code(void)
{
	List *code = get_code_list();
	assert(code != NULL && "The code list can't be NULL");
	
	int list_size = List_count(code);
	
	if (list_size == 0){
		return;
	}
	SDL_Rect cb = cl_get_code_button_size();
	LIST_FOREACH(code, first, next, cur){
		code_line_t *line = cur->value;	
		draw_code_line_buttons(line, sb_chk_rst_esc_menu_active());
	
		int comma = cl_get_instruction_operand_quantity(line->ins->id);

		if (comma == TWO_OPERANDS 
			&& (MISSING_BOTH != line->state && MISSING_OP1 != line->state)){
		
			int comma_ofs = cw_get_comma_offset();
			SDL_Rect r = {.x = line->ins->b->r.x + comma_ofs, 
						  .y = line->ins->b->r.y, .h = cb.h};

			dw_draw_texture_fit_h(r, g_comma_tex);
		}
	}
	return;
}


/* Function: cw_get_instruction_y_coord
 * -----------------------------------------------------------------------------
 * Returns the y coordinaten of the instruction by its position
 *
 * Arguments:
 * instruction_position: Instruction position in the code list
 *
 * Return:
 *	y_coordinate: The y coordinate of the instruction in a given position.
 */
int cw_get_instruction_y_coord(int instruction_position)
{
	int code_size = cw_get_code_list_size();
	assert(instruction_position >= 0 && instruction_position < code_size &&
		   "Incorrect instruction position value");

	List *code = get_code_list();
	code_line_t *c;
	
	int i = 0;
	LIST_FOREACH(code, first, next, cur){ 
		c = cur->value;
		if (i == instruction_position){
			return c->ins->b->r.y;
		}
		i++;
	}
	return 0;
}

/* Function: display_line_number
 * -------------------------------------
 * This function displays the number of each of the line of code according
 * to the number of instructions in the code list
 *
 * Arguments:
 * code: a list with the code developed by the player
 *
 * Return:
 *	void
 */
void display_line_number(void)
{
	List *code = get_code_list();
	assert(code != NULL && "The code list can't be NULL");
	
	int list_size = List_count(code);
	if (list_size == 0){
		return;
	}

	int number_ofs = um_padding_horizontal_with_border();
	SDL_Rect sb = cw_get_stage_code_box();
	int x = sb.x + number_ofs;
	int y = cw_get_code_line_y(0);
	int h = ty_code_height();
	
	int line_number = 1;
	for(int i = 0; i < list_size; i++){
	int instruction = cw_get_instruction_at_code_pos(i);
		if (instruction != LABEL){
			SDL_Rect r = {.x = x, .y = y, .h = h};
			dw_draw_texture_fit_h(r, g_numbers[line_number]);
			line_number++;
		}
		y += cw_get_code_line_spacing();
	}
}

/* Function: cw_get_code_line_y
 * -------------------------------------
 * Returns the y coodinate a instruction must be assigned
 *
 * Arguments:
 * 	position: position of the line
 *
 * Return:
 * 	The y coordinate the instruction must have 
 */
int cw_get_code_line_y(int pos)
{
	List *code = get_code_list();
	assert(code != NULL && "The code list can't be NULL");
	
	int list_size = List_count(code);
	int border_ofs = um_padding_vertical_with_border() + um_padding_vertical();
	SDL_Rect tb = cw_get_text_box_rect();
	int y = tb.y + tb.h + border_ofs;

	if (list_size == 0 || pos == 0){
		return y;
	}
	
	for(int i = 0; i < list_size; i++){
		y += cw_get_code_line_spacing();
		if (pos == i){
			break;
		}
	}
	return y;
}

/* Function: cw_get_code_line_coord_at_pos
 * -----------------------------------------------------------------------------
 * This function return a given element of the code line for a position
 * with the properties of the position that should be assigned, for example
 * the instruction, operand1, or operand 2 for a given position
 *
 * Arguments:
 *	element: 
 *
 * Return:
 *	Void.
 */
SDL_Rect cw_get_code_line_coord_at_pos(int code_line_element, int pos)
{
	int op1_ofs = cw_get_operand1_offset();
	int op2_ofs = cw_get_operand2_offset();
	
	int x = cw_get_code_line_x(MOV);
	int y = cw_get_code_line_y(0);
	
	SDL_Rect bwh = cl_get_code_button_size();
	SDL_Rect cl = {.x = x, .y = y, .w = bwh.w, .h = bwh.h};

	int list_size = cw_get_code_list_size();

	switch(code_line_element){
		case CW_OP1:
			cl.x += op1_ofs;
			break;
		case CW_OP2:
			cl.x += op2_ofs;	
			break;
		default:
			break;
	}

	if (list_size == 0 || pos == 0){
		return cl;
	}
	

	for(int i = 0; i < list_size; i++){
		cl.y += cw_get_code_line_spacing();
		if (pos == i){
			break;
		}
	}
	return cl;
}

/* Function: cw_sort_code
 * -----------------------------------------------------------------------------
 * This function iterates throught the list of code done by the player to 
 * verify which instructions are out of position. If a instruction is out
 * of position, it will accomodate it's coordinates.
 *
 * Arguments:
 *	none.
 *
 * Return:
 *	Void.
 */
void cw_sort_code(void)
{
	List *code = get_code_list();
	int list_size = List_count(code);
	
	if (0 == list_size){
		return;
	}
	
	int op1_ofs = cw_get_operand1_offset();
	int op2_ofs = cw_get_operand2_offset();
	
	int y = cw_get_code_line_y(0);

	int mdelta = ax_get_cw_move_delta();
	LIST_FOREACH(code, first, next, cur){

		code_line_t *line = cur->value;
		int x = cw_get_code_line_x(line->ins->id);
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
		y += cw_get_code_line_spacing();

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

ListNode *get_list_node_by_value(code_line_t *line)
{
	List *code = get_code_list();
	assert(NULL != code && "The list pointer is NULL");
	assert(NULL != line && "The line pointer is NULL");

	LIST_FOREACH(code, first, next, cur){
		
		if (cur->value == line){
			return cur;
		}
	}
	return NULL;
}

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

bool cw_check_if_in_code_list(code_line_t *line)
{
	List *code = get_code_list();
	assert(NULL != code && "The code pointer is NULL");
	assert(NULL != line && "The line pointer is NULL");

	if (line == NULL){
		return false;
	}

	LIST_FOREACH(code, first, next, cur){
		
		if (cur->value == line){
			return true;
		}
	}
	return false;
}


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
	List *code = get_code_list();
	int first_y = cw_get_code_line_y(0);

	int h = cw_get_code_line_spacing();
	int list_size = List_count(code);

	for(int i = 0; i < list_size; i++){
		if(y >= first_y && y < first_y + h){
			return i;
		}
		first_y += cw_get_code_line_spacing();
	}	
	return NOT_FOUND;
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
	List *code = get_code_list();
	assert(NULL != code &&  "Code list is NULL");
	assert(NULL != line &&  "The code line object is NULL");

		
	List_push(code, line);
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
	List *code = get_code_list();
	assert(NULL != code &&  "Code list is NULL");
	assert(NULL != line &&  "The code line object is NULL");

	int list_count = List_count(code);
		
	if (0 == list_count){
		List_push(code, line);
		return;
	} 
		
	code_line_t *first = code->first->value;
	int mouse_y = ms_get_mouse_y();
	int y = first->ins->b->r.y;
	
	if (mouse_y < y){
		List_unshift(code, line);
		return;
	} 

	code_line_t *last = code->last->value;
	y = last->ins->b->r.y;
	int h = last->ins->b->r.h;
	if (mouse_y >= y + h){
		List_push(code, line);
		return;
	}

	int position = get_code_line_position(mouse_y);
	if (position != NOT_FOUND){
		List_insert_at_position(code, line, position);
		return;
	} else {
		// If list comes to this place without adding code line, it's added 
		// to the last element
		List_push(code, line);
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
	List *code = get_code_list();
	assert(NULL != code && "The list of code is NULL");
	assert(NULL != line && "The evaluated line is NULL");
	
	int list_size = List_count(code);
	
	if (0 == list_size){
		return false;
	}
	
	int y = cw_get_code_line_y(0);

	int mouse_y = ms_get_mouse_y();
	bool ret = false;
	SDL_Rect cb = cl_get_code_button_size();
	int h = cb.h;
	if (mouse_y < y || mouse_y >= y + list_size*h){
		ret = false;
	} else {
		LIST_FOREACH(code, first, next, cur){
			if (cur->value == line){
				if (mouse_y >= y && mouse_y < y + h){
					ret =  true;	
				}
			}
			y += cw_get_code_line_spacing();
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
	List *code = get_code_list();

	while (cw_get_code_list_size() > 0){
		code_line_t *line = cw_get_code_line_at_pos(0);
		ListNode *node = get_list_node_by_value(line);
		cl_destroy_code_line(line);
		List_remove(code, node);
	}
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
	List *code = get_code_list();
	assert(code != NULL &&  "Code list is NULL");
	assert(line != NULL &&  "The code line object is NULL");
	cw_cancel_pending_operand_change();
	
	if (in_code_window() == true || line->ins->id == LABEL || del == false){
		if (cw_check_if_in_code_list(line) == true){
			if (chk_sel_line_in_pos(line) == false && arng == true){
				ListNode *node = get_list_node_by_value(line);
				List_remove(code, node);
			}
		} 
		if (cw_check_if_in_code_list(line) == false && arng == true){
			add_code_line(line);
			update_label_instructions();
			update_jump_instructions();
		} else if (cw_check_if_in_code_list(line) == false && arng == false){
			add_code_line_last_pos(line);
			update_label_instructions();
			update_jump_instructions();
		}
		if (line->ins->id == LABEL){
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
		if (cw_check_if_in_code_list(line) == true){
			if (cl_is_ins_jmp_type(line->ins->id) == true && line->op1 != NULL){
				code_line_t *addr = line->op1->jptr;
				ListNode *node = get_list_node_by_value(addr);
				cl_destroy_code_line(addr);
				List_remove(code, node);
			}
			ListNode *node = get_list_node_by_value(line);
			List_remove(code, node);
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
	draw_code_line_buttons(line, em_get_escape_state());
}

/* Function: cw_is_operand_1_pending
 * -----------------------------------------------------------------------------
 * This function verifies if any of the programmed lines is pending op2
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 * 	true if a line is pending operand, false if otherwise
 *
 */
bool cw_is_operand_2_pending(void)
{
	List *code = get_code_list();

	bool pending = false;
	LIST_FOREACH(code, first, next, cur){
		code_line_t *c = cur->value;
		int s = c->state;
		if (s == MISSING_BOTH || s == MISSING_OP2 || s == CHANGING_OP2){
			pending = true;
			break;
		}
	}
	return pending;
}

/* Function: cw_is_operand_1_pending
 * -----------------------------------------------------------------------------
 * This function verifies if any of the programmed lines is pending op1
 *
 * Arguments:
 * 	Void.
 *	
 * Return:
 * 	true if a line is pending operand, false if otherwise
 *
 */
bool cw_is_operand_1_pending(void)
{
	List *code = get_code_list();

	bool pending = false;
	LIST_FOREACH(code, first, next, cur){
		code_line_t *c = cur->value;
		int s = c->state;
		if (s == MISSING_BOTH || s == MISSING_OP1 || s == CHANGING_OP1){
			pending = true;
			break;
		}
	}
	return pending;
}

/* Function: cw_is_operand_pending
 * -----------------------------------------------------------------------------
 * This function verifies if any of the programmed lines is pending an operand
 *
 * Arguments:
 * 	code: the list of code programmed by the player
 *	
 * Return:
 * 	true if a line is pending operand, false if otherwise
 *
 */
bool cw_is_operand_pending(void)
{
	List *code = get_code_list();

	bool pending = false;
	LIST_FOREACH(code, first, next, cur){
		code_line_t *c = cur->value;
		int s = c->state;
		if (s == MISSING_BOTH || s == MISSING_OP1 || s == MISSING_OP2 ||
			s == CHANGING_OP1 || s == CHANGING_OP2){
			pending = true;
			break;
		}
	}
	return pending;
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
	List *code = get_code_list();
	if (code == NULL) {
		return NULL;
	}

	LIST_FOREACH(code, first, next, cur){
		code_line_t *c = cur->value;
		int s = c->state;
		if (CHANGING_OP1 == s || CHANGING_OP2 == s){
			return c;
		}
	}
	for (ListNode *current = code->first; current != NULL; current = current->next){
		code_line_t *c = current->value;
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
	List *code = get_code_list();
	int list_size = List_count(code);
	if (0 == list_size){
		return true;
	}
	
	int y = cw_get_code_line_y(0);

	int retval = true;
	LIST_FOREACH(code, first, next, cur){

		code_line_t *value = cur->value;
		btn_t *b = value->ins->b;
		int x = cw_get_code_line_x(value->ins->id);

		if (b->r.x < x || b->r.x > x || b->r.y < y || b->r.y > y){
			retval = false;
			break;
		} 
		y += cw_get_code_line_spacing();
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
bool cw_chk_click_code_op2(int code_line_pos)
{
	List *code = get_code_list();
	assert(NULL != code && "The code pointer cannot be NULL");

	bool clicked = false;
	
	int i = 1; // code line pos starts a 1
	LIST_FOREACH(code, first, next, cur){ 
		code_line_t *c = cur->value;
		if (c->op2 != NULL && code_line_pos == i){
			if (true == bt_chk_rel_btn(c->op2->b, NULL)){
				clicked = true;
				break;
			}
		}
		i++;
	}
	return clicked;
}

/* Function: cw_chk_click_code_op
 * -----------------------------------------------------------------------------
 * This function checks if the player clicked on an operando of the instructions
 * available in the code window.
 *
 * Arguments:
 * 	code: the list of code programmed by the player
 *	
 * Return:
 * 	true if player clicked on operand. 
 *	false if otherwise.
 *
 */
bool cw_chk_click_code_op(void)
{
	List *code = get_code_list();
	assert(NULL != code && "The code pointer cannot be NULL");
	bool clicked = false;
	
	if (sb_chk_rst_esc_menu_active() == false){
		LIST_FOREACH(code, first, next, cur){ 
			code_line_t *c = cur->value;
			if (NULL != c->op1){
				if (true == bt_chk_rel_btn(c->op1->b, NULL)){
					clicked = true;
					break;
				}
			} 
			if (NULL != c->op2){
				if (true == bt_chk_rel_btn(c->op2->b, NULL)){
					clicked = true;
					break;
				}
			}
		}
	}
	return clicked;
}

static void reset_button_animation(btn_t *button)
{
	if (button == NULL){
		return;
	}
	button->animated = false;
	button->anim_dir = false;
	button->anim_state = 0;
}

/* Function: cw_change_clicked_line_status
 * -----------------------------------------------------------------------------
 * This function changes the status of a line whose operand has been clicked
 * in order to make the game selecte the clicked operand.
 *
 * Arguments:
 * 	none.
 *	
 * Return:
 *	void.
 *
 */
void cw_change_clicked_code_line_state(void)
{
	List *code = get_code_list();
	assert(NULL != code && "The code pointer cannot be NULL");
	code_line_t *selected_line = NULL;
	int selected_operand = NO_OPERAND;
	LIST_FOREACH(code, first, next, cur){
		code_line_t *c = cur->value;
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

	for (ListNode *current = code->first; current != NULL; current = current->next){
		code_line_t *c = current->value;
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
		int qty = cl_get_instruction_operand_quantity(c->ins->id);
		if (qty == TWO_OPERANDS && c->op1 == NULL && c->op2 == NULL){
			c->state = MISSING_BOTH;
		} else if (qty != ZERO_OPERANDS && c->op1 == NULL){
			c->state = MISSING_OP1;
		} else if (qty == TWO_OPERANDS && c->op2 == NULL){
			c->state = MISSING_OP2;
		} else {
			c->state = COMPLETE;
		}
	}

	btn_t *selected_button = selected_operand == OP1 ?
		selected_line->op1->b : selected_line->op2->b;
	selected_line->state = selected_operand == OP1 ? CHANGING_OP1 : CHANGING_OP2;
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
	code_line_t *line = cw_get_code_line_pending_operand();
	if (line == NULL || (line->state != CHANGING_OP1 &&
	                     line->state != CHANGING_OP2) || line->ins == NULL) {
		return;
	}

	int quantity = cl_get_instruction_operand_quantity(line->ins->id);
	bool missing_op2 = line->state == CHANGING_OP1 &&
		quantity == TWO_OPERANDS && line->op2 == NULL;
	if (quantity == TWO_OPERANDS && line->op1 == NULL && line->op2 == NULL) {
		line->state = MISSING_BOTH;
	} else if (quantity != ZERO_OPERANDS && line->op1 == NULL) {
		line->state = MISSING_OP1;
	} else if (quantity == TWO_OPERANDS && line->op2 == NULL) {
		line->state = MISSING_OP2;
	} else {
		line->state = missing_op2 ? MISSING_OP2 : COMPLETE;
	}

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
void cw_assign_op_to_line(operand_t *op, code_line_t *line)
{
	assert(NULL != op && "The operand cannot be NULL");
	assert(NULL != line && "The line cannot be NULL");
	release_active_jump_replacement_claims();

	int op1_ofs = cw_get_operand1_offset();
	int op2_ofs = cw_get_operand2_offset();

	int operand_quantity = cl_get_instruction_operand_quantity(line->ins->id);
	switch(line->state){
		case MISSING_BOTH:
			line->op1 = op;
			line->op1->b->r.x = line->ins->b->r.x + op1_ofs;
			line->op1->b->r.y = line->ins->b->r.y;
			line->state = MISSING_OP2;
			break;
		case MISSING_OP1:
			line->op1 = op;
			line->op1->b->r.x = line->ins->b->r.x + op1_ofs;
			line->op1->b->r.y = line->ins->b->r.y;
			if (operand_quantity == ONE_OPERAND){
				line->state = COMPLETE;
			} else {
				line->state = MISSING_OP2;
			}
			break;
		case MISSING_OP2:
			line->op2 = op;
			line->op2->b->r.x = line->ins->b->r.x + op2_ofs;
			line->op2->b->r.y = line->ins->b->r.y;
			line->state = COMPLETE;
			break;
		case CHANGING_OP1:
			cl_destroy_operand(line->op1);	
			line->op1 = op;
			line->op1->b->r.x = line->ins->b->r.x + op1_ofs;
			line->op1->b->r.y = line->ins->b->r.y;
			reset_button_animation(line->op1->b);
			line->state = (operand_quantity == TWO_OPERANDS &&
							line->op2 == NULL) ? MISSING_OP2 : COMPLETE;
			break;
		case CHANGING_OP2:
			cl_destroy_operand(line->op2);	
			line->op2 = op;
			line->op2->b->r.x = line->ins->b->r.x + op2_ofs;
			line->op2->b->r.y = line->ins->b->r.y;
			reset_button_animation(line->op2->b);
			line->state = COMPLETE;
			break;
		case COMPLETE:
			printf("cl_assign_operand_to line Error, there should not be an" 
					"operand assigment to a complete instruction. \n");
			break;
		default:
			printf("Error, the state of the code line is invalid\n");
			break;
	}
}

/* Function: cw_reset_code_exectution
*------------------------------------------------------------------------------
* Arguments:
* 	None.
*	
* Return:
*	Void.
*/
void cw_reset_code_execution(void)
{
	release_active_jump_replacement_claims();
	List *code = get_code_list();

   	LIST_FOREACH(code, first, next, cur){ 
	   	code_line_t *c = cur->value;
		c->state = COMPLETE;   
   }
}

/* function: cw_create_label_code_line
 * -----------------------------------------------------------------------------
 * This function is called when a jump instruction in in the code, it generates
 * the label codeline that is required as the jump destitu
 *
 * Arguments:
 * 	Void.
 *
 * Return:
 *	Void.
 */
code_line_t *cw_create_label_code_line(void)
{
	code_line_t *new = NULL;
	char *text = cl_get_instruction_text(LABEL);
	texture_t *t = dw_create_text_tex(text, C_WHITE);
	SDL_Rect r = cl_get_code_button_size();
	r.x = ms_get_mouse_x() - r.w/2;
	r.y = ms_get_mouse_y() - r.h/2;

	btn_t *b = bt_create_btn(r, t);
	check_mem(b);
	
	instruction_t *i = cl_create_instruction(LABEL, b);
	new = cl_create_code_line(i);

error:
	return new;

}
