#include <assert.h>
#include <math.h>
#include <SDL_ttf.h>
#include "game_mechanics_mc.h"
#include "sdl_config.h"
#include "tutorial_tr.h"

static void test_draw_rotated_texture(int x, int y, int height, double angle,
									  texture_t *texture);
#define iw_get_highlight_instruction_progress test_instruction_progress
#define cw_get_code_box_highlight_progress test_code_box_progress
#define cw_get_challenge_highlight_progress test_challenge_progress
#define bf_get_input_buffer_highlight_progress test_input_progress
#define bf_get_output_buffer_highlight_progress test_output_progress
#define rg_get_register_highlight_progress test_register_progress
#define sb_get_button_highlight_progress test_button_progress
#define im_get_highlight_progress test_immediate_progress
#define rr_get_failure_back_button_rect test_failure_back_button_rect
#define dw_draw_rotated_texture_fits_h test_draw_rotated_texture
#include "../source/arrow_ar.c"
#undef dw_draw_rotated_texture_fits_h
#undef rr_get_failure_back_button_rect
#undef im_get_highlight_progress
#undef sb_get_button_highlight_progress
#undef rg_get_register_highlight_progress
#undef bf_get_output_buffer_highlight_progress
#undef bf_get_input_buffer_highlight_progress
#undef cw_get_challenge_highlight_progress
#undef cw_get_code_box_highlight_progress
#undef iw_get_highlight_instruction_progress

static float provider_progress[HIGHLIGHT_IMMEDIATES + 1];
static bool provider_active[HIGHLIGHT_IMMEDIATES + 1];
static int arrow_draw_count;

static void test_draw_rotated_texture(int x, int y, int height, double angle,
									  texture_t *texture)
{
	(void)x;
	(void)y;
	(void)height;
	(void)angle;
	(void)texture;
	arrow_draw_count++;
}

static bool test_progress(highlight_target_t target, float *progress)
{
	if (progress == NULL || !provider_active[target]) {
		return false;
	}
	*progress = provider_progress[target];
	return true;
}

bool test_instruction_progress(float *progress)
{
	return test_progress(HIGHLIGHT_INSTRUCTION, progress);
}

bool test_code_box_progress(float *progress, float animation_limit)
{
	return animation_limit > 0.0f &&
	       test_progress(HIGHLIGHT_CODE_BOX, progress);
}

bool test_challenge_progress(float *progress, float animation_limit)
{
	return animation_limit > 0.0f &&
	       test_progress(HIGHLIGHT_CHALLENGE, progress);
}

bool test_input_progress(float *progress, float animation_limit)
{
	return animation_limit > 0.0f &&
	       test_progress(HIGHLIGHT_INPUT_BUFFER, progress);
}

bool test_output_progress(float *progress, float animation_limit)
{
	return animation_limit > 0.0f &&
	       test_progress(HIGHLIGHT_OUTPUT_BUFFER, progress);
}

bool test_register_progress(float *progress, float animation_limit)
{
	return animation_limit > 0.0f &&
	       test_progress(HIGHLIGHT_REGISTERS, progress);
}

bool test_button_progress(int button_id, float *progress, float animation_limit)
{
	return animation_limit > 0.0f &&
	       test_progress(button_id == STEP ? HIGHLIGHT_STEP : HIGHLIGHT_FAST,
	                     progress);
}

bool test_immediate_progress(float *progress, float animation_limit)
{
	return animation_limit > 0.0f &&
	       test_progress(HIGHLIGHT_IMMEDIATES, progress);
}

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
	assert(get_motion_progress(&g_arrow_error, 0.0f) == 0.0f);
	assert(g_arrow_error.motion_pulse.value == 0.0f);
	assert(get_motion_progress(&g_arrow_error, -1.0f) == 0.0f);
	assert(g_arrow_error.motion_pulse.value == 0.0f);
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

static void test_arrow_highlight_mapping(void)
{
	static const struct {
		int arrow_id;
		highlight_target_t target;
	} mappings[] = {
		{AR_INS, HIGHLIGHT_INSTRUCTION},
		{AR_INS_MINUS, HIGHLIGHT_INSTRUCTION},
		{AR_DROP, HIGHLIGHT_CODE_BOX},
		{AR_PLAY, HIGHLIGHT_NONE},
		{AR_STEP, HIGHLIGHT_STEP},
		{AR_FAST, HIGHLIGHT_FAST},
		{AR_CODE, HIGHLIGHT_CODE_BOX},
		{AR_DEL, HIGHLIGHT_CODE_BOX},
		{AR_OP2, HIGHLIGHT_CODE_BOX},
		{AR_ERROR, HIGHLIGHT_NONE},
		{AR_CHALLENGE, HIGHLIGHT_CHALLENGE},
		{AR_IB, HIGHLIGHT_INPUT_BUFFER},
		{AR_OB, HIGHLIGHT_OUTPUT_BUFFER},
		{AR_EXEC, HIGHLIGHT_NONE},
		{AR_REG, HIGHLIGHT_REGISTERS},
		{AR_ZF, HIGHLIGHT_NONE},
		{AR_IMM_UP, HIGHLIGHT_IMMEDIATES}
	};
	for (size_t index = 0; index < sizeof(mappings) / sizeof(mappings[0]);
	     index++) {
		assert(g_arrow_highlight_targets[mappings[index].arrow_id] ==
		       mappings[index].target);
		assert(arrow_has_highlight_provider(mappings[index].arrow_id) ==
		       (mappings[index].target != HIGHLIGHT_NONE));
	}
	/* These UI targets have no animated highlight owner in the current game. */
	assert(g_arrow_highlight_targets[AR_PLAY] == HIGHLIGHT_NONE);
	assert(g_arrow_highlight_targets[AR_ERROR] == HIGHLIGHT_NONE);
	assert(g_arrow_highlight_targets[AR_ZF] == HIGHLIGHT_NONE);
	assert(g_arrow_highlight_targets[AR_EXEC] == HIGHLIGHT_NONE);
}

static void test_inactive_mapped_provider_lifecycle(void)
{
	arrow_t arrow = {
		.arrow_id = AR_CODE,
		.box = {.x = 900, .y = 800},
		.startx = 120,
		.starty = 240,
		.travel = 80,
		.dir = AR_UP,
		.visible = true,
		.motion_initialized = false,
		.motion_pulse = {.value = 6.0f, .descending = true}
	};
	provider_active[HIGHLIGHT_CODE_BOX] = false;
	int draws_before = arrow_draw_count;
	animate_arrow(&arrow, 15.0f);
	assert(arrow_draw_count == draws_before);
	assert(arrow.box.x == arrow.startx);
	assert(arrow.box.y == arrow.starty);
	assert(!arrow.motion_initialized);
	assert(arrow.motion_pulse.value == 6.0f);
	assert(arrow.motion_pulse.descending);

	provider_active[HIGHLIGHT_CODE_BOX] = true;
	provider_progress[HIGHLIGHT_CODE_BOX] = 0.75f;
	float same_frame_progress = -1.0f;
	assert(get_highlight_progress(&arrow, &same_frame_progress, 15.0f));
	animate_arrow(&arrow, 15.0f);
	assert(arrow_draw_count == draws_before + 1);
	assert(arrow.box.x == arrow.startx);
	assert(arrow.box.y == arrow.starty - (int)(arrow.travel * same_frame_progress));
	assert(!arrow.motion_initialized);
	assert(arrow.motion_pulse.value == 6.0f);
	assert(arrow.motion_pulse.descending);
	provider_active[HIGHLIGHT_CODE_BOX] = false;
}

static void test_synchronized_arrow_positions(void)
{
	static const int synchronized_arrows[] = {
		AR_INS, AR_INS_MINUS, AR_DROP, AR_STEP, AR_FAST, AR_CODE, AR_DEL,
		AR_OP2, AR_CHALLENGE, AR_IB, AR_OB, AR_REG, AR_IMM_UP
	};
	static const float samples[] = {0.0f, 0.25f, 0.5f, 0.75f, 1.0f};
	static const int directions[] = {AR_UP, AR_DOWN, AR_LEFT, AR_RIGHT};
	for (int target = HIGHLIGHT_INSTRUCTION;
	     target <= HIGHLIGHT_IMMEDIATES; target++) {
		provider_active[target] = true;
		for (size_t index = 0;
		     index < sizeof(synchronized_arrows) / sizeof(synchronized_arrows[0]);
		     index++) {
			int arrow_id = synchronized_arrows[index];
			if (g_arrow_highlight_targets[arrow_id] != target) {
				continue;
			}
			arrow_t arrow = {
				.arrow_id = arrow_id,
				.startx = 100,
				.starty = 200,
				.travel = 120,
				.motion_initialized = false,
								.progress_from_positive_side = arrow_id == AR_INS,
								.highlight_progress_inverted = arrow_id == AR_DEL,
				.motion_pulse = {.value = 4.0f, .descending = true}
			};
			for (size_t direction = 0;
			     direction < sizeof(directions) / sizeof(directions[0]);
			     direction++) {
				arrow.dir = directions[direction];
				for (size_t sample = 0;
				     sample < sizeof(samples) / sizeof(samples[0]); sample++) {
					provider_progress[target] = samples[sample];
					float current = -1.0f;
					assert(get_highlight_progress(&arrow, &current, 15.0f));
					assert(fabsf(current - samples[sample]) < 0.00001f);
					float repeated = -1.0f;
					assert(get_highlight_progress(&arrow, &repeated, 15.0f));
					assert(fabsf(current - repeated) < 0.00001f);
					position_arrow(&arrow, current);
					float geometry_progress = arrow.highlight_progress_inverted ?
					    1.0f - samples[sample] : samples[sample];
					position_arrow(&arrow, geometry_progress);
					int distance = (int)(arrow.travel * geometry_progress);
					if (arrow.dir == AR_UP) {
						assert(arrow.box.y == arrow.starty - distance);
					} else if (arrow.dir == AR_DOWN) {
						assert(arrow.box.y == arrow.starty + distance);
					} else if (arrow.dir == AR_LEFT) {
						int expected_x = arrow.progress_from_positive_side ?
						    arrow.startx + arrow.travel - distance :
						    arrow.startx - distance;
						assert(arrow.box.x == expected_x);
					} else {
						assert(arrow.box.x == arrow.startx + distance);
					}
					assert(!arrow.motion_initialized);
					assert(arrow.motion_pulse.value == 4.0f);
					assert(arrow.motion_pulse.descending);
				}
			}
		}
		provider_active[target] = false;
	}

	arrow_t zero_travel = {
		.arrow_id = AR_DEL,
		.box = {.x = 50, .y = 60},
		.dir = AR_LEFT,
		.startx = 50,
		.starty = 60,
		.travel = 0
	};
	position_arrow(&zero_travel, 1.0f);
	assert(zero_travel.box.x == zero_travel.startx);
	assert(zero_travel.box.y == zero_travel.starty);
	zero_travel.box.x = zero_travel.startx;
	zero_travel.box.y = zero_travel.starty;
	position_arrow(&zero_travel, NAN);
	assert(zero_travel.box.x == zero_travel.startx);
	assert(zero_travel.box.y == zero_travel.starty);
}

static void test_level1_code_box_arrow_regression(void)
{
	assert(tr_load_level(1));
	cw_create_code_list();
	for (int index = 0; index < tr_get_step_count(); index++) {
		const tutorial_step_t *candidate = tr_get_step(index);
		if (candidate != NULL &&
		    strcmp(candidate->name, "select_last_instruction") != 0) {
			tr_deactivate(candidate->name);
		}
	}
	cs_context_t context = {.code_size = 3};
	const tutorial_step_t *step = tr_update(&context);
	assert(step != NULL);
	assert(strcmp(step->name, "select_last_instruction") == 0);
	assert(step->arrow_count == 1);
	assert(step->arrow_ids[0] == AR_CODE);
	assert(step->highlight_targets & TUTORIAL_HIGHLIGHT_CODE_BOX);
	assert(g_arrow_highlight_targets[step->arrow_ids[0]] == HIGHLIGHT_CODE_BOX);

	provider_active[HIGHLIGHT_CODE_BOX] = true;
	arrow_t arrow = {.arrow_id = step->arrow_ids[0], .travel = 120,
	                 .startx = 300, .starty = 400, .dir = AR_UP};
	provider_progress[HIGHLIGHT_CODE_BOX] = 1.0f;
	float progress = 0.0f;
	assert(get_highlight_progress(&arrow, &progress, 15.0f));
	assert(fabsf(progress - 1.0f) < 0.00001f);
	position_arrow(&arrow, progress);
	assert(arrow.box.y == arrow.starty - arrow.travel);
	provider_progress[HIGHLIGHT_CODE_BOX] = 0.0f;
	assert(get_highlight_progress(&arrow, &progress, 15.0f));
	position_arrow(&arrow, progress);
	assert(arrow.box.y == arrow.starty);
	provider_active[HIGHLIGHT_CODE_BOX] = false;
	progress = 0.0f;
	assert(!get_highlight_progress(&arrow, &progress, 15.0f));
	assert(progress == 0.0f);
	assert(tr_update(NULL) == NULL);
	tr_clear();
}

static void test_challenge_arrow_highlight_configuration(void)
{
	assert(tr_load_level(13));
	const tutorial_step_t *step =
		tr_get_named_step("complete_compare_branch_program");
	assert(step != NULL);
	assert(step->arrow_count == 1);
	assert(step->arrow_ids[0] == AR_CHALLENGE);
	assert(step->highlight_targets & TUTORIAL_HIGHLIGHT_CHALLENGE);
	assert(g_arrow_highlight_targets[step->arrow_ids[0]] ==
	       HIGHLIGHT_CHALLENGE);
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
	test_arrow_highlight_mapping();
	test_inactive_mapped_provider_lifecycle();
	test_synchronized_arrow_positions();
	test_level1_code_box_arrow_regression();
	test_challenge_arrow_highlight_configuration();
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