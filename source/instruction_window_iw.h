#ifndef INSTRUCTION_WINDOW_IW_H
#define INSTRUCTION_WINDOW_IW_H

#include "list.h"
#include "code_line_cl.h"

void iw_create_instruction_list();
void iw_destroy_instruction_list();
void iw_add_instruction_to_list(int id);
bool iw_check_clicked_instruction();
instruction_t *iw_get_clicked_instruction();
void iw_display_instructions();
void iw_set_instruction_box(SDL_Rect r);
int iw_get_instruction_y_by_id(int id);
int iw_get_instruction_list_size();
SDL_Rect iw_get_instruction_rect_by_pos(int pos);
void iw_init_ins_box_texture();
#endif
