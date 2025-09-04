#ifndef TEXT_TX_H
#define TEXT_TX_H
#include<stdio.h>
#include<stdlib.h>
#include"dimensions_dm.h"
#include"draw_dw.h"

void tx_init_global_msgs();
void tx_text_box(int pos, int msg_id);
void tx_bottom_msg(int pos, int msg_id);
void tx_upd_boxes_pos();
int tx_get_text_box_member(int text_box_id, int member);
void tx_init_level_text_textures(int level_id);
void tx_init_text_boxes();
void tx_free_level_text_textures();
void tx_init_level_1_texts();
void tx_init_level_2_texts();
void tx_init_level_3_texts();

enum text_box_positions{
	TX_BOX_MIN,
	TX_INS_BOX,
	TX_UPPER_BOX,
	TX_CENTER_BOX,
	TX_LOWER_BOX,
	TX_CODE_BOX,
	TX_STAGEBUTTON_BOX,
	TX_BIG_BOX,
	TX_ERROR_BOX,
	TX_BOX_MAX
};

enum gbl_msgs{
	TX_MSG_CLICKANY,
	TX_MSG_PRESSPLAY,
	TX_MSG_PRESSBACK
};

enum l3_messages{
	TX_L3_WELCOME,
	TX_L3_DESCRIPTION1,
	TX_L3_DESCRIPTION2,
	TX_L3_SELINS1,
	TX_L3_DROPINS,
	TX_L3_AVAILOPS1,
	TX_L3_SELRAX,
	TX_L3_AVAILOPS2,
	TX_L3_SELIB,
	TX_L3_SELINS2,
	TX_L3_READ,
	TX_L3_MAX
};

enum l2_messages{
	TX_L2_WELCOME,
	TX_L2_DESCRIPTION,
	TX_L2_SELLAST,
	TX_L2_DELINS,
	TX_L2_CHANGEOP,
	TX_L2_SELIB,
	TX_L2_MOVINS,
	TX_L2_PRESSPLAY,
	TX_L2_MAX
};

enum l1_messages{
	TX_L1_WELCOME,
	TX_L1_DESCRIPTION,
	TX_L1_OBJ1,
	TX_L1_OBJ2,
	TX_L1_OBJ3,
	TX_L1_SELINS1,
	TX_L1_DROPINS1,
	TX_L1_SELOP1,
	TX_L1_SELOP2,
	TX_L1_PLAYTUT,
	TX_L1_SELINS2,
	TX_L1_ERROR,
	TX_L1_DROPINS2,
	TX_L1_SELOP1_2,
	TX_L1_SELOP2_2,
	TX_L1_PRESSPLAY,
	TX_L1_CONGRATS,
	TX_L1_MAX
};

#define TUT_TEXT_X INS_BOX_X
#define TUT_TEXT_Y SCREEN_HEIGHT/2 - MSG_BOX_H

#define MSG_BOX_W (INS_BOX_W + 100)
#define MSG_BOX_H 210

#define MSG_INS_BOX_X INS_BOX_X
#define MSG_INS_BOX_Y (SCREEN_HEIGHT/2 - MSG_BOX_H/3)


#define ERROR_BOX_Y_OFFSET 60
#define ERROR_MSG_BOX_X RES_BOX_X + MSG_BOX_W/2 - ARROW_W/2
#define ERROR_MSG_BOX_Y RES_BOX_Y + RES_BOX_H + ERROR_BOX_Y_OFFSET

#define L4_MSG_FOURTH_CHALLENGE "Welcome to Level 04. In this level you "\
" solve the challenge by yourself applying what has been learned."\
" (Click anywhere to continue)."

#define L5_MSG_FIFTH_CHALLENGE "Welcome to Level 05. Notice that now are more"\
" registers available. You will need them to solve the challenge."\
" (Click anywhere to continue)."

#define L6_MSG_SIXTH_CHALLENGE "Welcome to Level 06. Notice that there is"\
" a new instruction available \"add\". "\
" (Click anywhere to continue)."

#define L7_MSG_SEVENTH_CHALLENGE "Welcome to Level 07. Did you know that"\
" instructions \"mov\" \"add\" can be used with both operands being registers?"\
" (Click anywhere to continue)."

#define L8_MSG_EIGHTH_CHALLENGE "You are now in Level 08. I have new info for"\
" you: Both operands of instructions \"mov\" \"add\" can be the same register."\
" (Click anywhere to continue)."

#define L9_MSG_NINTH_CHALLENGE "You have arrive to level 09. There is a new"\
" instruction: \"jmp\", which jumps to a place pointed by a LABEL"\
" (Click anywhere to continue)."

#define L5_MSG_NEW_REGS "Notice that now are more registers available"\
" try using them to solve the challenge."

#define L6_MSG_ADD "For Example:        \"add OP1, OP2\" adds OP1 + OP2"\
" and stores the result in OP1. (Click anywhere to continue)."

#define L6_MSG_SOLVE "Solve the challenge using a combination of the"\
" \"mov\" and \"add\" instructions."\

#define L7_MSG_ADD "For Example:        \"add rax, rbx\" adds rax + rbx"\
" and stores the result in rax. (Click anywhere to continue)."

#define L8_MSG_ADD "For Example:        \"add rax, rax\" adds rax + rax"\
" and stores the result in rax. (Click anywhere to continue)."

#define L9_MSG_NEW_INS_JMP "The operand of the \"jmp\" instruction is a "\
" LABEL that must be defined in the code."\
" (Click anywhere to continue)."

#define L9_MSG_RESTRICTION "Restriction: the maximum"\
" number of instructions that can be used to solve de challenge is 4."\
" (Click anywhere to continue)."

#define L9_MSG_SELECT_LABEL "Select an drag a LABEL from the instruction box."

#define MSG_COMPAT_OPS "Notice: Not all operands are compatible with"\
" each other, for instance the"\
" instruction:  mov [ob], [ib]"\
" is not possible."






#endif
