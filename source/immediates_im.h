#ifndef IMMEDIATES_IM_H
#define IMMEDIATES_IM_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "code_line_cl.h"

void im_set_imm_up_avail(bool state);
void im_draw_imm();
void im_init_imm_assets();
bool im_ms_rel_in_upimm();
operand_t *im_create_sel_imm_op();
operand_t *im_create_imm_op_by_id(int op_id);




#endif
