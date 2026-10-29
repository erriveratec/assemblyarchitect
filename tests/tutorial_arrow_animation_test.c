#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <SDL_ttf.h>
#include "game_mechanics_mc.h"
#include "sdl_config.h"
#include "tutorial_tr.h"
#include "tutorial_tr_internal.h"
#include "code_window_cw.h"
#include "code_window_cw_internal.h"
#include "registers_rg.h"
#include "stage_buttons_sb.h"
#include "gameplay/ui_highlight_source.h"
#include "gameplay/highlight_animation_ha.h"

static void test_draw_rotated_texture(int x, int y, int height, double angle,
									  texture_t *texture);
SDL_Rect test_stage_buttons_rect(void);
SDL_Rect test_stage_button_rect(int member);
int test_code_list_size(void);
SDL_Rect test_code_line_coord_at_pos(int element, int position);
bool test_failure_back_button_rect(SDL_Rect *button_rect);
bool test_instruction_progress(float *progress);
bool test_code_box_progress(float *progress, float animation_limit);
bool test_challenge_progress(float *progress, float animation_limit);
bool test_input_progress(float *progress, float animation_limit);
bool test_output_progress(float *progress, float animation_limit);
bool test_register_progress(float *progress, float animation_limit);
bool test_zero_flag_progress(float *progress, float animation_limit);
bool test_button_progress(int button_id, float *progress,
	                      float animation_limit);
bool test_immediate_progress(float *progress, float animation_limit);
#define iw_get_highlight_instruction_progress test_instruction_progress
#define cw_get_code_box_highlight_progress test_code_box_progress
#define cw_get_code_list_size test_code_list_size
#define cw_get_code_line_coord_at_pos test_code_line_coord_at_pos
#define cw_get_challenge_highlight_progress test_challenge_progress
#define bf_get_input_buffer_highlight_progress test_input_progress
#define bf_get_output_buffer_highlight_progress test_output_progress
#define rg_get_register_highlight_progress test_register_progress
#define rg_get_zero_flag_highlight_progress test_zero_flag_progress
#define sb_get_button_highlight_progress test_button_progress
#define im_get_highlight_progress test_immediate_progress
#define rr_get_failure_back_button_rect test_failure_back_button_rect
#define sb_get_stage_btns test_stage_buttons_rect
#define sb_get_sb_rect test_stage_button_rect
#define dw_draw_rotated_texture_fits_h test_draw_rotated_texture
#include "../source/arrow_ar.c"
#undef dw_draw_rotated_texture_fits_h
#undef sb_get_sb_rect
#undef sb_get_stage_btns
#undef rr_get_failure_back_button_rect
#undef im_get_highlight_progress
#undef sb_get_button_highlight_progress
#undef rg_get_register_highlight_progress
#undef rg_get_zero_flag_highlight_progress
#undef bf_get_output_buffer_highlight_progress
#undef bf_get_input_buffer_highlight_progress
#undef cw_get_challenge_highlight_progress
#undef cw_get_code_line_coord_at_pos
#undef cw_get_code_list_size
#undef cw_get_code_box_highlight_progress
#undef iw_get_highlight_instruction_progress

bool bf_get_output_buffer_highlight_progress(float *progress, float animation_limit);
bool rg_get_register_highlight_progress(float *progress, float animation_limit);
bool rg_get_zero_flag_highlight_progress(float *progress, float animation_limit);

static float provider_progress[HIGHLIGHT_ZERO_FLAG + 1];
static bool provider_active[HIGHLIGHT_ZERO_FLAG + 1];
static bool use_shared_output_provider;
static int arrow_draw_count;
static int code_box_provider_calls;
static SDL_Rect op2_line_rect;
static texture_t g_test_register_arrow;

static bool load_fixture(const char *fixture_name, const char *text)
{
	FILE *stream = tmpfile();
	assert(stream != NULL);
	size_t length = strlen(text);
	assert(fwrite(text, 1, length, stream) == length);
	rewind(stream);
	bool loaded = tr_load_tutorial_stream_for_testing(stream, fixture_name);
	fclose(stream);
	return loaded;
}

static void assert_fixture_rejected(const char *fixture_name, const char *text)
{
	assert(!load_fixture(fixture_name, text));
	assert(tr_get_step_count() == 0);
	assert(tr_get_step(0) == NULL);
	float progress = -1.0f;
	assert(!rg_get_zero_flag_highlight_progress(
	    &progress, (float)dw_get_animation_limit()));
}

static void test_challenge_frame_geometry_helper(void)
{
	const SDL_Rect container = {.x = 100, .y = 120, .w = 300, .h = 240};
	const SDL_Rect text = {.x = 160, .y = 170, .w = 80, .h = 40};
	SDL_Rect frame = {0};
	assert(cw_calculate_challenge_frame_rect(text, container, 5, &frame));
	assert(frame.x == 155 && frame.y == 165 && frame.w == 90 && frame.h == 50);
	assert(cw_calculate_challenge_frame_rect(text, container, 0, &frame));
	assert(memcmp(&frame, &text, sizeof(frame)) == 0);

	assert(cw_calculate_challenge_frame_rect(
	    (SDL_Rect){.x = 102, .y = 150, .w = 50, .h = 40},
	    container, 5, &frame));
	assert(frame.x == container.x);
	assert(cw_calculate_challenge_frame_rect(
	    (SDL_Rect){.x = 345, .y = 150, .w = 50, .h = 40},
	    container, 5, &frame));
	assert(frame.x + frame.w == container.x + container.w);
	assert(cw_calculate_challenge_frame_rect(
	    (SDL_Rect){.x = 150, .y = 122, .w = 50, .h = 40},
	    container, 5, &frame));
	assert(frame.y == container.y);
	assert(cw_calculate_challenge_frame_rect(
	    (SDL_Rect){.x = 150, .y = 315, .w = 50, .h = 40},
	    container, 5, &frame));
	assert(frame.y + frame.h == container.y + container.h);

	SDL_Rect sentinel = {.x = 7, .y = 11, .w = 13, .h = 17};
	frame = sentinel;
	assert(!cw_calculate_challenge_frame_rect(text, container, -1, &frame));
	assert(memcmp(&frame, &sentinel, sizeof(frame)) == 0);
	assert(!cw_calculate_challenge_frame_rect(text, container, 0, NULL));
	assert(!cw_calculate_challenge_frame_rect(
	    (SDL_Rect){.x = 0, .y = 0, .w = 0, .h = 5}, container, 0, &frame));
	assert(memcmp(&frame, &sentinel, sizeof(frame)) == 0);
	assert(!cw_calculate_challenge_frame_rect(
	    text, (SDL_Rect){.x = 0, .y = 0, .w = -1, .h = 5}, 0, &frame));
	assert(memcmp(&frame, &sentinel, sizeof(frame)) == 0);
	assert(!cw_calculate_challenge_frame_rect(
	    (SDL_Rect){.x = 200, .y = 200, .w = 20, .h = 20},
	    (SDL_Rect){.x = 0, .y = 0, .w = 1, .h = 1}, 0, &frame));
	assert(memcmp(&frame, &sentinel, sizeof(frame)) == 0);
	assert(!cw_calculate_challenge_frame_rect(
	    (SDL_Rect){.x = INT_MAX - 2, .y = 10, .w = 10, .h = 20},
	    (SDL_Rect){.x = INT_MIN, .y = 0, .w = INT_MAX, .h = 100}, 5, &frame));
	assert(memcmp(&frame, &sentinel, sizeof(frame)) == 0);
}

SDL_Rect test_stage_buttons_rect(void)
{
	return (SDL_Rect){.x = 300, .y = 500, .w = 600, .h = 80};
}

SDL_Rect test_stage_button_rect(int member)
{
	assert(member == PLAY);
	return (SDL_Rect){.x = 520, .y = 400, .w = 160, .h = 60};
}

static const float TEST_PROGRESS_EPSILON = 0.00001f;

static void assert_progress_equal(float actual, float expected)
{
	assert(isfinite(actual));
	assert(isfinite(expected));
	assert(fabsf(actual - expected) <= TEST_PROGRESS_EPSILON);
}

int test_code_list_size(void)
{
	return 2;
}

SDL_Rect test_code_line_coord_at_pos(int element, int position)
{
	assert(position == 1);
	(void)element;
	return op2_line_rect;
}

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
	code_box_provider_calls++;
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
	if (use_shared_output_provider) {
		return bf_get_output_buffer_highlight_progress(progress, animation_limit);
	}
	return animation_limit > 0.0f &&
	       test_progress(HIGHLIGHT_OUTPUT_BUFFER, progress);
}

bool test_register_progress(float *progress, float animation_limit)
{
	return animation_limit > 0.0f &&
	       test_progress(HIGHLIGHT_REGISTERS, progress);
}

bool test_zero_flag_progress(float *progress, float animation_limit)
{
	return animation_limit > 0.0f &&
	       test_progress(HIGHLIGHT_ZERO_FLAG, progress);
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
static int back_button_offset;

bool test_failure_back_button_rect(SDL_Rect *button_rect)
{
	*button_rect = (SDL_Rect){
		.x = dm_scale_to_res(800 + back_button_offset),
		.y = dm_scale_to_res(500 + back_button_offset),
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
	assert(!g_arrow_error.highlight_progress_inverted);
	assert(g_arrow_error.travel == size.w);
	assert(g_arrow_error.startx + g_arrow_error.travel + size.w == back_button.x);
	assert(g_arrow_error.box.x == g_arrow_error.startx);
	assert(g_arrow_error.starty == back_button.y + back_button.h / 2 - size.h / 2);
	assert(g_arrow_error.box.y == g_arrow_error.starty);
	assert(g_arrow_error.box.w == size.w);
	assert(g_arrow_error.box.h == size.h);
	assert(!g_arrow_error.motion_initialized);
	assert(g_arrow_error.motion_pulse.value == 0.0f);
	assert(!g_arrow_error.motion_pulse.descending);
}

static void assert_error_position(float progress, float limit)
{
	SDL_Rect back_button;
	assert(test_failure_back_button_rect(&back_button));
	float sampled = -1.0f;
	assert(get_highlight_progress(&g_arrow_error, &sampled, limit));
	assert_progress_equal(sampled, progress);
	int draws_before = arrow_draw_count;
	ar_display_arrow(AR_ERROR, limit);
	assert(arrow_draw_count == draws_before + 1);
	assert(g_arrow_error.box.x ==
	       g_arrow_error.startx + (int)(g_arrow_error.travel * progress));
	assert(g_arrow_error.box.y ==
	       back_button.y + back_button.h / 2 - g_arrow_error.box.h / 2);
	assert(!g_arrow_error.highlight_progress_inverted);
	assert(!g_arrow_error.motion_initialized);
	assert(g_arrow_error.motion_pulse.value == 0.0f);
	assert(!g_arrow_error.motion_pulse.descending);
	if (progress == 0.0f) {
		assert(g_arrow_error.box.x == g_arrow_error.startx);
	} else if (progress == 1.0f) {
		assert(g_arrow_error.box.x == back_button.x - g_arrow_error.box.w);
	}
}

static void test_error_arrow_tracks_output_highlight(void)
{
	static const float samples[] = {0.0f, 0.25f, 0.5f, 0.75f, 1.0f};
	dm_set_screen_resolution(DM_RESOLUTION_1600X900);
	const float limits[] = {dw_get_animation_limit(), 7.3f};
	provider_active[HIGHLIGHT_OUTPUT_BUFFER] = true;
	for (size_t index = 0; index < sizeof(limits) / sizeof(limits[0]); index++) {
		ar_init_arrow(AR_ERROR);
		assert_reset();
		for (size_t sample = 0; sample < sizeof(samples) / sizeof(samples[0]); sample++) {
			provider_progress[HIGHLIGHT_OUTPUT_BUFFER] = samples[sample];
			assert_error_position(samples[sample], limits[index]);
		}
	}
	provider_active[HIGHLIGHT_OUTPUT_BUFFER] = false;
}

static void test_resolution(dm_resolution_t resolution)
{
	dm_set_screen_resolution(resolution);
	float limits[] = {dw_get_animation_limit(), 7.3f};
	ar_init_arrow(AR_ERROR);
	assert_reset();
	provider_active[HIGHLIGHT_OUTPUT_BUFFER] = true;
	provider_progress[HIGHLIGHT_OUTPUT_BUFFER] = 1.0f;
	for (unsigned int index = 0; index < sizeof(limits) / sizeof(limits[0]); index++) {
		assert_error_position(1.0f, limits[index]);
		g_arrow_error.visible = false;
		float hidden_value = g_arrow_error.motion_pulse.value;
		int draws_before = arrow_draw_count;
		ar_display_arrow(AR_ERROR, limits[index]);
		assert(arrow_draw_count == draws_before);
		assert(g_arrow_error.motion_pulse.value == hidden_value);
		ar_init_arrow(AR_ERROR);
		assert_reset();
	}
	int draws_before = arrow_draw_count;
	ar_display_arrow(AR_ERROR, 0.0f);
	ar_display_arrow(AR_ERROR, -1.0f);
	assert(arrow_draw_count == draws_before);
	assert(!g_arrow_error.motion_initialized);
	assert(g_arrow_error.motion_pulse.value == 0.0f);
	provider_active[HIGHLIGHT_OUTPUT_BUFFER] = false;
	layout_available = false;
	ar_init_arrow(AR_ERROR);
	assert(!g_arrow_error.visible);
	layout_available = true;
	ar_init_arrow(AR_ERROR);
	assert_reset();
}

static void test_tutorial_reentry(void)
{
	static const char fixture[] =
	    "[tutorial]\nformat = 1\n"
	    "# fixture comment\n"
	    "[zero_flag_case]\nbox = center\nheader = none\ndismiss = none\n"
	    " arrow = zf \nhighlight = zero_flag\nwhen.code_size = 0\n"
	    "text_begin\nControlled ZF fixture\ntext_end\n"
	    "[play_case]\nbox = upper\nheader = none\ndismiss = none\n"
	    "arrow = play\nhighlight = code_box\nwhen.code_size = 1\n"
	    "text_begin\nControlled play fixture\ntext_end\n"
	    "[plain_case]\nbox = lower\nheader = none\ndismiss = none\n"
	    "when.code_size = 2\ntext_begin\nText-only fixture\ntext_end\n";
	assert(load_fixture("arrow-runtime-fixture", fixture));
	assert(tr_get_step_count() == 3);
	const tutorial_step_t *zf_step = tr_get_named_step("zero_flag_case");
	const tutorial_step_t *play_step = tr_get_named_step("play_case");
	const tutorial_step_t *plain_step = tr_get_named_step("plain_case");
	assert(zf_step != NULL && zf_step->arrow_count == 1 &&
	       zf_step->arrow_ids[0] == AR_ZF);
	assert(play_step != NULL && play_step->arrow_count == 1 &&
	       play_step->arrow_ids[0] == AR_PLAY);
	assert(plain_step != NULL && plain_step->arrow_count == 0);
	assert(plain_step->highlight_targets == 0);

	ha_reset();
	rg_set_zero_flag_highlight_source(UI_HIGHLIGHT_SOURCE_AVAILABILITY, true);
	cs_context_t context = {.code_size = 0, .operation_flag = NO_OPERATION};
	assert(tr_update(&context) == zf_step);
	assert(g_arrow_zf.arrow_id == AR_ZF && g_arrow_zf.visible);
	float progress = -1.0f;
	float limit = (float)dw_get_animation_limit();
	assert(rg_get_zero_flag_highlight_progress(&progress, limit));
	assert(!cw_get_code_box_highlight_progress(&progress, limit));
	assert(!rg_get_register_highlight_progress(&progress, limit));
	assert(!sb_get_button_highlight_progress(STEP, &progress, limit));
	provider_active[HIGHLIGHT_ZERO_FLAG] = true;
	provider_progress[HIGHLIGHT_ZERO_FLAG] = 0.5f;
	int draws_before = arrow_draw_count;
	tr_render_step("zero_flag_case");
	assert(arrow_draw_count == draws_before + 1);
	for (int frame = 0; frame < 4 * (dw_get_animation_limit() +
	                                  dw_get_animation_step() - 1) /
	                         dw_get_animation_step(); frame++) {
		ha_begin_frame();
		assert(ha_get_progress() >= 0.0f && ha_get_progress() <= 1.0f);
		assert(ha_get_value(limit) >= 0.0f && ha_get_value(limit) <= limit);
	}
	g_arrow_zf.motion_pulse.value = 6.25f;
	assert(tr_update(&context) == zf_step);
	assert(g_arrow_zf.motion_pulse.value == 6.25f);

	context.code_size = 2;
	assert(tr_update(&context) == plain_step);
	assert(rg_get_zero_flag_highlight_progress(&progress, limit));
	draws_before = arrow_draw_count;
	tr_render_step("plain_after_arrow");
	assert(arrow_draw_count == draws_before);
	rg_set_zero_flag_highlight_source(UI_HIGHLIGHT_SOURCE_AVAILABILITY, false);
	assert(!rg_get_zero_flag_highlight_progress(&progress, limit));
	assert(!cw_get_code_box_highlight_progress(&progress, limit));
	assert(!rg_get_register_highlight_progress(&progress, limit));

	context.code_size = 0;
	assert(tr_update(&context) == zf_step);
	context.code_size = 1;
	assert(tr_update(&context) == play_step);
	assert(g_arrow_play.arrow_id == AR_PLAY && g_arrow_play.visible);
	assert(cw_get_code_box_highlight_progress(&progress, limit));
	assert(!rg_get_zero_flag_highlight_progress(&progress, limit));
	context.code_size = 2;
	assert(tr_update(&context) == plain_step);
	assert(!cw_get_code_box_highlight_progress(&progress, limit));
	assert(tr_update(NULL) == NULL);
	assert(!rg_get_zero_flag_highlight_progress(&progress, limit));
	assert(!sb_get_button_highlight_progress(STEP, &progress, limit));
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
		{AR_PLAY, HIGHLIGHT_CODE_BOX},
		{AR_STEP, HIGHLIGHT_STEP},
		{AR_FAST, HIGHLIGHT_FAST},
		{AR_CODE, HIGHLIGHT_CODE_BOX},
		{AR_DEL, HIGHLIGHT_CODE_BOX},
		{AR_OP2, HIGHLIGHT_CODE_BOX},
		{AR_ERROR, HIGHLIGHT_OUTPUT_BUFFER},
		{AR_CHALLENGE, HIGHLIGHT_CHALLENGE},
		{AR_IB, HIGHLIGHT_INPUT_BUFFER},
		{AR_OB, HIGHLIGHT_OUTPUT_BUFFER},
		{AR_EXEC, HIGHLIGHT_NONE},
		{AR_REG, HIGHLIGHT_REGISTERS},
		{AR_ZF, HIGHLIGHT_ZERO_FLAG},
		{AR_IMM_UP, HIGHLIGHT_IMMEDIATES}
	};
	for (size_t index = 0; index < sizeof(mappings) / sizeof(mappings[0]);
	     index++) {
		assert(g_arrow_highlight_targets[mappings[index].arrow_id] ==
		       mappings[index].target);
		assert(arrow_has_highlight_provider(mappings[index].arrow_id) ==
		       (mappings[index].target != HIGHLIGHT_NONE));
	}
	/* PLAY can use shared timing without a visible highlight; EXEC uses private timing. */
	assert(g_arrow_highlight_targets[AR_PLAY] == HIGHLIGHT_CODE_BOX);
	assert(arrow_has_highlight_provider(AR_PLAY));
	assert(g_arrow_highlight_targets[AR_ZF] == HIGHLIGHT_ZERO_FLAG);
	assert(g_arrow_highlight_targets[AR_EXEC] == HIGHLIGHT_NONE);
}

static void render_play_arrow_at_progress(bool highlight_active,
												  float provider_phase,
												  float expected_phase,
												  SDL_Rect *result)
{
	arrow_t arrow = {
		.arrow_id = AR_PLAY,
		.box = {.x = 120, .y = 820, .w = 45, .h = 45},
		.startx = 120,
		.starty = 240,
		.travel = 80,
		.dir = AR_DOWN,
		.visible = true,
		.highlight_progress_inverted = true,
		.motion_initialized = false,
		.motion_pulse = {.value = 6.25f, .descending = true}
	};
	provider_active[HIGHLIGHT_CODE_BOX] = highlight_active;
	provider_progress[HIGHLIGHT_CODE_BOX] = provider_phase;
	int draws_before = arrow_draw_count;
	code_box_provider_calls = 0;
	animate_arrow(&arrow, 15.0f);
	assert(arrow_draw_count == draws_before + 1);
	assert(code_box_provider_calls == 1);
	assert(arrow.box.x == arrow.startx);
	assert(arrow.box.y ==
	       arrow.starty + (int)(arrow.travel * expected_phase));
	assert(!arrow.motion_initialized);
	assert(arrow.motion_pulse.value == 6.25f);
	assert(arrow.motion_pulse.descending);
	if (result != NULL) {
		*result = arrow.box;
	}
}

static void test_play_arrow_uses_shared_phase_with_active_provider(void)
{
	static const float samples[] = {0.0f, 0.25f, 0.5f, 0.75f, 1.0f};
	for (size_t index = 0; index < sizeof(samples) / sizeof(samples[0]); index++) {
		SDL_Rect rect;
		render_play_arrow_at_progress(true, samples[index], samples[index],
		                              &rect);
		assert(rect.y == 240 + (int)(80 * samples[index]));
		if (samples[index] == 0.0f) {
			assert(rect.y == 240);
		} else if (samples[index] == 1.0f) {
			assert(rect.y == 240 + 80);
		}
	}
}

static void test_play_arrow_position_is_independent_of_highlight_visibility(void)
{
	ha_reset();
	provider_active[HIGHLIGHT_CODE_BOX] = false;
	int upper_endpoints = 0;
	int lower_endpoints = 0;
	bool saw_descending = false;
	bool saw_ascending_again = false;
	float previous = ha_get_progress();
	int frame_budget = 4 * (dw_get_animation_limit() +
	                         dw_get_animation_step() - 1) /
	                    dw_get_animation_step();
	for (int frame = 0; frame <= frame_budget; frame++) {
		ha_begin_frame();
		float phase = ha_get_progress();
		SDL_Rect active_rect;
		SDL_Rect inactive_rect;
		render_play_arrow_at_progress(true, phase, phase, &active_rect);
		render_play_arrow_at_progress(false, phase, phase, &inactive_rect);
		assert(active_rect.x == inactive_rect.x);
		assert(active_rect.y == inactive_rect.y);
		if (phase == 1.0f) {
			assert(active_rect.y == 240 + 80);
			if (previous < 1.0f) {
				upper_endpoints++;
			}
		}
		if (phase == 0.0f) {
			assert(active_rect.y == 240);
			if (frame > 0 && previous > 0.0f) {
				lower_endpoints++;
			}
		}
		if (phase < previous) {
			saw_descending = true;
			assert(ha_is_descending());
		} else if (saw_descending && phase > previous) {
			saw_ascending_again = true;
			assert(!ha_is_descending());
		}
		previous = phase;
	}
	assert(upper_endpoints > 0);
	assert(lower_endpoints > 0);
	assert(saw_descending && saw_ascending_again);

	provider_active[HIGHLIGHT_CODE_BOX] = false;
}

static void test_play_arrow_activation_transition_is_continuous(void)
{
	ha_reset();
	ha_begin_frame();
	ha_begin_frame();
	float phase = ha_get_progress();
	assert(phase > 0.0f && phase < 1.0f);
	SDL_Rect inactive_rect;
	SDL_Rect active_rect;
	render_play_arrow_at_progress(false, phase, phase, &inactive_rect);
	render_play_arrow_at_progress(true, phase, phase, &active_rect);
	assert(inactive_rect.x == active_rect.x);
	assert(inactive_rect.y == active_rect.y);
	assert(ha_get_progress() == phase);
}

static void test_play_arrow_deactivation_transition_is_continuous(void)
{
	ha_reset();
	ha_begin_frame();
	ha_begin_frame();
	float phase = ha_get_progress();
	assert(phase > 0.0f && phase < 1.0f);
	SDL_Rect active_rect;
	SDL_Rect inactive_rect;
	render_play_arrow_at_progress(true, phase, phase, &active_rect);
	render_play_arrow_at_progress(false, phase, phase, &inactive_rect);
	assert(active_rect.x == inactive_rect.x);
	assert(active_rect.y == inactive_rect.y);
	assert(ha_get_progress() == phase);
}

static void test_play_arrow_does_not_mutate_private_motion(void)
{
	static const bool provider_states[] = {true, false};
	for (size_t index = 0;
	     index < sizeof(provider_states) / sizeof(provider_states[0]); index++) {
		float phase = provider_states[index] ? 0.625f : ha_get_progress();
		SDL_Rect rect;
		render_play_arrow_at_progress(provider_states[index], 0.625f, phase,
		                              &rect);
	}
}

static void test_play_arrow_initialization_policy(void)
{
	dm_set_screen_resolution(DM_RESOLUTION_1600X900);
	ar_init_arrow(AR_PLAY);
	assert(g_arrow_play.arrow_id == AR_PLAY);
	assert(g_arrow_play.dir == AR_DOWN);
	assert(g_arrow_play.travel > 0);
	assert(!g_arrow_play.progress_from_positive_side);
	assert(!g_arrow_play.highlight_progress_inverted);
	assert(!g_arrow_play.motion_initialized);
}

static void test_independent_arrow_retains_private_motion(void)
{
	arrow_t arrow = {
		.arrow_id = AR_EXEC,
		.box = {.x = 120, .y = 240, .w = 45, .h = 45},
		.startx = 120,
		.starty = 240,
		.travel = 80,
		.dir = AR_DOWN,
		.visible = true,
		.motion_initialized = false,
		.motion_pulse = {.value = 0.0f, .descending = false}
	};
	int draws_before = arrow_draw_count;
	animate_arrow(&arrow, 15.0f);
	assert(arrow_draw_count == draws_before + 1);
	assert(arrow.motion_initialized);
	assert(arrow.motion_pulse.value > 0.0f);
}

static void assert_mandatory_provider_behavior(int arrow_id,
													  highlight_target_t target)
{
	arrow_t arrow = {
		.arrow_id = arrow_id,
		.box = {.x = 900, .y = 800},
		.startx = 120,
		.starty = 240,
		.travel = 80,
		.dir = AR_UP,
		.visible = true,
		.motion_initialized = false,
		.motion_pulse = {.value = 6.0f, .descending = true}
	};
	provider_active[target] = false;
	int draws_before = arrow_draw_count;
	animate_arrow(&arrow, 15.0f);
	assert(arrow_draw_count == draws_before);
	assert(arrow.box.x == arrow.startx);
	assert(arrow.box.y == arrow.starty);
	assert(!arrow.motion_initialized);
	assert(arrow.motion_pulse.value == 6.0f);
	assert(arrow.motion_pulse.descending);

	provider_active[target] = true;
	provider_progress[target] = 0.75f;
	float same_frame_progress = -1.0f;
	assert(get_highlight_progress(&arrow, &same_frame_progress, 15.0f));
	animate_arrow(&arrow, 15.0f);
	assert(arrow_draw_count == draws_before + 1);
	assert(arrow.box.x == arrow.startx);
	assert(arrow.box.y == arrow.starty - (int)(arrow.travel * same_frame_progress));
	assert(!arrow.motion_initialized);
	assert(arrow.motion_pulse.value == 6.0f);
	assert(arrow.motion_pulse.descending);
	provider_active[target] = false;
}

static void test_mandatory_provider_arrow_retains_existing_behavior(void)
{
	assert_mandatory_provider_behavior(AR_CODE, HIGHLIGHT_CODE_BOX);
	assert_mandatory_provider_behavior(AR_ERROR, HIGHLIGHT_OUTPUT_BUFFER);
	assert_mandatory_provider_behavior(AR_ZF, HIGHLIGHT_ZERO_FLAG);
}

static void test_synchronized_arrow_positions(void)
{
	static const int synchronized_arrows[] = {
		AR_INS, AR_INS_MINUS, AR_DROP, AR_STEP, AR_FAST, AR_CODE, AR_DEL,
		AR_OP2, AR_ERROR, AR_CHALLENGE, AR_IB, AR_OB, AR_REG, AR_IMM_UP, AR_ZF
	};
	static const float samples[] = {0.0f, 0.25f, 0.5f, 0.75f, 1.0f};
	static const int directions[] = {AR_UP, AR_DOWN, AR_LEFT, AR_RIGHT};
	for (highlight_target_t target = HIGHLIGHT_INSTRUCTION;
	     target <= HIGHLIGHT_ZERO_FLAG; target++) {
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

static void test_op2_vertical_clearance(void)
{
	static const dm_resolution_t resolutions[] = {
		DM_RESOLUTION_1600X900,
		DM_RESOLUTION_1920X1080
	};
	static const float samples[] = {0.0f, 0.25f, 0.5f, 0.75f, 1.0f};
	op2_line_rect = (SDL_Rect){.x = 423, .y = 504, .w = 90, .h = 40};

	for (size_t resolution = 0;
	     resolution < sizeof(resolutions) / sizeof(resolutions[0]);
	     resolution++) {
		dm_set_screen_resolution(resolutions[resolution]);
		SDL_Rect arrow_size = ar_get_arrow_wh();
		int original_y = op2_line_rect.y + op2_line_rect.h / 2;
		int expected_clearance =
			dm_scale_to_res(OP2_ARROW_VERTICAL_CLEARANCE_DESIGN_PX);
		int expected_start_y = original_y + expected_clearance;

		ar_init_arrow(AR_OP2);
		assert(g_arrow_op2.starty == expected_start_y);
		assert(g_arrow_op2.box.y == expected_start_y);
		assert(g_arrow_op2.travel == arrow_size.h);
		assert(g_arrow_op2.dir == AR_UP);
		assert(g_arrow_op2.visible);
		assert(!g_arrow_op2.highlight_progress_inverted);

		for (size_t sample = 0;
		     sample < sizeof(samples) / sizeof(samples[0]); sample++) {
			int expected_original_y =
				original_y - (int)(arrow_size.h * samples[sample]);
			int expected_corrected_y = expected_original_y + expected_clearance;
			position_arrow(&g_arrow_op2, samples[sample]);
			assert(g_arrow_op2.box.y == expected_corrected_y);
		}
	}
}

static uint64_t tutorial_structure_signature(void)
{
	uint64_t sum = 0;
	uint64_t mixed_xor = 0;
	for (int index = 0; index < tr_get_step_count(); index++) {
		const tutorial_step_t *step = tr_get_step(index);
		uint64_t hash = UINT64_C(1469598103934665603);
		for (const unsigned char *cursor =
	         (const unsigned char *)step->name; *cursor != '\0'; cursor++) {
			hash = (hash ^ *cursor) * UINT64_C(1099511628211);
		}
		hash = (hash ^ (uint64_t)step->arrow_count) *
		       UINT64_C(1099511628211);
		for (int arrow = 0; arrow < step->arrow_count; arrow++) {
			hash = (hash ^ (uint64_t)step->arrow_ids[arrow]) *
			       UINT64_C(1099511628211);
		}
		hash = (hash ^ (uint64_t)step->highlight_targets) *
		       UINT64_C(1099511628211);
		sum += hash;
		mixed_xor ^= hash;
	}
	return sum ^ mixed_xor ^ (uint64_t)tr_get_step_count();
}

static void test_tutorial_arrow_parser_fixtures(void)
{
	static const struct {
		const char *token;
		int expected_arrow;
	} mappings[] = {
		{"zf", AR_ZF},
		{"zero_flag", AR_ZF},
		{"play", AR_PLAY},
		{"instruction", AR_INS},
		{"output_buffer", AR_OB}
	};
	char fixture[512];
	for (size_t index = 0; index < sizeof(mappings) / sizeof(mappings[0]);
	     index++) {
		int length = snprintf(fixture, sizeof(fixture),
		    "[tutorial]\nformat = 1\n[case]\nbox = center\nheader = none\n"
		    "dismiss = none\narrow = %s\ntext_begin\nFixture\ntext_end\n",
		    mappings[index].token);
		assert(length > 0 && (size_t)length < sizeof(fixture));
		assert(load_fixture("arrow-token-mapping", fixture));
		const tutorial_step_t *step = tr_get_named_step("case");
		assert(step != NULL && step->arrow_count == 1);
		assert(step->arrow_ids[0] == mappings[index].expected_arrow);
		tr_clear();
	}

	static const char reordered[] =
	    "[tutorial]\nformat = 1\n"
	    "[text_only]\nbox = center\nheader = none\ndismiss = none\n"
	    "text_begin\nText fixture\ntext_end\n"
	    "[plain_after_arrow]\nbox = center\nheader = none\ndismiss = none\n"
	    "text_begin\nNo arrow fixture\ntext_end\n"
	    "[arrow_step]\nbox = center\nheader = none\ndismiss = none\n"
	    "arrow = zf\ntext_begin\nArrow fixture\ntext_end\n";
	assert(load_fixture("arrow-order-reversed", reordered));
	assert(tr_get_step_count() == 3);
	assert(tr_get_named_step("text_only")->arrow_count == 0);
	assert(tr_get_named_step("plain_after_arrow")->arrow_count == 0);
	assert(tr_get_named_step("arrow_step")->arrow_count == 1);
	assert(tr_get_named_step("arrow_step")->arrow_ids[0] == AR_ZF);
	tr_clear();
	assert(load_fixture("arrow-order-original",
	    "[tutorial]\nformat = 1\n[arrow_step]\nbox = center\nheader = none\n"
	    "dismiss = none\narrow = zf\ntext_begin\nArrow fixture\ntext_end\n"
	    "[plain_after_arrow]\nbox = center\nheader = none\ndismiss = none\n"
	    "text_begin\nNo arrow fixture\ntext_end\n"));
	assert(tr_get_named_step("plain_after_arrow")->arrow_count == 0);
	assert(tr_get_named_step("arrow_step")->arrow_ids[0] == AR_ZF);
	tr_clear();
	assert(load_fixture("empty-arrow-tutorial",
	    "[tutorial]\nformat = 1\n[plain]\nbox = center\nheader = none\n"
	    "dismiss = none\ntext_begin\nNo arrows here\ntext_end\n"));
	assert(tr_get_step_count() == 1);
	assert(tr_get_named_step("plain")->arrow_count == 0);
	tr_clear();

	assert_fixture_rejected("unknown-arrow",
	    "[tutorial]\nformat = 1\n[bad]\narrow = output_bufr\n"
	    "text_begin\nBad\ntext_end\n");
	assert_fixture_rejected("empty-arrow",
	    "[tutorial]\nformat = 1\n[bad]\narrow = \ntext_begin\nBad\ntext_end\n");
	assert_fixture_rejected("duplicate-arrow",
	    "[tutorial]\nformat = 1\n[bad]\narrow = zf\narrow = play\n"
	    "text_begin\nBad\ntext_end\n");
	assert_fixture_rejected("malformed-section",
	    "[tutorial]\nformat = 1\n[bad\ntext_begin\nBad\ntext_end\n");
	assert_fixture_rejected("truncated-text",
	    "[tutorial]\nformat = 1\n[partial]\ntext_begin\nnever closed\n");
	assert_fixture_rejected("partial-parse-cleanup",
	    "[tutorial]\nformat = 1\n[valid_first]\nbox = center\nheader = none\n"
	    "text_begin\nAllocated before failure\ntext_end\n[invalid_second]\n"
	    "arrow = unknown\ntext_begin\nFailure\ntext_end\n");
	char long_fixture[512];
	char long_token[180];
	memset(long_token, 'a', sizeof(long_token) - 1);
	long_token[sizeof(long_token) - 1] = '\0';
	int length = snprintf(long_fixture, sizeof(long_fixture),
	    "[tutorial]\nformat = 1\n[bad]\narrow = %s\ntext_begin\nBad\ntext_end\n",
	    long_token);
	assert(length > 0 && (size_t)length < sizeof(long_fixture));
	assert_fixture_rejected("long-arrow-token", long_fixture);
}

static void test_all_tutorial_arrow_ids(void)
{
	DIR *levels = opendir("data/levels");
	assert(levels != NULL);
	int level_ids[128];
	size_t level_count = 0;
	struct dirent *entry;
	while ((entry = readdir(levels)) != NULL) {
		char *end = NULL;
		errno = 0;
		long id = strtol(entry->d_name, &end, 10);
		if (errno != 0 || end == entry->d_name || *end != '\0' ||
		    id < 0 || id > INT_MAX || level_count ==
		    sizeof(level_ids) / sizeof(level_ids[0])) {
			continue;
		}
		char tutorial_path[256];
		struct stat file_info;
		snprintf(tutorial_path, sizeof(tutorial_path),
		         "data/levels/%02ld/tutorial.cfg", id);
		if (stat(tutorial_path, &file_info) == 0 && S_ISREG(file_info.st_mode)) {
			level_ids[level_count++] = (int)id;
		}
	}
	closedir(levels);
	for (size_t left = 0; left < level_count; left++) {
		for (size_t right = left + 1; right < level_count; right++) {
			if (level_ids[right] < level_ids[left]) {
				int temporary = level_ids[left];
				level_ids[left] = level_ids[right];
				level_ids[right] = temporary;
			}
		}
	}
	assert(level_count > 0);
	for (size_t level_index = 0; level_index < level_count; level_index++) {
		int level_id = level_ids[level_index];
		assert(tr_load_level(level_id));
		uint64_t first_signature = tutorial_structure_signature();
		for (int index = 0; index < tr_get_step_count(); index++) {
			const tutorial_step_t *step = tr_get_step(index);
			assert(step != NULL);
			assert(step->name[0] != '\0');
			assert(step->text_message != NULL);
			assert(step->box >= TUTORIAL_BOX_CENTER_UP &&
			       step->box <= TUTORIAL_BOX_INSTRUCTION);
			assert(step->header >= TUTORIAL_HEADER_NONE &&
			       step->header <= TUTORIAL_HEADER_INSTRUCTION);
			assert(step->dismiss >= TUTORIAL_DISMISS_NONE &&
			       step->dismiss <= TUTORIAL_DISMISS_OPERATION_ERROR);
			assert(step->arrow_count >= 0 &&
			       step->arrow_count <= TUTORIAL_MAX_ARROWS);
			for (int arrow = 0; arrow < step->arrow_count; arrow++) {
				int id = step->arrow_ids[arrow];
				assert(id >= 0 && id < ARROW_ID_COUNT);
				assert((size_t)id < sizeof(g_arrow_highlight_targets) /
				       sizeof(g_arrow_highlight_targets[0]));
				assert(arrow_has_highlight_provider(id) || id == AR_EXEC);
			}
		}
		tr_clear();
		assert(tr_get_step_count() == 0);
		assert(tr_load_level(level_id));
		assert(tutorial_structure_signature() == first_signature);
		for (int index = 0; index < tr_get_step_count(); index++) {
			const tutorial_step_t *step = tr_get_step(index);
			assert(step != NULL && step->text_message != NULL);
			for (int arrow = 0; arrow < step->arrow_count; arrow++) {
				assert(step->arrow_ids[arrow] >= 0 &&
				       step->arrow_ids[arrow] < ARROW_ID_COUNT);
			}
		}
		tr_clear();
	}
}

static void run_isolated_test(void (*test)(void))
{
	tr_clear();
	ha_reset();
	rg_set_zero_flag_highlight_source(UI_HIGHLIGHT_SOURCE_AVAILABILITY, false);
	rg_set_register_highlight_source(UI_HIGHLIGHT_SOURCE_AVAILABILITY, false);
	sb_set_highlight_source(UI_HIGHLIGHT_SOURCE_AVAILABILITY, STEP, false);
	sb_set_highlight_source(UI_HIGHLIGHT_SOURCE_AVAILABILITY, FAST, false);
	memset(provider_progress, 0, sizeof(provider_progress));
	memset(provider_active, 0, sizeof(provider_active));
	use_shared_output_provider = false;
	arrow_draw_count = 0;
	code_box_provider_calls = 0;
	op2_line_rect = (SDL_Rect){0};
	layout_available = true;
	back_button_offset = 0;
	dm_set_screen_resolution(DM_RESOLUTION_1600X900);
	SDL_RenderSetClipRect(g_renderer, NULL);
	test();
	tr_update(NULL);
	tr_clear();
	rg_set_zero_flag_highlight_source(UI_HIGHLIGHT_SOURCE_AVAILABILITY, false);
	rg_set_register_highlight_source(UI_HIGHLIGHT_SOURCE_AVAILABILITY, false);
	sb_set_highlight_source(UI_HIGHLIGHT_SOURCE_AVAILABILITY, STEP, false);
	sb_set_highlight_source(UI_HIGHLIGHT_SOURCE_AVAILABILITY, FAST, false);
	SDL_RenderSetClipRect(g_renderer, NULL);
	ha_reset();
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
	g_test_register_arrow = (texture_t){
		.w = 45,
		.h = 45,
		.texture = SDL_CreateTexture(g_renderer, SDL_PIXELFORMAT_RGBA8888,
		                             SDL_TEXTUREACCESS_STATIC, 45, 45)
	};
	assert(g_test_register_arrow.texture != NULL);
	g_reg_arrow = &g_test_register_arrow;
	run_isolated_test(test_arrow_highlight_mapping);
	run_isolated_test(test_all_tutorial_arrow_ids);
	run_isolated_test(test_challenge_frame_geometry_helper);
	run_isolated_test(test_tutorial_arrow_parser_fixtures);
	run_isolated_test(test_play_arrow_uses_shared_phase_with_active_provider);
	run_isolated_test(test_play_arrow_position_is_independent_of_highlight_visibility);
	run_isolated_test(test_play_arrow_activation_transition_is_continuous);
	run_isolated_test(test_play_arrow_deactivation_transition_is_continuous);
	run_isolated_test(test_play_arrow_does_not_mutate_private_motion);
	run_isolated_test(test_play_arrow_initialization_policy);
	run_isolated_test(test_independent_arrow_retains_private_motion);
	run_isolated_test(test_mandatory_provider_arrow_retains_existing_behavior);
	run_isolated_test(test_synchronized_arrow_positions);
	run_isolated_test(test_op2_vertical_clearance);
	run_isolated_test(test_error_arrow_tracks_output_highlight);
	test_resolution(DM_RESOLUTION_1600X900);
	test_tutorial_reentry();
	test_resolution(DM_RESOLUTION_1920X1080);
	test_tutorial_reentry();
	TTF_CloseFont(g_font);
	SDL_DestroyTexture(g_test_register_arrow.texture);
	SDL_DestroyRenderer(g_renderer);
	SDL_FreeSurface(g_screen);
	TTF_Quit();
	SDL_Quit();
	return 0;
}