#ifndef ARROW_AR_H
#define ARROW_AR_H
#include<stdio.h>
#include<stdlib.h>
#include"text_tx.h"
#include"dimensions_dm.h"
#include"draw_dw.h"

extern texture_t *g_lv_arrow;
extern texture_t *g_ib_arrow;
extern texture_t *g_ob_arrow;

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
	AR_CHALLENGE,
	AR_IB,
	AR_OB
};

void ar_initialize_arrows();
void ar_display_arrow(int arrow_id);
void ar_animate_arrow(arrow_t *arrow);

enum movement{
	AR_MOV_MIN,
	AR_UP,
	AR_DOWN,
	AR_LEFT,
	AR_RIGHT,
	AR_MOV_MAX
};

#define ARROW_INS_X (INS_BOX_X + INS_BOX_W)/4
#define ARROW_CODE_X (INS_BOX_X + INS_BOX_W)/2

#endif

