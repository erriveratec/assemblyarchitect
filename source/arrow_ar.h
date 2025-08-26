#ifndef ARROW_AR_H
#define ARROW_AR_H
#include<stdio.h>
#include<stdlib.h>
#include"text_tx.h"
#include"dimensions.h"
#include"draw_dw.h"


typedef struct arrow_t{
	SDL_Rect box;
	int startx;
	int starty;
	int travel;
	int dir;
	bool in_place;
	bool visible;
	texture_t *texture;
} arrow_t;

enum arrow_id{
	AR_INS,
	AR_DROP,
	AR_PLAY,
	AR_CODE,
	AR_DEL,
	AR_OP2,
	AR_ERROR,
	AR_CHALLENGE
};

void ar_initialize_arrows();
void ar_display_arrow(int arrow_id);
void dw_animate_arrow(int start_x, int start_y, arrow_t *arrow, 
														   int dir, int travel);

enum movement{
	DW_MOV_MIN,
	DW_UP,
	DW_DOWN,
	DW_LEFT,
	DW_RIGHT,
	DW_MOV_MAX
};

#define ARROW_INS_X (INS_BOX_X + INS_BOX_W)/4
#define ARROW_INS_Y INS_BOX_Y + (MSG_INS_BOX_Y - INS_BOX_Y)*3/4
#define ARROW_CODE_X (INS_BOX_X + INS_BOX_W)/2
#define ARROW_CODE_Y (MSG_INS_BOX_Y + MSG_BOX_H + ARROW_H)


#endif

