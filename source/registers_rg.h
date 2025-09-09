#ifndef REGISTERS_RG_H
#define REGISTERS_RG_H

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
void rg_set_register_box(SDL_Rect r);
int rg_get_registers_text_width(int h);
void rg_add_register_to_list(int id);
void rg_draw_registers();
bool rg_check_mouse_released_in_register();
operand_t *rg_get_default_operand_register();
operand_t *rg_create_operand_of_selected_register();
operand_t *rg_create_register_operand_by_id(int id);
int rg_get_register_value_box_x_coord_by_id(int id);
int rg_get_register_value_box_y_coord_by_id(int id);
value_box_t rg_get_register_value_box_by_id(int id);
void rg_set_register_value_box(int id, value_box_t val);
void rg_reset_register_values();
void rg_destroy_register_list();
void rg_update_register_box_position();
void rg_init_value_boxes();
int rg_get_ibox_x();
int rg_get_ibox_y();
int rg_get_obox_y();
value_box_t rg_get_ibox_value_box();
value_box_t rg_get_obox_value_box();
void rg_set_ibox_value_box(value_box_t value);	
void rg_set_obox_value_box(value_box_t value);	
void rg_reset_ibox();
void rg_reset_obox();
void rg_destroy_value_boxes();
void rg_init_reg_texture();
SDL_Rect rg_get_register_box();
List *rg_get_register_list();
#endif
