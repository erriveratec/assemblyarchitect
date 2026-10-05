#include "ui/value_box_vb.h"

#include <stdlib.h>

#include "aux.h"
#include "dimensions_dm.h"

static const int VALUE_BOX_WIDTH = 50;
static const int VALUE_BOX_HEIGHT = 40;

SDL_Rect vb_get_size(void)
{
	SDL_Rect size = {
		.x = 0,
		.y = 0,
		.w = dm_scale_to_res(VALUE_BOX_WIDTH),
		.h = dm_scale_to_res(VALUE_BOX_HEIGHT)
	};
	return size;
}

SDL_Rect vb_get_text_size(void)
{
	SDL_Rect size = vb_get_size();
	size.h -= size.h / 10;
	return size;
}

int vb_get_vertical_offset(void)
{
	return vb_get_size().h / 4;
}

void vb_copy(vb_value_box_t *dst, vb_value_box_t src, bool copy_position)
{
	dst->value = src.value;
	dst->type = src.type;
	dst->visible_box = src.visible_box;
	if (copy_position) {
		dst->box = src.box;
	}
	dw_free_texture(dst->t);
	char *number = ax_number_to_string(src.value);
	dst->t = dw_create_text_tex(number, C_WHITE);
	free(number);
}

void vb_draw(vb_value_box_t *box, SDL_Color color)
{
	SDL_Rect text_size = vb_get_text_size();
	int text_width = box->t == NULL
		? 0
		: ax_get_texture_w_fit_h(text_size.h, box->t);

	dw_draw_filled_rectangle(box->box, C_BLACK, color);

	int x_offset = (text_size.w - text_width) / 2;
	int y_offset = (text_size.h / 5) / 2;
	SDL_Rect text_bounds = {
		.x = box->box.x + x_offset,
		.y = box->box.y + y_offset,
		.w = text_width,
		.h = text_size.h
	};
	if (box->t != NULL) {
		dw_draw_texture_fit_h(text_bounds, box->t);
	}
}
