#ifndef CODE_LAYOUT_CLY_H
#define CODE_LAYOUT_CLY_H

#include <stdbool.h>
#include <SDL.h>

bool cly_calculate_challenge_frame_rect(
	SDL_Rect text_bounds, SDL_Rect container, int allowance, SDL_Rect *frame);
bool cly_calculate_pending_operand_rect(
	SDL_Rect base_operand_rect, float normalized_progress, int animation_limit,
	SDL_Rect *animated_operand_rect);
SDL_Rect cly_calculate_stage_code_box(
	SDL_Rect instruction_bounds, SDL_Rect stage_buttons,
	Uint32 width, Uint32 y);
int cly_code_line_spacing(int button_height, int vertical_padding);
int cly_operand_offset(int button_width, int horizontal_padding, int operand);
int cly_code_line_y(int base_y, int spacing, int position, int list_size);
SDL_Rect cly_code_line_rect(SDL_Rect base, int x_offset, int spacing,
	int position, int list_size);
bool cly_rect_contains_point(SDL_Rect rect, int x, int y);
int cly_code_line_position_at_y(
	int first_y, int spacing, int list_size, int y);

#endif
