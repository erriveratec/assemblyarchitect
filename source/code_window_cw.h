#ifndef CODE_WINDOW_H
#define CODE_WINDOW_H

#include <SDL.h>
#include "code_line_cl.h"
#include "list.h"
#include "button_bt.h"
#include "mouse_ms.h"

#define CW_EMPTY 0

enum operand_positions{
	FIRST_OP,
	SECOND_OP
};

void cw_draw_code_window();
void cw_create_code_list();
void cw_set_code_box(int x, int y, int w, int h);
void cw_set_challenge_text(char *text);
void cw_set_stage_name(char *text);
void cw_player_holding_instruction(code_line_t *instruction);
bool cw_check_if_in_code_list(code_line_t *instruction);
bool cw_check_clicked_code();
code_line_t *cw_get_clicked_code();
bool cw_check_code_sorted();
void cw_sort_code();
void cw_highlight_code_pending_operand();
bool cw_check_code_pending_operand();
bool cw_check_code_pending_op1();
bool cw_check_code_pending_op2();
code_line_t *cw_get_code_line_pending_operand();
bool cw_check_clicked_code_operand();
void cw_change_clicked_code_line_state();
int cw_get_code_list_size();
int cw_get_instruction_at_code_pos(int position);
int cw_get_instruction_operand(int position, int operand_number);
code_line_t *cw_get_code_line_at_pos(int pos);
void cw_reset_code_execution();
void cw_add_saved_line(char *line);
void cw_destroy_code_window_assets();
int cw_get_instruction_y_coord(int instruction_position); //review usage
int cw_get_code_line_x(int instruction_id);
bool cw_check_released_in_label();
operand_t *cw_create_jump_operand();
void cw_update_saved_jump_instructions();
void cw_operate_jump_instruction(code_line_t *line);
int cw_get_code_box_member(int member);
int cw_get_line_y(int pos);
int cw_get_text_box_member(int member);

#endif
