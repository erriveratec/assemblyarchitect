
#include "ui/button_bt.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "mouse_ms.h"
#include "dimensions_dm.h"
#include "aux.h"
#include "ui/ui_metrics_um.h"
#include <SDL_mixer.h>

static const Uint32 BUTTON_LIFT = 4;
static const float HOVER_SCALE = 0.06f;
static const Uint32 BUTTON_SHADOW_OFS = 5; // The shadow of iface buttons
static const int MODAL_BUTTON_WIDTH = 155;
static const int MODAL_BUTTON_HEIGHT = 60;

static const Uint32 IFACE_BUTTON_SPACE_W = 7; // Used for the button inner bord
static const Uint32 IFACE_BUTTON_OUTER_W = 3; //Outer width of the iface button


SDL_Rect bt_get_modal_button_size(void)
{
	SDL_Rect size = {
		.x = 0,
		.y = 0,
		.w = dm_scale_to_res(MODAL_BUTTON_WIDTH),
		.h = dm_scale_to_res(MODAL_BUTTON_HEIGHT)
	};
	return size;
}

bool bt_chk_hover_iface_btn(iface_btn_t *btn);
static void draw_btn_scaled_texture(SDL_Rect box, iface_btn_t *b);
static int get_w_iface_space_border(void);
static int get_w_iface_outer_border(void);
static int get_w_button_padding(void);
static int get_h_button_padding(void);
static void draw_button(btn_t *button, bool blk, bool hv,
	                    const SDL_Color *text_color,
	                    bool use_highlight_progress,
	                    float normalized_progress);

static float clamp_highlight_progress(float normalized_progress)
{
	if (!isfinite(normalized_progress)) {
		return 0.0f;
	}
	return fmaxf(0.0f, fminf(1.0f, normalized_progress));
}

static float get_highlight_growth(float normalized_progress, int animation_limit)
{
	if (animation_limit <= 0) {
		return 0.0f;
	}
	return clamp_highlight_progress(normalized_progress) *
	       (float)animation_limit;
}

bool bt_calculate_highlight_group_rects(const SDL_FRect *base_rects,
	                                    size_t rect_count,
	                                    float normalized_progress,
	                                    int animation_limit,
	                                    SDL_FRect *draw_rects)
{
	if (base_rects == NULL || draw_rects == NULL || rect_count == 0 ||
	    !isfinite(normalized_progress)) {
		return false;
	}
	SDL_FRect group = {0};
	for (size_t index = 0; index < rect_count; index++) {
		SDL_FRect rect = base_rects[index];
		if (!isfinite(rect.x) || !isfinite(rect.y) ||
		    !isfinite(rect.w) || !isfinite(rect.h) ||
		    rect.w <= 0.0f || rect.h <= 0.0f) {
			return false;
		}
		if (index == 0) {
			group = rect;
		} else {
			float left = fminf(group.x, rect.x);
			float top = fminf(group.y, rect.y);
			float right = fmaxf(group.x + group.w, rect.x + rect.w);
			float bottom = fmaxf(group.y + group.h, rect.y + rect.h);
			group = (SDL_FRect){left, top, right - left, bottom - top};
		}
	}
	if (!isfinite(group.w) || group.w <= 0.0f) {
		return false;
	}
	float growth = get_highlight_growth(normalized_progress, animation_limit);
	float scale = 1.0f + growth / group.w;
	return bt_calculate_highlight_group_rects_with_scale(
		base_rects, rect_count, scale, BT_HIGHLIGHT_ANCHOR_CENTER, draw_rects);
}

bool bt_calculate_button_highlight_scale(const btn_t *button,
	                                     float normalized_progress,
	                                     float *scale)
{
	if (button == NULL || button->t == NULL || scale == NULL ||
	    button->t->w <= 0 || button->t->h <= 0 ||
	    button->r.w <= 0 || button->r.h <= 0 ||
	    !isfinite(normalized_progress)) {
		return false;
	}
	float width_scale = (float)button->r.w / (float)button->t->w;
	float height_scale = (float)button->r.h / (float)button->t->h;
	float reference_extent = width_scale < height_scale ?
		(float)button->r.w : (float)button->r.h;
	float growth = get_highlight_growth(normalized_progress,
	                                   dw_get_animation_limit());
	*scale = 1.0f + growth / reference_extent;
	return isfinite(*scale) && *scale > 0.0f;
}

bool bt_calculate_highlight_group_rects_with_scale(
	const SDL_FRect *base_rects,
	size_t rect_count,
	float scale,
	bt_highlight_horizontal_anchor_t horizontal_anchor,
	SDL_FRect *draw_rects)
{
	if (base_rects == NULL || draw_rects == NULL || rect_count == 0 ||
	    !isfinite(scale) || scale <= 0.0f ||
	    (horizontal_anchor != BT_HIGHLIGHT_ANCHOR_CENTER &&
	     horizontal_anchor != BT_HIGHLIGHT_ANCHOR_LEFT)) {
		return false;
	}
	SDL_FRect group = {0};
	for (size_t index = 0; index < rect_count; index++) {
		SDL_FRect rect = base_rects[index];
		if (!isfinite(rect.x) || !isfinite(rect.y) ||
		    !isfinite(rect.w) || !isfinite(rect.h) ||
		    rect.w <= 0.0f || rect.h <= 0.0f) {
			return false;
		}
		if (index == 0) {
			group = rect;
		} else {
			float left = fminf(group.x, rect.x);
			float top = fminf(group.y, rect.y);
			float right = fmaxf(group.x + group.w, rect.x + rect.w);
			float bottom = fmaxf(group.y + group.h, rect.y + rect.h);
			group = (SDL_FRect){left, top, right - left, bottom - top};
		}
	}
	float anchor_x = horizontal_anchor == BT_HIGHLIGHT_ANCHOR_LEFT ?
		group.x : group.x + group.w / 2.0f;
	float center_y = group.y + group.h / 2.0f;
	for (size_t index = 0; index < rect_count; index++) {
		draw_rects[index] = (SDL_FRect){
			.x = anchor_x + (base_rects[index].x - anchor_x) * scale,
			.y = center_y + (base_rects[index].y - center_y) * scale,
			.w = base_rects[index].w * scale,
			.h = base_rects[index].h * scale
		};
	}
	return true;
}

static SDL_FRect get_fitted_button_rect(SDL_Rect box, const texture_t *texture)
{
	float width_scale = (float)box.w / (float)texture->w;
	float height_scale = (float)box.h / (float)texture->h;
	if (width_scale < height_scale) {
		int height = (int)((float)texture->h * width_scale);
		int y = (int)((float)box.y + ((float)box.h -
		                             (float)texture->h * width_scale) / 2.0f);
		return (SDL_FRect){
			.x = (float)box.x,
			.y = (float)y,
			.w = (float)box.w,
			.h = (float)height
		};
	}
	int width = (int)((float)texture->w * height_scale);
	int x = (int)((float)box.x + ((float)box.w -
	                             (float)texture->w * height_scale) / 2.0f);
	return (SDL_FRect){
		.x = (float)x,
		.y = (float)box.y,
		.w = (float)width,
		.h = (float)box.h
	};
}

static SDL_Rect get_button_draw_box(const bt_group_draw_item_t *item,
	                                bool *is_hovered)
{
	btn_t *button = item->button;
	bool hover = (ax_chk_mouse_hover_rect(button->r) && button->enabled &&
	              !item->blocked) || (item->hover && !item->blocked);
	*is_hovered = hover;
	SDL_Rect box = button->r;
	if (hover) {
		box = ax_scale_rect_percentage(box, bt_get_hover_factor());
		box.y -= bt_get_btn_lift();
	}
	return box;
}

static bool draw_button_group(
	const bt_group_draw_item_t *items,
	size_t item_count,
	float normalized_progress,
	bool use_explicit_scale,
	float explicit_scale,
	bt_highlight_horizontal_anchor_t horizontal_anchor)
{
	if (items == NULL || item_count == 0) {
		return false;
	}
	bt_group_draw_item_t *valid_items = calloc(item_count, sizeof(*valid_items));
	SDL_FRect *base_rects = calloc(item_count, sizeof(*base_rects));
	SDL_FRect *draw_rects = calloc(item_count, sizeof(*draw_rects));
	if (valid_items == NULL || base_rects == NULL || draw_rects == NULL) {
		free(valid_items);
		free(base_rects);
		free(draw_rects);
		return false;
	}
	size_t valid_count = 0;
	for (size_t index = 0; index < item_count; index++) {
		btn_t *button = items[index].button;
		if (button == NULL || button->t == NULL || button->t->w <= 0 ||
		    button->t->h <= 0 || button->r.w <= 0 || button->r.h <= 0) {
			continue;
		}
		bool duplicate = false;
		for (size_t previous = 0; previous < valid_count; previous++) {
			if (valid_items[previous].button == button) {
				duplicate = true;
				break;
			}
		}
		if (!duplicate) {
			valid_items[valid_count++] = items[index];
		}
	}
	if (valid_count == 0) {
		free(valid_items);
		free(base_rects);
		free(draw_rects);
		return true;
	}
	for (size_t index = 0; index < valid_count; index++) {
		bool hovered;
		SDL_Rect box = get_button_draw_box(&valid_items[index], &hovered);
		base_rects[index] = get_fitted_button_rect(box,
		                                            valid_items[index].button->t);
	}
	for (size_t index = 1; index < valid_count; index++) {
		size_t current = index;
		while (current > 0 &&
		       base_rects[current].x < base_rects[current - 1].x) {
			SDL_FRect rect = base_rects[current];
			base_rects[current] = base_rects[current - 1];
			base_rects[current - 1] = rect;
			bt_group_draw_item_t item = valid_items[current];
			valid_items[current] = valid_items[current - 1];
			valid_items[current - 1] = item;
			current--;
		}
	}
	bool geometry_valid = use_explicit_scale ?
		bt_calculate_highlight_group_rects_with_scale(
			base_rects, valid_count, explicit_scale, horizontal_anchor,
			draw_rects) :
		bt_calculate_highlight_group_rects(
			base_rects, valid_count, normalized_progress,
			dw_get_animation_limit(), draw_rects);
	if (!geometry_valid) {
		free(valid_items);
		free(base_rects);
		free(draw_rects);
		return false;
	}
	for (size_t index = 0; index < valid_count; index++) {
		btn_t *button = valid_items[index].button;
		if (button->rect) {
			dw_draw_rectangle(button->r, C_WHITE);
		}
		bool hovered;
		(void)get_button_draw_box(&valid_items[index], &hovered);
		SDL_Color color = C_LIGHTGREY;
		if ((hovered && button->enabled && !valid_items[index].blocked) ||
		    (valid_items[index].hover && !valid_items[index].blocked)) {
			color = C_WHITE;
		} else if (!button->enabled) {
			color = C_GREY;
		}
		dw_set_texture_color_mod(button->t, color);
		if (!button->enabled) {
			dw_draw_texture_rect_f(base_rects[index], button->t);
		} else {
			dw_draw_texture_rect_f(draw_rects[index], button->t);
		}
		dw_set_texture_color_mod(button->t, C_WHITE);
	}
	free(valid_items);
	free(base_rects);
	free(draw_rects);
	return true;
}

bool bt_draw_btn_group_with_highlight_progress(
	const bt_group_draw_item_t *items,
	size_t item_count,
	float normalized_progress)
{
	return draw_button_group(items, item_count, normalized_progress, false, 1.0f,
	                         BT_HIGHLIGHT_ANCHOR_CENTER);
}

bool bt_draw_btn_group_with_highlight_scale(
	const bt_group_draw_item_t *items,
	size_t item_count,
	float scale,
	bt_highlight_horizontal_anchor_t horizontal_anchor)
{
	return draw_button_group(items, item_count, 0.0f, true, scale,
	                         horizontal_anchor);
}

/* Function: get_w_button_padding
 * -----------------------------------------------------------------------------
 *	Returns the scale value for the horizontal padding
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset
 */
static int get_w_button_padding(void)
{
	return um_padding_horizontal() +
		   dm_scale_to_res(IFACE_BUTTON_SPACE_W);
}

/* Function: get_h_button_padding
 * -----------------------------------------------------------------------------
 *	Returns the scale value for the vertical padding
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset
 */
static int get_h_button_padding(void)
{
	return um_padding_vertical() +
		   dm_scale_to_res(IFACE_BUTTON_SPACE_W);
}



/* Function: get_w_iface_space_border
 * -----------------------------------------------------------------------------
 *	Returns the interfacer border space that will be use for interface buttons
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	The offset that will be use of the rectangle padding of the interface
 */
static int get_w_iface_space_border(void)
{
	return dm_scale_to_res(IFACE_BUTTON_SPACE_W);
}

/* Function: get_w_iface_outer_border
 * -----------------------------------------------------------------------------
 *	Returns the interfacer border space that will be use for interface buttons
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	The offset that will be use of the rectangle padding of the interface
 */
static int get_w_iface_outer_border(void)
{
	return dm_scale_to_res(IFACE_BUTTON_OUTER_W);
}

/* Function: bt_get_ofs_button_shadow
 * -----------------------------------------------------------------------------
 *	Returns the shadow offset for the buttons
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	The offset that will be used for the shadow of the buttons
 */
int bt_get_ofs_button_shadow(void)
{
	return dm_scale_to_res(BUTTON_SHADOW_OFS);
}

/* Function: get_hover_factor
 * -----------------------------------------------------------------------------
 * Returns the increment of the elements during hover
 * 
 * Arguments:
 *	Void.
 *
 * Return:
 *	The hover factor
 */
int bt_get_btn_lift(void)
{
	return dm_scale_to_res(BUTTON_LIFT);
}

/* Function: get_hover_factor
 * -----------------------------------------------------------------------------
 * Returns the increment of the elements during hover
 * 
 * Arguments:
 *	Void.
 *
 * Return:
 *	The hover factor
 */
float bt_get_hover_factor(void)
{
	return HOVER_SCALE;
}

/* Function: bt_destroy_iface_btn
 *-----------------------------------------------------------------------------
 * This function frees and destroys an iface button object directed by a 
 * memory pointer
 *
 * Arguments:
 * 	b: the button to be destroyed
 *	
 * Return:
 *	Void
 *
 */
void bt_destroy_iface_btn(iface_btn_t *b)
{
	SDL_DestroyTexture(b->t->texture);
	free(b->t);
	free(b);
}

/* Function: bt_chk_rel_iface_btn
 *------------------------------------------------------------------------------
 * Arguments:
 * 	button: the button to be verified
 *	
 * Return:
 *	true if the mouse clicked inside the button
 *	false if otherwise
*/
bool bt_chk_rel_iface_btn(iface_btn_t *btn, Mix_Chunk *rel_sound)
{
	assert(btn != NULL && "The button pointer cannot be NULL");

	int mouse_x = ms_get_mouse_x();
	int mouse_y = ms_get_mouse_y();
	int mouse_left_released = ms_left_released();
	
	if (mouse_left_released == false || btn->enabled == false){
		return false;
	}

	if (mouse_x > btn->r.x && mouse_x < (btn->r.x + btn->r.w) &&
		mouse_y > btn->r.y && mouse_y < (btn->r.y + btn->r.h)){
		if (rel_sound) Mix_PlayChannel(-1, rel_sound, 0);
		return true;
	} else {
		return false;
	} 
}


/* Function: bt_chk_mouse_click_iface_button
 *------------------------------------------------------------------------------
 * This function verifies is the mouse has clicked insiden an iface button.
 *
 * Arguments:
 * 	button: the button to be verified
 *	
 * Return:
 *	true if the mouse clicked inside the button
 *	false if otherwise
 *
*/
bool bt_chk_mouse_click_iface_btn(iface_btn_t *btn)
{
	assert(NULL != btn && "The button pointer cannot be NULL");

	int mouse_x = ms_get_mouse_x();
	int mouse_y = ms_get_mouse_y();
	int mouse_left_pressed = ms_left_pressed();

	bool clicked_in_button;
	if (mouse_left_pressed == false || btn->enabled == false){
		clicked_in_button = false;
	} else if (mouse_x > btn->r.x && 
			   mouse_x < (btn->r.x + btn->r.w) && 
			   mouse_y > btn->r.y && 
			   mouse_y < (btn->r.y + btn->r.h)){
		clicked_in_button = true;
	} else {
		clicked_in_button = false;
	} 
	return clicked_in_button;
}

/* Function: chk_mouse_hover_iface_button
 *------------------------------------------------------------------------------
 * This function verifies is the mouse has clicked insiden an iface button.
 *
 * Arguments:
 * 	button: the button to be verified
 *	
 * Return:
 *	true if the mouse clicked inside the button
 *	false if otherwise
 *
*/
bool bt_chk_hover_iface_btn(iface_btn_t *btn)
{
	assert(NULL != btn && "The button pointer cannot be NULL");

	int mouse_x = ms_get_mouse_x();
	int mouse_y = ms_get_mouse_y();

	bool hover;
	if (mouse_x > btn->r.x 
		&& mouse_x < (btn->r.x + btn->r.w) 
		&& mouse_y > btn->r.y 
		&& mouse_y < (btn->r.y + btn->r.h)){
		hover = true;
	} else {
		hover = false;
	} 
	return hover;
}
/* Function: bt_create_iface_btn
 * -----------------------------------------------------------------------------
 * Creates an interface button used for menus with letters
 * 	
 * Arguments:
 *	b: box with x, y, w, h
 *	t: texture of the button.
 *  enabled: boolean to determine if the button is enabled at a given time
 *
 * Return:
 *	Pointer to the button object
 */
iface_btn_t *bt_create_iface_btn(SDL_Rect r, texture_t *t, bool enabled)
{
	iface_btn_t *new_button = malloc(sizeof(iface_btn_t));
	if (new_button == NULL) {
		return NULL;
	}
	new_button->r = r;
	new_button->t = t;
	new_button->enabled = enabled;
	new_button->hovered = false;
	return new_button;
}

/* Function: draw_btn_scaled_texture
 * -----------------------------------------------------------------------------
 * Scales the texture of the interface button according to whats needed.
 * If the texture is NULL does not do anything.
 * 	
 * Arguments:
 *	box: The box that will be used to calculate the texture.
 *	b: The button that contains the texture.
 *
 * Return:
 *	Void.
 */
static void draw_btn_scaled_texture(SDL_Rect box, iface_btn_t *b)
{
	if (b->t == NULL) return;
	
	int w_pad = get_w_button_padding();
	int h_pad = get_h_button_padding();
	
	float scale_w = (float)(box.w - 2*w_pad)/b->t->w;
	float scale_h = (float)(box.h - 2*h_pad)/b->t->h;

	if (scale_w < scale_h){
		int y = box.y + (box.h - b->t->h*scale_w)/2;
		SDL_Rect r = {.x = box.x + w_pad, .y = y, .w = box.w - 2*w_pad};
		dw_draw_texture_fits_width(r, b->t);
	} else {
		int x = box.x + (int)(box.w - b->t->w*scale_h)/2;
		SDL_Rect r = {.x = x, .y = box.y + w_pad, .h = box.h - 2*h_pad};
		dw_draw_texture_fit_h(r, b->t);
	}
}

/* Function: bt_draw_iface_btn
 * -----------------------------------------------------------------------------
 * This function draws the interface button 
 * 	
 * Arguments:
 *	b: The button object that is going to be drawn
 *	blk = If true, the buttons just draws without interaction 
 *
 * Return:
 *	Void.
 */
int bt_draw_iface_btn(iface_btn_t *b, bool blk, Mix_Chunk *hover_sound)
{
	assert(b != NULL && "The button pointer is NULL");
	return bt_draw_iface_btn_at_rect(b, b->r, blk, hover_sound);
}

int bt_draw_iface_btn_at_rect(iface_btn_t *b,
							  SDL_Rect draw_rect,
							  bool blk,
							  Mix_Chunk *hover_sound)
{
	assert(b != NULL && "The button pointer is NULL");
	int status = BTN_NONE;	
	SDL_Rect box = draw_rect;
 	bool hover = bt_chk_hover_iface_btn(b);
	bool clicked = bt_chk_mouse_click_iface_btn(b);
	int offset = get_w_iface_space_border();
	int shadow_offset = bt_get_ofs_button_shadow();	

	SDL_Color border_color = C_LIGHTGREY;
	dw_set_texture_color_mod(b->t, border_color);

	if (hover == true && b->enabled == true && blk == false){
		if (b->hovered == false && hover_sound != NULL){
			if (hover_sound) Mix_PlayChannel(-1, hover_sound, 0);
			b->hovered = true;
		}
		float h_ofs = bt_get_hover_factor();
		box = ax_scale_rect_percentage(box, h_ofs);
		box.y -= bt_get_btn_lift();
		shadow_offset += 2*h_ofs;
		border_color = C_WHITE;
		dw_set_texture_color_mod(b->t, C_WHITE);
		status = BTN_HOVER;
	} else {
		b->hovered = false;
	}

	if (b->enabled == false){
		dw_draw_filled_rectangle(box, C_BLACK, C_GREY);
		SDL_Rect in = ax_pad_rectangle(box, offset, true);
		dw_draw_rectangle(in, C_GREY);
		draw_btn_scaled_texture(box, b);
	} else if (clicked == true && blk == false){
		SDL_Rect shadow = {.x = box.x + shadow_offset, 
						   .y = box.y + shadow_offset,
					   	   .w = box.w, 
						   .h = box.h};
		dw_draw_filled_rectangle(shadow, C_DIMGREY, C_DIMGREY);
		dw_draw_filled_rectangle(box, C_WHITE, C_WHITE);
		SDL_Rect in = ax_pad_rectangle(box, offset, true);
		dw_draw_filled_rectangle(in, C_NEARBLACK, C_WHITE);
		draw_btn_scaled_texture(box, b);
		status = BTN_CLICKPRESS;
	} else {
		SDL_Rect shadow = {.x = box.x + shadow_offset, 
						   .y = box.y + shadow_offset,
					   	   .w = box.w, 
						   .h = box.h};
		dw_draw_filled_rectangle(shadow, C_DIMGREY, C_DIMGREY);
		
		int outer_border = get_w_iface_outer_border();
		dw_draw_thick_rect(box, outer_border, border_color);

		SDL_Rect in = ax_pad_rectangle(box, offset, true);
		if (hover == true && blk == false){
			dw_draw_filled_rectangle(in, C_SOFTBLACK, C_WHITE);
		} else {
			dw_draw_filled_rectangle(in, C_NEARBLACK, C_SILVERGREY);
		}
		
		in = ax_pad_rectangle(in, 1, true);
		dw_draw_inner_shadow_lines(in, C_GREY, C_SILVERGREY);
		in = ax_pad_rectangle(in, 1, true);
		dw_draw_inner_shadow_lines(in, C_GREY, C_SILVERGREY);
		if (hover == true && blk == false){
			in = ax_pad_rectangle(in, 1, true);
			dw_draw_inner_shadow_lines(in, C_GREY, C_SILVERGREY);
		}
		draw_btn_scaled_texture(box, b);
	}
	
	dw_set_texture_color_mod(b->t, C_WHITE);

	return status;
}

/* Function: bt_draw_iface_btn_nopad
 * -----------------------------------------------------------------------------
 * This function draws the interface button 
 * 	
 * Arguments:
 *	b: The button object that is going to be drawn
 *
 * Return:
 *	Void.
 */
void bt_draw_iface_btn_nopad(iface_btn_t *b)
{
	assert(b != NULL && "The button pointer is NULL");
	
	int status = SUCCESS;	

	int shadow_offset = bt_get_ofs_button_shadow();	
	SDL_Rect shadow = {.x = b->r.x + shadow_offset, .y = b->r.y + shadow_offset,
					   .w = b->r.w, .h = b->r.h};
	dw_draw_filled_rectangle(shadow, C_DARKGREY, C_DARKGREY);

	bool clicked = bt_chk_mouse_click_iface_btn(b);
	int offset = get_w_iface_space_border();
	
	if (b->enabled == false){
		dw_draw_filled_rectangle(b->r, C_BLACK, C_GREY);
		SDL_Rect in = ax_pad_rectangle(b->r, offset, true);
		dw_draw_rectangle(in, C_GREY);
	} else if (clicked == true){
		dw_draw_filled_rectangle(b->r, C_WHITE, C_WHITE);
		SDL_Rect in = ax_pad_rectangle(b->r, offset, true);
		dw_draw_filled_rectangle(in, C_BLACK, C_WHITE);
	} else {
		dw_draw_filled_rectangle(b->r, C_BLACK, C_WHITE);
		SDL_Rect in = ax_pad_rectangle(b->r, offset, true);
		dw_draw_rectangle(in, C_WHITE);
	}
	
	SDL_Rect p = ax_pad_rectangle(b->r, offset, true);
	float scale_w = (float)(b->r.w - 2*offset)/b->t->w;
	float scale_h = (float)(b->r.h - 2*offset)/b->t->h;
	
	if (scale_w < scale_h){
		dw_draw_texture_fits_width(p, b->t);
	} else {
		int x = p.x + (p.w - b->t->w*scale_h)/2;
		SDL_Rect r = {.x = x, .y = p.y, .h = p.h};
		dw_draw_texture_fit_h(r, b->t);
	}
	assert(status != FAIL && "The texture could not be drawn");
}

/* Function: bt_create_btn
 * -----------------------------------------------------------------------------
 * This function is used to create a button object, it receives as 
 * arguments the parameters of the button. The scale of the texture is 
 * calculated based on the parameters of widht and height
 * 	
 * Arguments:
 *	b: box with x, y, w, h
 *	t: texture of the button.
 *  animate: boolean that determines if the texture will be animated.	
 * 	anim_state: 
 *  rect: boolean that determines if a rectangle is draw around the button.	
 *
 * Return:
 *	Pointer to the button object
 */
btn_t *bt_create_btn(SDL_Rect r, texture_t *t)
{
	btn_t *new_button = malloc(sizeof(btn_t));
	new_button->r = r;
	new_button->t = t;
	new_button->animated = false;
	new_button->anim_dir = false;
	new_button->anim_state = 0;
	new_button->rect = false;
	new_button->enabled = true;
	new_button->hovered = false;

	return new_button;
}

/* Function: bt_draw_btn
 * -----------------------------------------------------------------------------
 * This function draws the image of a given button using the according
 * scale factor of the image to fit on the button. The texture is centered
 * 	
 * Arguments:
 *	button: The button that is going to be displayed
 *	blk: Blocks the interactiver, hover state of the button.
 * 	hv: forces the hover state in case another state is needed
 *
 * Return:
 *	Void
 */
void bt_draw_btn(btn_t *b, bool blk, bool hv)
{
	draw_button(b, blk, hv, NULL, false, 0.0f);
}

void bt_draw_btn_with_highlight_progress(btn_t *button, bool blk, bool hv,
	                                    float normalized_progress)
{
	draw_button(button, blk, hv, NULL, true, normalized_progress);
}

void bt_draw_btn_with_text_color(btn_t *button, bool blk, bool hv,
	                            SDL_Color text_color)
{
	draw_button(button, blk, hv, &text_color, false, 0.0f);
}

static void draw_button(btn_t *b, bool blk, bool hv,
	                    const SDL_Color *text_color,
	                    bool use_highlight_progress,
	                    float normalized_progress)
{
	assert(b != NULL && "The button pointer is NULL");
	
	if (b->rect == true){
		dw_draw_rectangle(b->r, C_WHITE);
	}
	
	bool hover = ax_chk_mouse_hover_rect(b->r);
	SDL_Rect box = b->r;	
	dw_set_texture_color_mod(b->t, C_LIGHTGREY);

	if ((hover == true && b->enabled == true && blk == false) 
		|| (hv == true && blk == false)){
		float h_ofs = bt_get_hover_factor();
		box = ax_scale_rect_percentage(box, h_ofs);
		box.y -= bt_get_btn_lift();
		dw_set_texture_color_mod(b->t, C_WHITE);
	} else if (b->enabled == false){
		dw_set_texture_color_mod(b->t, C_GREY);
	}
	if (text_color != NULL) {
		dw_set_texture_color_mod(b->t, *text_color);
	}

	float scale_w = (float)b->r.w/b->t->w;
	float scale_h = (float)b->r.h/b->t->h;
	
	if ((!use_highlight_progress && b->animated == false) ||
	    b->enabled == false){
		if (scale_w < scale_h){
			int y = box.y + (box.h - b->t->h*scale_w)/2;
			SDL_Rect r = {.x = box.x, .y = y, .w = box.w};
			dw_draw_texture_fits_width(r, b->t);
		} else {
			int x = box.x + (box.w - b->t->w*scale_h)/2;
			SDL_Rect r = {.x = x, .y = box.y, .h = box.h};
			dw_draw_texture_fit_h(r, b->t);
		}
	} else {
		int highlight_state;
		if (use_highlight_progress) {
			if (!isfinite(normalized_progress)) {
				normalized_progress = 0.0f;
			}
			if (normalized_progress < 0.0f) normalized_progress = 0.0f;
			if (normalized_progress > 1.0f) normalized_progress = 1.0f;
			int animation_limit = dw_get_animation_limit();
		highlight_state = animation_limit > 0 ?
			(int)(normalized_progress * (float)animation_limit) : 0;
		} else {
			highlight_state = b->anim_state;
		}
		
		if (scale_w < scale_h){
			int y = b->r.y + (b->r.h - b->t->h*scale_w)/2;
			SDL_Rect r = {.x = b->r.x - highlight_state/2,
						  .y = y - highlight_state/2,
						  .w = b->r.w + highlight_state};
			dw_draw_texture_fits_width(r, b->t);
		} else {
			int x = b->r.x + (b->r.w - b->t->w*scale_h)/2;
			SDL_Rect r = {.x = x - highlight_state/2,
						  .y = b->r.y - highlight_state/2,
						  .h = b->r.h + highlight_state};
			dw_draw_texture_fit_h(r, b->t);
		}
		if (!use_highlight_progress) {
			int anim_max = dw_get_animation_limit();
			int anim_delta = dw_get_animation_step();
			if (b->anim_dir == false && b->anim_state == anim_max){
				b->anim_dir = true;
			} else if (b->anim_dir == true && b->anim_state == 0){
				b->anim_dir = false;
			}
			if (b->anim_dir == false){
				b->anim_state += anim_delta;
			} else if (b->anim_dir == true){
				b->anim_state -= anim_delta;
			}
		}
	}
	dw_set_texture_color_mod(b->t, C_WHITE);
}

/* Function: assign_button_parameters
 * This function receive a function and modifies its parameters
 * 
 * Arguments:
 *	x: The x coordinate of the button.
 *  y: The y coordiante of the button.
 *  w: The width of the button.
 *	h: The height of the button.
 * 	b: The button pointer of the object.
 *	
 * Return:
 *	void
 *
 */
void assign_button_parameters(SDL_Rect r, btn_t *b)
{
	b->r = r;
}

/* Function: bt_check_mouse_click_in_button
 *------------------------------------------------------------------------------
 * This function verifies is the mouse has clicked insiden a give button.
 *
 * Arguments:
 * 	button: the button to be verified
 *	
 * Return:
 *	true if the mouse clicked inside the button
 *	false if otherwise
 *
*/
bool bt_btn_clicked(btn_t *button)
{
	assert(NULL != button && "The button pointer cannot be NULL");

	int mouse_x = ms_get_mouse_x();
	int mouse_y = ms_get_mouse_y();
	int mouse_left_pressed = ms_left_pressed();

	bool clicked_in_button;
	if (mouse_left_pressed == false){
		clicked_in_button = false;
	} else if (mouse_x > button->r.x && 
			   mouse_x < (button->r.x + button->r.w) && 
			   mouse_y > button->r.y && 
			   mouse_y < (button->r.y + button->r.h)){
		clicked_in_button = true;
	} else {
		clicked_in_button = false;
	} 
	return clicked_in_button;
}

/* Function: bt_btn_rclicked
 *------------------------------------------------------------------------------
 * This function verifies is the mouse has right clicked insiden a give button.
 *
 * Arguments:
 * 	button: the button to be verified
 *	
 * Return:
 *	true if the mouse right clicked inside the button
 *	false if otherwise
 *
*/
bool bt_btn_rclicked(btn_t *button)
{
	assert(NULL != button && "The button pointer cannot be NULL");

	int mouse_x = ms_get_mouse_x();
	int mouse_y = ms_get_mouse_y();
	int mouse_right_pressed = ms_right_pressed();

	bool clicked_in_button;
	if (mouse_right_pressed == false){
		clicked_in_button = false;
	} else if (mouse_x > button->r.x && 
			   mouse_x < (button->r.x + button->r.w) && 
			   mouse_y > button->r.y && 
			   mouse_y < (button->r.y + button->r.h)){
		clicked_in_button = true;
	} else {
		clicked_in_button = false;
	} 
	return clicked_in_button;
}

/* Function: check_mouse_released_button
 *------------------------------------------------------------------------------
 * Arguments:
 * 	button: the button to be verified
 *	
 * Return:
 *	true if the mouse clicked inside the button
 *	false if otherwise
*/
bool bt_chk_rel_btn(btn_t *button, Mix_Chunk *rel_sound)
{
	assert(button != NULL && "The button pointer cannot be NULL");

	int mouse_x = ms_get_mouse_x();
	int mouse_y = ms_get_mouse_y();
	int mouse_left_released = ms_left_released();
	
	if (mouse_left_released == false || button->enabled == false){
		return false;
	}
	if (mouse_x > button->r.x && mouse_x < (button->r.x + button->r.w) &&
		mouse_y > button->r.y && mouse_y < (button->r.y + button->r.h)){
		if (rel_sound) Mix_PlayChannel(-1, rel_sound, 0);
		return true;
	} else {
		return false;
	} 
}


/* Function: bt_copy_button
 * This function duplicates a button object of a pointer
 * it creates different pointers that can be used on a list
 *
 * Arguments:
 * 	b: the button to be copied
 *	
 * Return:
 *	The new button object 
 *
 */
btn_t *bt_copy_button(btn_t *b)
{
	btn_t *copied_button = malloc(sizeof(btn_t));
	copied_button->r = b->r;
	copied_button->t = malloc(sizeof(texture_t));
	copied_button->t->w = b->t->w;
	copied_button->t->h = b->t->h;
	copied_button->t->texture = b->t->texture;
	copied_button->animated = b->animated;
	copied_button->anim_state = b->anim_state;
	copied_button->rect = b->rect;

	
	return copied_button;
}

/* Function: bt_destroy_button
 * This function frees and destroys a button object directed by a 
 * memory pointer
 *
 * Arguments:
 * 	b: the button to be destroyed
 *	
 * Return:
 *	Void
 *
 */
void bt_destroy_button(btn_t *b)
{
	SDL_DestroyTexture(b->t->texture);
	free(b->t);
	free(b);
}
