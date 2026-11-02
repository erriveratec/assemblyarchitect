#include "code_layout_cly.h"

#include <limits.h>
#include <math.h>
#include <stdint.h>

bool cly_calculate_challenge_frame_rect(
	SDL_Rect text_bounds, SDL_Rect container, int allowance, SDL_Rect *frame)
{
	if (frame == NULL || text_bounds.w <= 0 || text_bounds.h <= 0 ||
	    container.w <= 0 || container.h <= 0 || allowance < 0) {
		return false;
	}
	int64_t left = (int64_t)text_bounds.x - allowance;
	int64_t top = (int64_t)text_bounds.y - allowance;
	int64_t right = (int64_t)text_bounds.x + text_bounds.w + allowance;
	int64_t bottom = (int64_t)text_bounds.y + text_bounds.h + allowance;
	int64_t container_right = (int64_t)container.x + container.w;
	int64_t container_bottom = (int64_t)container.y + container.h;
	if (left < container.x) left = container.x;
	if (top < container.y) top = container.y;
	if (right > container_right) right = container_right;
	if (bottom > container_bottom) bottom = container_bottom;
	if (right <= left || bottom <= top || left < INT_MIN || top < INT_MIN ||
	    right > INT_MAX || bottom > INT_MAX || right - left > INT_MAX ||
	    bottom - top > INT_MAX) {
		return false;
	}
	*frame = (SDL_Rect){
		.x = (int)left,
		.y = (int)top,
		.w = (int)(right - left),
		.h = (int)(bottom - top)
	};
	return true;
}

bool cly_calculate_pending_operand_rect(
	SDL_Rect base_operand_rect, float normalized_progress, int animation_limit,
	SDL_Rect *animated_operand_rect)
{
	if (animated_operand_rect == NULL ||
	    !isfinite(normalized_progress) || base_operand_rect.w <= 0 ||
	    base_operand_rect.h <= 0) {
		return false;
	}
	if (animation_limit <= 0) {
		*animated_operand_rect = base_operand_rect;
		return true;
	}

	float progress = fmaxf(0.0f, fminf(1.0f, normalized_progress));
	float maximum_inset = fminf((float)animation_limit,
	                            fminf((float)(base_operand_rect.w - 1),
	                                  (float)(base_operand_rect.h - 1)));
	float current_inset = maximum_inset * (1.0f - progress);
	SDL_FRect rect = {
		.x = (float)base_operand_rect.x + current_inset / 2.0f,
		.y = (float)base_operand_rect.y + current_inset / 2.0f,
		.w = fmaxf(1.0f, (float)base_operand_rect.w - current_inset),
		.h = fmaxf(1.0f, (float)base_operand_rect.h - current_inset)
	};
	*animated_operand_rect = (SDL_Rect){
		.x = (int)lroundf(rect.x),
		.y = (int)lroundf(rect.y),
		.w = (int)lroundf(rect.w),
		.h = (int)lroundf(rect.h)
	};
	if (animated_operand_rect->w < 1) animated_operand_rect->w = 1;
	if (animated_operand_rect->h < 1) animated_operand_rect->h = 1;
	return true;
}

SDL_Rect cly_calculate_stage_code_box(
	SDL_Rect instruction_bounds, SDL_Rect stage_buttons, Uint32 width, Uint32 y)
{
	return (SDL_Rect){
		.x = instruction_bounds.x + instruction_bounds.w,
		.y = (int)y,
		.w = (int)width,
		.h = stage_buttons.y - 2 * (int)y
	};
}

int cly_code_line_spacing(int button_height, int vertical_padding)
{
	return button_height + vertical_padding;
}

int cly_operand_offset(int button_width, int horizontal_padding, int operand)
{
	return operand == 1 ? button_width : 2 * button_width + horizontal_padding;
}

int cly_code_line_y(int base_y, int spacing, int position, int list_size)
{
	int y = base_y;
	if (list_size == 0 || position == 0) {
		return y;
	}
	for (int index = 0; index < list_size; index++) {
		y += spacing;
		if (position == index) {
			break;
		}
	}
	return y;
}

SDL_Rect cly_code_line_rect(SDL_Rect base, int x_offset, int spacing,
	int position, int list_size)
{
	base.x += x_offset;
	if (list_size == 0 || position == 0) {
		return base;
	}
	for (int index = 0; index < list_size; index++) {
		base.y += spacing;
		if (position == index) {
			break;
		}
	}
	return base;
}

bool cly_rect_contains_point(SDL_Rect rect, int x, int y)
{
	return x > rect.x && x < rect.x + rect.w &&
		y > rect.y && y < rect.y + rect.h;
}

int cly_code_line_position_at_y(
	int first_y, int spacing, int list_size, int y)
{
	if (spacing <= 0 || list_size <= 0) {
		return -1;
	}
	for (int index = 0; index < list_size; index++) {
		if (y >= first_y && y < first_y + spacing) {
			return index;
		}
		first_y += spacing;
	}
	return -1;
}
