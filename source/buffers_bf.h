#ifndef BUFFERS_BF_H
#define BUFFERS_BF_H
#include <stdio.h>
#include <stdlib.h>
#include "code_line_cl.h"

enum input_list_type{
	NOT_ASSIGNED,
	NATURAL,
	WHOLE,
	CHAR,
	MIXED
};

extern int g_input_list_type;
extern int g_input_buffer_size;

bool bf_check_released_in_buffer();
void bf_set_input_box(SDL_Rect r);
void bf_set_output_box(SDL_Rect r);
void bf_set_input_buffer_button(SDL_Rect r);
void bf_set_output_buffer_button(SDL_Rect r);
void bf_create_input_list();
void bf_create_output_list();
void bf_destroy_buffer_lists();
void bf_create_natural_numbers_input_list(int size);
void bf_initialize_buffer_operands();
void bf_draw_buffers();
int bf_get_buffer_value_box_x_coord_by_id(int op_id);
int bf_get_buffer_value_box_y_coord_by_id(int op_id);
value_box_t bf_get_input_buffer_value_box();
value_box_t bf_get_output_buffer_value_box();
void bf_reset_input_list();
void bf_reset_output_list();
void bf_add_output_to_list();
void bf_set_output_buffer_value_box(value_box_t val);
bool check_if_output_buffer_position_set();
int get_input_buffer_list_size();
int get_output_buffer_list_size();
List *get_input_list();
List *get_output_list();
void print_output_list();
operand_t *bf_create_operand_of_selected_buffer();
operand_t *bf_create_buffer_operand_by_id(int id);
void bf_reset_input_list_x_pos();
void bf_set_input_buffer_size(int size);
void bf_set_win_condition();
void bf_reset_win_condition();
void bf_init_buffer_assets();
#endif
