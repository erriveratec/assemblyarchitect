#include <assert.h>
#include <math.h>
#include <stdio.h>

#include "code_layout_cly.h"

static void test_challenge_frame_clipping(void)
{
	SDL_Rect frame = {0};
	assert(cly_calculate_challenge_frame_rect(
		(SDL_Rect){.x = 8, .y = 12, .w = 20, .h = 10},
		(SDL_Rect){.x = 10, .y = 10, .w = 30, .h = 30}, 5, &frame));
	assert(SDL_RectEquals(&frame, &(SDL_Rect){.x = 10, .y = 10, .w = 23, .h = 17}));
	assert(!cly_calculate_challenge_frame_rect(
		(SDL_Rect){.x = 8, .y = 12, .w = 20, .h = 10},
		(SDL_Rect){.x = 10, .y = 10, .w = 30, .h = 30}, -1, &frame));
}

static void test_pending_operand_animation_geometry(void)
{
	const SDL_Rect base = {.x = 10, .y = 20, .w = 30, .h = 20};
	SDL_Rect animated = {0};
	assert(cly_calculate_pending_operand_rect(base, 0.0f, 10, &animated));
	assert(SDL_RectEquals(&animated,
		&(SDL_Rect){.x = 15, .y = 25, .w = 20, .h = 10}));
	assert(cly_calculate_pending_operand_rect(base, 1.0f, 10, &animated));
	assert(SDL_RectEquals(&animated, &base));
	assert(!cly_calculate_pending_operand_rect(base, NAN, 10, &animated));
}

static void test_hit_test_boundaries(void)
{
	SDL_Rect rect = {.x = 10, .y = 20, .w = 30, .h = 40};
	assert(cly_rect_contains_point(rect, 11, 21));
	assert(!cly_rect_contains_point(rect, 10, 21));
	assert(!cly_rect_contains_point(rect, 40, 21));
	assert(!cly_rect_contains_point(rect, 11, 20));
	assert(!cly_rect_contains_point(rect, 11, 60));
	assert(cly_code_line_position_at_y(100, 40, 2, 100) == 0);
	assert(cly_code_line_position_at_y(100, 40, 2, 139) == 0);
	assert(cly_code_line_position_at_y(100, 40, 2, 140) == 1);
	assert(cly_code_line_position_at_y(100, 40, 2, 180) == -1);
}

int main(void)
{
	test_challenge_frame_clipping();
	test_pending_operand_animation_geometry();
	test_hit_test_boundaries();
	puts("code layout tests passed");
	return 0;
}
