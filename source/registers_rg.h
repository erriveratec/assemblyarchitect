#ifndef REGISTERS_H
#define REGISTERS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "list.h"
#include "code_line_cl.h"
#include "aux.h"


typedef struct reg_t{
	button_t *b;
	int id;
	value_box_t value;
} reg_t;

void create_register_list();
void rg_set_register_box(int x, int y, int w, int h);
int rg_get_registers_text_width(int h);
void rg_add_register_to_list(int id);
void rg_display_registers();
bool rg_check_released_in_register();
operand_t *rg_get_default_operand_register();
operand_t *rg_create_operand_of_selected_register();
operand_t *rg_create_register_operand_by_id(int id);
int rg_get_register_value_box_x_coord_by_id(int id);
int rg_get_register_value_box_y_coord_by_id(int id);
value_box_t get_register_value_box_by_id(int id);
void rg_set_register_value_box(int id, value_box_t val);
void reset_register_values();
void rg_destroy_register_list();
void rg_update_register_box_position();
int rg_get_register_box_member(int member);
void rg_initialize_value_boxes();
#endif
