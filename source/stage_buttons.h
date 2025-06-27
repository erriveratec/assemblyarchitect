#ifndef STAGE_BUTTONS_H
#define STAGE_BUTTONS_H

#include <stdlib.h>
#include <stdio.h>
#include "button_bt.h"

void draw_stage_buttons(int code_size);
void initialize_stage_buttons();
bool check_clicked_stage_button();
bool check_released_in_stage_button();
int identify_clicked_stage_button();

enum stage_buttons_e {
	INVALID,
	PLAY,
	STOP,
	FORWARD,
	BACKWARD
};

extern texture_t *stop_button;
extern texture_t *step_back_button;
extern texture_t *play_button;
extern texture_t *step_forward_button;
extern texture_t *return_button;

#endif
