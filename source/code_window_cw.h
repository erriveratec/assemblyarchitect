#ifndef CODE_WINDOW_CW_H
#define CODE_WINDOW_CW_H

#include <SDL.h>
#include "code_line_cl.h"
#include "migration/legacy_program_adapter.h"

#define CW_EMPTY 0
#define MAX_CODE_LINES 99

enum operand_positions{
	FIRST_OP,
	SECOND_OP
};

enum code_line_element{
	CW_INS,
	CW_OP1,
	CW_OP2,
	CW_LABEL
};

typedef enum cw_append_result {
	CW_APPEND_NOT_APPLICABLE = 0,
	CW_APPEND_COMMITTED,
	CW_APPEND_FAILED
} cw_append_result_t;

typedef bool (*cw_append_authority_fn)(
	const aa_legacy_line_snapshot_t *snapshot,
	aa_instruction_id_t *created_id,
	void *context);

typedef bool (*cw_append_prepare_fn)(size_t required_count, void *context);

typedef enum cw_existing_edit_kind {
	CW_EXISTING_EDIT_REMOVE,
	CW_EXISTING_EDIT_MOVE
} cw_existing_edit_kind_t;

typedef enum cw_existing_edit_result {
	CW_EXISTING_EDIT_NOT_APPLICABLE = 0,
	CW_EXISTING_EDIT_NO_CHANGE,
	CW_EXISTING_EDIT_COMMITTED,
	CW_EXISTING_EDIT_FAILED
} cw_existing_edit_result_t;

typedef bool (*cw_existing_edit_authority_fn)(
	cw_existing_edit_kind_t kind,
	size_t from,
	size_t to,
	const aa_legacy_line_snapshot_t *snapshot,
	void *context);

void cw_draw_code_window();
void cw_create_code_list();
void cw_set_scroll_box(SDL_Rect r);
void cw_set_code_box(SDL_Rect r);
void cw_set_challenge_text(char *text);
void cw_set_challenge_highlight(bool enabled);
void cw_set_code_box_highlight(bool enabled);
void cw_set_stage_name(char *text);
bool cw_player_holding_instruction(code_line_t *line, bool arng, bool del);
void cw_draw_held_instruction(code_line_t *line);
void cw_clear_held_instruction(void);
bool cw_refresh_label_and_jump_presentation(void);
bool cw_rebuild_domain_bindings(const aa_program_t *program);
bool cw_domain_bindings_valid(const aa_program_t *program);
bool cw_get_domain_instruction_id(const code_line_t *line,
								 aa_instruction_id_t *instruction_id);
void cw_clear_domain_bindings(void);
cw_append_result_t cw_append_new_line_authoritatively(
	code_line_t *line,
	bool arrange,
	bool delete_enabled,
	bool authority_allowed,
	cw_append_authority_fn commit_domain,
	cw_append_prepare_fn prepare_domain,
	void *context);
cw_existing_edit_result_t cw_edit_existing_line_authoritatively(
	code_line_t *line,
	bool arrange,
	bool delete_enabled,
	bool authority_allowed,
	cw_existing_edit_authority_fn commit_domain,
	void *context);
bool cw_check_if_in_code_list(code_line_t *instruction);
bool cw_chk_click_code();
bool cw_chk_rclick_code();

code_line_t *cw_get_clicked_code();
code_line_t *cw_get_rclicked_code();
code_line_t *cw_clone_rclicked_line(code_line_t *line);

bool cw_check_code_sorted();
void cw_sort_code();
bool cw_is_operand_pending();
bool cw_is_operand_1_pending();
bool cw_is_operand_2_pending();
code_line_t *cw_get_code_line_pending_operand();
bool cw_chk_click_code_op();
bool cw_chk_click_code_op2(int code_line_pos);

void cw_change_clicked_code_line_state();
int cw_get_code_list_size();
bool cw_get_legacy_program_reader(aa_legacy_program_reader_t *reader);
int cw_get_instruction_at_code_pos(int position);
int cw_get_instruction_operand(int position, int operand_number);
code_line_t *cw_get_code_line_at_pos(int pos);
void cw_reset_code_execution();
void cw_add_saved_line(char *line);
void cw_destroy_code_window_assets();
int cw_get_instruction_y_coord(int instruction_position); //review usage
int cw_get_code_line_x(int instruction_id);
int cw_get_code_line_y(int pos);
bool cw_ms_rel_in_label();
code_line_t *cw_get_released_label_code_line(void);
operand_t *cw_create_jmp_op(code_line_t *addr);
bool cw_update_saved_jump_instructions(void);
void cw_operate_jump_instruction(code_line_t *line);
SDL_Rect cw_get_text_box_rect();
float cw_get_challenge_highlight_limit();
void cw_init_code_window_texture();
SDL_Rect cw_get_code_line_coord_at_pos(int code_line_element, int pos);
int cw_get_code_line_pos_by_ptr(code_line_t *line);
code_line_t *cw_create_label_code_line();
void cw_clear_code_list();
void cw_assign_op_to_line(operand_t *op, code_line_t *line);
void cw_highlight_code_pending_operand();
SDL_Rect cw_get_stage_code_box();
int cw_get_code_line_spacing(void);
#endif
