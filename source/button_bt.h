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
	SDL_Rect r;
	bool act;
	bool rect;
	bool fill;
	SDL_Color in;
	SDL_Color out;
	texture_t *t;
} button_t;

typedef struct iface_btn_t{
	SDL_Rect r;
	texture_t *t;
	bool enabled;
} iface_btn_t;

button_t *bt_create_button(SDL_Rect b, bool act, bool rect, int fill, 
									 SDL_Color in , SDL_Color out,texture_t *t);
void bt_draw_button(button_t *button, bool padding);
bool bt_check_mouse_click_button(button_t *button);
bool bt_check_mouse_released_button(button_t *button);
void assign_button_parameters(SDL_Rect r, button_t *b);
button_t *bt_copy_button(button_t *b);
void bt_destroy_button(button_t *b);
iface_btn_t *bt_create_iface_btn(SDL_Rect r, texture_t *t, bool enabled);
void bt_draw_iface_btn(iface_btn_t *b);
void bt_draw_iface_btn_nopad(iface_btn_t *b);
bool bt_chk_mouse_click_iface_btn(iface_btn_t *button);
bool bt_chk_mouse_rel_iface_btn(iface_btn_t *button);
void bt_destroy_iface_btn(iface_btn_t *b);
#endif
