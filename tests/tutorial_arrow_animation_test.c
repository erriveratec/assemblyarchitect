#include <assert.h>
#include <math.h>
#include <SDL_ttf.h>
#include "game_mechanics_mc.h"
#include "sdl_config.h"
#include "tutorial_tr.h"

#define rr_get_failure_back_button_rect test_failure_back_button_rect
#include "../source/arrow_ar.c"
#undef rr_get_failure_back_button_rect

static bool layout_available = true;

bool test_failure_back_button_rect(SDL_Rect *button_rect)
{
	*button_rect = (SDL_Rect){
		.x = dm_scale_to_res(800), .y = dm_scale_to_res(500),
		.w = dm_scale_to_res(160), .h = dm_scale_to_res(60)
	};
	return layout_available;
}

static void assert_reset(void)
{
	SDL_Rect back_button;
	assert(test_failure_back_button_rect(&back_button));
	SDL_Rect size = ar_get_arrow_wh();
	assert(g_arrow_error.visible);
	assert(g_arrow_error.dir == AR_RIGHT);
	assert(g_arrow_error.travel == size.w);
	assert(g_arrow_error.startx + g_arrow_error.travel + size.w == back_button.x);
	assert(g_arrow_error.box.x == g_arrow_error.startx);
	assert(g_arrow_error.starty == back_button.y + back_button.h / 2 - size.h / 2);
	assert(g_arrow_error.box.y == g_arrow_error.starty);
	assert(!g_arrow_error.motion_initialized);
	assert(g_arrow_error.motion_pulse.value == 0.0f);
	assert(!g_arrow_error.motion_pulse.descending);
}

static void assert_cycles(float limit, int travel)
{
	dw_pulse_t highlight;
	dw_pulse_reset(&highlight);
	g_arrow_error.travel = travel;
	int half_cycle = (dw_get_animation_limit() + dw_get_animation_step() - 1)
		/ dw_get_animation_step();
	int upper_endpoints = 0;
	int lower_endpoints = 0;
	float previous = 0.0f;
	for (int frame = 0; frame <= 8 * (half_cycle + 1); frame++) {
		float expected = dw_pulse_progress(dw_pulse_value(&highlight, limit), limit);
		float actual = get_motion_progress(&g_arrow_error, limit);
		assert(fabsf(actual - expected) < 0.000001f);
		position_arrow(&g_arrow_error, actual);
		assert(g_arrow_error.box.x >= g_arrow_error.startx);
		assert(g_arrow_error.box.x <= g_arrow_error.startx + travel);
		assert(g_arrow_error.box.y == g_arrow_error.starty);
		if (actual == 1.0f && previous < 1.0f) {
			upper_endpoints++;
			assert(g_arrow_error.box.x == g_arrow_error.startx + travel);
		}
		if (frame > 0 && actual == 0.0f && previous > 0.0f) {
			lower_endpoints++;
			assert(g_arrow_error.box.x == g_arrow_error.startx);
		}
		previous = actual;
		dw_pulse_advance(&highlight, limit);
		assert(g_arrow_error.motion_pulse.descending == highlight.descending);
	}
	assert(upper_endpoints >= 3 && lower_endpoints >= 3);
}

static void test_resolution(dm_resolution_t resolution)
{
	dm_set_screen_resolution(resolution);
	float limits[] = {dw_get_animation_limit(), 7.3f};
	for (unsigned int index = 0; index < sizeof(limits) / sizeof(limits[0]); index++) {
		ar_init_arrow(AR_ERROR);
		assert_reset();
		assert_cycles(limits[index], g_arrow_error.travel);
		g_arrow_error.visible = false;
		float hidden_value = g_arrow_error.motion_pulse.value;
		ar_display_arrow(AR_ERROR, limits[index]);
		assert(g_arrow_error.motion_pulse.value == hidden_value);
		ar_init_arrow(AR_ERROR);
		assert_reset();
		assert_cycles(limits[index], dm_scale_to_res(73));
	}
	layout_available = false;
	ar_init_arrow(AR_ERROR);
	assert(!g_arrow_error.visible);
	layout_available = true;
	ar_init_arrow(AR_ERROR);
	assert_reset();
}

static void test_tutorial_reentry(void)
{
	assert(tr_load_level(0));
	cs_context_t context = {
		.code_size = 1,
		.code_sorted = true,
		.operation_flag = OUTPUT_BUFFER_INCOMPLETE
	};
	const tutorial_step_t *failure_step = tr_get_named_step("explain_missing_output");
	assert(failure_step != NULL);
	for (int opening = 0; opening < 3; opening++) {
		assert(tr_update(&context) == failure_step);
		assert_reset();
		float limit = dw_get_animation_limit();
		assert_cycles(limit, g_arrow_error.travel);
		float value = g_arrow_error.motion_pulse.value;
		assert(tr_update(&context) == failure_step);
		assert(g_arrow_error.motion_pulse.value == value);
		assert(tr_update(NULL) == NULL);
	}
	tr_clear();
}

int main(void)
{
	assert(SDL_Init(0) == 0);
	assert(TTF_Init() == 0);
	g_screen = SDL_CreateRGBSurfaceWithFormat(0, 1920, 1080, 32,
	                                         SDL_PIXELFORMAT_RGBA32);
	assert(g_screen != NULL);
	g_renderer = SDL_CreateSoftwareRenderer(g_screen);
	assert(g_renderer != NULL);
	g_font = TTF_OpenFont("fonts/DOSVGA437.ttf", 130);
	assert(g_font != NULL);
	test_resolution(DM_RESOLUTION_1600X900);
	test_tutorial_reentry();
	test_resolution(DM_RESOLUTION_1920X1080);
	test_tutorial_reentry();
	test_resolution(DM_RESOLUTION_1600X900);
	TTF_CloseFont(g_font);
	SDL_DestroyRenderer(g_renderer);
	SDL_FreeSurface(g_screen);
	TTF_Quit();
	SDL_Quit();
	return 0;
}