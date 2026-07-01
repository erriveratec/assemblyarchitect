#ifndef BUTTON_BT_H
#define BUTTON_BT_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "draw_dw.h"
#include <SDL_mixer.h>

typedef struct btn_t{
	SDL_Rect r;
	texture_t *t;
	bool animated;
	bool anim_dir;
	int anim_state;
	bool rect;
	bool enabled;
	bool hovered;
} btn_t;

typedef struct iface_btn_t{
	SDL_Rect r;
	texture_t *t;
	bool enabled;
	bool hovered;
} iface_btn_t;

enum btn_states{
	BTN_NONE,
	BTN_HOVER,
	BTN_CLICKPRESS,
	BTN_CLICKREL
};

btn_t *bt_create_btn(SDL_Rect b, texture_t *t);
void bt_draw_btn(btn_t *button, bool blk, bool hv);
bool bt_btn_clicked(btn_t *button);
bool bt_btn_rclicked(btn_t *button);

bool bt_chk_rel_btn(btn_t *button, Mix_Chunk *rel_sound);
void assign_button_parameters(SDL_Rect r, btn_t *b);
btn_t *bt_copy_button(btn_t *b);
void bt_destroy_button(btn_t *b);
iface_btn_t *bt_create_iface_btn(SDL_Rect r, texture_t *t, bool enabled);

int bt_draw_iface_btn(iface_btn_t *b, bool blk, Mix_Chunk *hover_sound);
void bt_draw_iface_btn_nopad(iface_btn_t *b);
bool bt_chk_mouse_click_iface_btn(iface_btn_t *button);
bool bt_chk_rel_iface_btn(iface_btn_t *button, Mix_Chunk *rel_sound);
void bt_destroy_iface_btn(iface_btn_t *b);
float bt_get_hover_factor();
int bt_get_btn_lift();

int bt_get_ofs_button_shadow();
bool bt_chk_hover_iface_btn(iface_btn_t *btn);

#endif
