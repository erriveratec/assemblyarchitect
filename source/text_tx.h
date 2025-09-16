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
void tx_init_level_9_texts();

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

enum l9_messages{
	TX_L9_MSG1,
	TX_L9_MSG2,
	TX_L9_MSG3,
	TX_L9_MSG4,
	TX_L9_MSG5,
	TX_L9_MSG6,
	TX_L9_MAX
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


#endif
