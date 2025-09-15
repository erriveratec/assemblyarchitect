#ifndef TEXT_TX_H
#define TEXT_TX_H
#include<stdio.h>
#include<stdlib.h>
#include"dimensions_dm.h"
#include"draw_dw.h"

void tx_init_global_msgs();
void tx_text_box(int pos, int msg_id);
void tx_bottom_msg(int pos, int msg_id);
int tx_get_text_box_member(int text_box_id, int member);
void tx_init_level_text_textures(int level_id);
void tx_init_text_boxes();
void tx_free_level_text_textures();
void tx_init_level_1_texts();
void tx_init_level_2_texts();
void tx_init_level_3_texts();
void tx_init_level_4_texts();
void tx_init_level_5_texts();
void tx_init_level_6_texts();
void tx_init_level_7_texts();
void tx_init_level_8_texts();

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

enum l8_messages{
	TX_L8_WELCOME,
	TX_L8_DESCRIPTION1,
	TX_L8_DESCRIPTION2,
	TX_L8_MAX
};

enum l7_messages{
	TX_L7_WELCOME,
	TX_L7_DESCRIPTION1,
	TX_L7_DESCRIPTION2,
	TX_L7_MAX
};

enum l6_messages{
	TX_L6_WELCOME,
	TX_L6_DESCRIPTION1,
	TX_L6_DESCRIPTION2,
	TX_L6_SELINS,
	TX_L6_MAX
};

enum l5_messages{
	TX_L5_WELCOME,
	TX_L5_DESCRIPTION1,
	TX_L5_MAX
};

enum l4_messages{
	TX_L4_WELCOME,
	TX_L4_DESCRIPTION1,
	TX_L4_MAX
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


#define L9_MSG_NINTH_CHALLENGE "You have arrive to level 09. There is a new"\
" instruction: \"jmp\", which jumps to a place pointed by a LABEL"\
" (Click anywhere to continue)."


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
