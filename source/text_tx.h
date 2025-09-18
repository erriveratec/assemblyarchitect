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
void tx_set_and_allocate_msgs_array(int size);
void tx_set_message_in_array(int pos, char *msg);

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
enum msgs{
	MSG1,
	MSG2,
	MSG3,
	MSG4,
	MSG5,
	MSG6,
	MSG7,
	MSG8,
	MSG9,
	MSG10,
	MSG11,
	MSG12,
	MSG13,
	MSG14,
	MSG15,
	MSG16,
	MSG17,
	MSG18,
	MSG19,
	MSG20,
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


#endif
