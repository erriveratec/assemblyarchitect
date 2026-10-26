#ifndef BUTTON_BT_H
#define BUTTON_BT_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "draw_dw.h"
#include <SDL_mixer.h>

SDL_Rect bt_get_modal_button_size(void);

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

typedef struct bt_group_draw_item_t {
	btn_t *button;
	bool blocked;
	bool hover;
} bt_group_draw_item_t;

typedef enum bt_highlight_horizontal_anchor_t {
	BT_HIGHLIGHT_ANCHOR_CENTER,
	BT_HIGHLIGHT_ANCHOR_LEFT
} bt_highlight_horizontal_anchor_t;

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
void bt_draw_btn_with_highlight_progress(btn_t *button, bool blk, bool hv,
	                                    float normalized_progress);
bool bt_calculate_highlight_group_rects(const SDL_FRect *base_rects,
	                                    size_t rect_count,
	                                    float normalized_progress,
	                                    int animation_limit,
	                                    SDL_FRect *draw_rects);
bool bt_calculate_button_highlight_scale(const btn_t *button,
	                                     float normalized_progress,
	                                     float *scale);
bool bt_calculate_highlight_group_rects_with_scale(
	const SDL_FRect *base_rects,
	size_t rect_count,
	float scale,
	bt_highlight_horizontal_anchor_t horizontal_anchor,
	SDL_FRect *draw_rects);
bool bt_draw_btn_group_with_highlight_progress(
	const bt_group_draw_item_t *items,
	size_t item_count,
	float normalized_progress);
bool bt_draw_btn_group_with_highlight_scale(
	const bt_group_draw_item_t *items,
	size_t item_count,
	float scale,
	bt_highlight_horizontal_anchor_t horizontal_anchor);
void bt_draw_btn_with_text_color(btn_t *button, bool blk, bool hv,
	                            SDL_Color text_color);
bool bt_btn_clicked(btn_t *button);
bool bt_btn_rclicked(btn_t *button);

bool bt_chk_rel_btn(btn_t *button, Mix_Chunk *rel_sound);
void assign_button_parameters(SDL_Rect r, btn_t *b);
btn_t *bt_copy_button(btn_t *b);
void bt_destroy_button(btn_t *b);
iface_btn_t *bt_create_iface_btn(SDL_Rect r, texture_t *t, bool enabled);

int bt_draw_iface_btn(iface_btn_t *b, bool blk, Mix_Chunk *hover_sound);
int bt_draw_iface_btn_at_rect(iface_btn_t *b,
							  SDL_Rect draw_rect,
							  bool blk,
							  Mix_Chunk *hover_sound);
void bt_draw_iface_btn_nopad(iface_btn_t *b);
bool bt_chk_mouse_click_iface_btn(iface_btn_t *button);
bool bt_chk_rel_iface_btn(iface_btn_t *button, Mix_Chunk *rel_sound);
void bt_destroy_iface_btn(iface_btn_t *b);
float bt_get_hover_factor(void);
int bt_get_btn_lift(void);

int bt_get_ofs_button_shadow(void);
bool bt_chk_hover_iface_btn(iface_btn_t *btn);

#endif
