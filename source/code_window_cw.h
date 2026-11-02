#ifndef CODE_WINDOW_CW_H
#define CODE_WINDOW_CW_H

#include <SDL.h>
#include "code_line_cl.h"
#include "code_program_cp.h"

#define CW_EMPTY 0
#define MAX_CODE_LINES 99

/* Temporary compatibility surface; migrate callers in Stage 10 and remove
 * only unused wrappers during Stage 11. */

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

void cw_draw_code_window(void);
void cw_create_code_list(void);
void cw_set_scroll_box(SDL_Rect r);
void cw_set_code_box(SDL_Rect r);
void cw_set_challenge_text(const char *text);
void cw_set_challenge_highlight(bool enabled);
void cw_set_code_box_highlight(bool enabled);
void cw_set_stage_name(char *text);
void cw_player_holding_instruction(code_line_t *line, bool arng, bool del);
bool cw_chk_click_code(void);
bool cw_chk_rclick_code(void);

code_line_t *cw_get_clicked_code(void);
code_line_t *cw_get_rclicked_code(void);
code_line_t *cw_clone_rclicked_line(code_line_t *line);

bool cw_check_code_sorted(void);
void cw_sort_code(void);
code_line_t *cw_get_code_line_pending_operand(void);
bool cw_chk_click_code_op(void);
bool cw_chk_click_code_op2(int code_line_pos);

void cw_change_clicked_code_line_state(void);
void cw_cancel_pending_operand_change(void);
void cw_reset_code_execution(void);
void cw_add_saved_line(char *line);
void cw_destroy_code_window_assets(void);
void cw_destroy_renderer_assets(void);
int cw_get_instruction_y_coord(int instruction_position); //review usage
void cw_update_saved_jump_instructions(void);
SDL_Rect cw_get_text_box_rect(void);
float cw_get_challenge_highlight_limit(void);
bool cw_get_code_box_highlight_progress(float *progress,
										float animation_limit);
bool cw_get_challenge_highlight_progress(float *progress,
										float animation_limit);
void cw_init_code_window_texture(void);
SDL_Rect cw_get_code_line_coord_at_pos(int code_line_element, int pos);
/* Returns a borrowed held label on success; failure retains no candidate assets.
 * Temporary stage/controller adapter, retired with Stage 10/11 edit migration. */
code_line_t *cw_create_automatic_jump_label(code_line_t *jump);
void cw_clear_code_list(void);
void cw_assign_op_to_line(operand_t *op, code_line_t *line);
void cw_highlight_code_pending_operand(void);
SDL_Rect cw_get_stage_code_box(void);
/* Borrowed model view; callers must not mutate program ordering or ownership. */
const code_program_t *cw_get_code_program(void);
#endif
