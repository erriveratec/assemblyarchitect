#ifndef IMMEDIATES_IM_H
#define IMMEDIATES_IM_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "code_line_cl.h"
#include "ui/value_box_vb.h"
#include "gameplay/ui_highlight_source.h"

SDL_Rect im_get_cell_size(void);
SDL_Rect im_get_grid_bounds(void);
SDL_Rect im_get_label_bounds(void);

void im_set_imm_up_avail(bool state);
bool im_are_imm_up_available(void);
void im_set_highlight_source(ui_highlight_source_t source, bool enabled);
bool im_are_immediates_highlighted(void);
void im_draw_imm(float animation_limit);
void im_init_imm_assets(void);
void im_layout_grid(void);
void im_destroy_imm_assets(void);
bool im_ms_rel_in_upimm(void);
int im_get_imm_value_box_x_coord_by_id(int id);
int im_get_imm_value_box_y_coord_by_id(int id);
operand_t *im_create_sel_imm_op(void);
operand_t *im_create_imm_op_by_id(int op_id);
vb_value_box_t im_get_imm_value_box_by_id(int id);




#endif
