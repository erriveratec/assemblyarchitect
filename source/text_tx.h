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

#endif
