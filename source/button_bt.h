#ifndef BUTTON_BT_H
#define BUTTON_BT_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "draw_dw.h"
#include "aux.h"
#include "list.h"
#include "mouse_ms.h"

typedef struct button_t{
	int x;
	int y;
	int w;
	int h;
	bool active;
	bool rectangle;
	bool filled;
	SDL_Color in;
	SDL_Color out;
	texture_t *texture;
} button_t;

button_t *create_button(int x, int y, int w, int h, bool act, bool rect, 
 									texture_t *t, SDL_Color in , SDL_Color out);
void bt_draw_button(button_t *button);
bool bt_check_mouse_click_button(button_t *button);
bool bt_check_mouse_released_button(button_t *button);
void assign_button_parameters(int x, int y, int w, int h, button_t *b);
button_t *bt_copy_button(button_t *b);
void bt_destroy_button(button_t *b);
#endif
