#include <assert.h>
#include <math.h>
#include <SDL_ttf.h>
#include "game_mechanics_mc.h"
#include "sdl_config.h"
#include "tutorial_tr.h"
#include "gameplay/highlight_animation_ha.h"

static void test_draw_rotated_texture(int x, int y, int height, double angle,
									  texture_t *texture);
#define iw_get_highlight_instruction_progress test_instruction_progress
#define cw_get_code_box_highlight_progress test_code_box_progress
#define cw_get_code_list_size test_code_list_size
#define cw_get_code_line_coord_at_pos test_code_line_coord_at_pos
#define cw_get_challenge_highlight_progress test_challenge_progress
#define bf_get_input_buffer_highlight_progress test_input_progress
#define bf_get_output_buffer_highlight_progress test_output_progress
#define rg_get_register_highlight_progress test_register_progress
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
#undef bf_get_output_buffer_highlight_progress
#undef bf_get_input_buffer_highlight_progress
#undef cw_get_challenge_highlight_progress
#undef cw_get_code_line_coord_at_pos
#undef cw_get_code_list_size
#undef cw_get_code_box_highlight_progress
#undef iw_get_highlight_instruction_progress

bool bf_get_output_buffer_highlight_progress(float *progress, float animation_limit);
bool rg_get_register_highlight_progress(float *progress, float animation_limit);

static float provider_progress[HIGHLIGHT_IMMEDIATES + 1];
static bool provider_active[HIGHLIGHT_IMMEDIATES + 1];
static bool use_shared_output_provider;
static int arrow_draw_count;
static int code_box_provider_calls;
static SDL_Rect op2_line_rect;

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

static void assert_cycles(float limit)
{
	int half_cycle = (dw_get_animation_limit() + dw_get_animation_step() - 1)
		/ dw_get_animation_step();
	int upper_endpoints = 0;
	int lower_endpoints = 0;
	float previous = 0.0f;
	for (int frame = 0; frame <= 8 * (half_cycle + 1); frame++) {
		ha_begin_frame();
		float actual = ha_get_progress();
		float register_progress = -1.0f;
		assert(rg_get_register_highlight_progress(&register_progress, limit));
		assert_progress_equal(register_progress, actual);
		assert_error_position(actual, limit);
		if (actual == 1.0f && previous < 1.0f) {
			upper_endpoints++;
			assert(g_arrow_error.box.x == g_arrow_error.startx + g_arrow_error.travel);
		}
		if (frame > 0 && actual == 0.0f && previous > 0.0f) {
			lower_endpoints++;
			assert(g_arrow_error.box.x == g_arrow_error.startx);
		}
		previous = actual;
	}
	assert(upper_endpoints >= 3 && lower_endpoints >= 3);
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
	assert(tr_load_level(0));
	cs_context_t context = {
		.code_size = 1,
		.code_sorted = true,
		.operation_flag = OUTPUT_BUFFER_INCOMPLETE
	};
	const tutorial_step_t *failure_step = tr_get_named_step("explain_missing_output");
	assert(failure_step != NULL);
	assert(failure_step->arrow_count == 1);
	assert(failure_step->arrow_ids[0] == AR_ERROR);
	assert(failure_step->highlight_targets & TUTORIAL_HIGHLIGHT_OUTPUT_BUFFER);
	assert(failure_step->highlight_targets & TUTORIAL_HIGHLIGHT_REGISTERS);
	assert(g_arrow_highlight_targets[failure_step->arrow_ids[0]] ==
	       HIGHLIGHT_OUTPUT_BUFFER);
	use_shared_output_provider = true;
	ha_reset();
	for (int opening = 0; opening < 3; opening++) {
		back_button_offset = opening * 31;
		ha_begin_frame();
		float entry_progress = ha_get_progress();
		assert(tr_update(&context) == failure_step);
		assert_reset();
		float limit = dw_get_animation_limit();
		assert_error_position(entry_progress, limit);
		assert_cycles(limit);
		SDL_Rect before_update = g_arrow_error.box;
		assert(tr_update(&context) == failure_step);
		assert_error_position(ha_get_progress(), limit);
		assert(g_arrow_error.box.x == before_update.x);
		assert(g_arrow_error.box.y == before_update.y);
		assert(tr_update(NULL) == NULL);
		float progress = -1.0f;
		assert(!get_highlight_progress(&g_arrow_error, &progress, limit));
		int draws_before = arrow_draw_count;
		ar_display_arrow(AR_ERROR, limit);
		assert(arrow_draw_count == draws_before);
		assert_reset();
		ha_begin_frame();
	}
	tr_clear();
	use_shared_output_provider = false;
	back_button_offset = 0;
}

static void test_play_tutorial_steps_preserve_existing_highlight_configuration(void)
{
	assert(tr_load_level(0));
	const tutorial_step_t *without_code_box =
		tr_get_named_step("run_first_instruction");
	assert(without_code_box != NULL);
	assert(without_code_box->arrow_count == 1);
	assert(without_code_box->arrow_ids[0] == AR_PLAY);
	assert(!(without_code_box->highlight_targets &
	         TUTORIAL_HIGHLIGHT_CODE_BOX));
	tr_clear();

	assert(tr_load_level(1));
	const tutorial_step_t *with_code_box = tr_get_named_step("run_program");
	assert(with_code_box != NULL);
	assert(with_code_box->arrow_count == 1);
	assert(with_code_box->arrow_ids[0] == AR_PLAY);
	assert(with_code_box->highlight_targets & TUTORIAL_HIGHLIGHT_CODE_BOX);
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
	/* These arrows remain intentionally independent of highlight providers. */
	assert(g_arrow_highlight_targets[AR_PLAY] == HIGHLIGHT_CODE_BOX);
	assert(arrow_has_highlight_provider(AR_PLAY));
	assert(g_arrow_highlight_targets[AR_ZF] == HIGHLIGHT_NONE);
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
}

static void test_synchronized_arrow_positions(void)
{
	static const int synchronized_arrows[] = {
		AR_INS, AR_INS_MINUS, AR_DROP, AR_STEP, AR_FAST, AR_CODE, AR_DEL,
		AR_OP2, AR_ERROR, AR_CHALLENGE, AR_IB, AR_OB, AR_REG, AR_IMM_UP
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

static void test_conditional_jump_tutorial_configuration(void)
{
	assert(tr_load_level(13));
	const tutorial_step_t *step =
		tr_get_named_step("complete_compare_branch_program");
	assert(step != NULL);
	assert(step->arrow_count == 1);
	assert(step->arrow_ids[0] == AR_ZF);
	assert(step->highlight_instruction_mask == (1 << JE));
	assert((step->highlight_targets & TUTORIAL_HIGHLIGHT_CHALLENGE) == 0);
	assert(g_arrow_highlight_targets[step->arrow_ids[0]] ==
	       HIGHLIGHT_NONE);
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
	test_play_arrow_uses_shared_phase_with_active_provider();
	test_play_arrow_position_is_independent_of_highlight_visibility();
	test_play_arrow_activation_transition_is_continuous();
	test_play_arrow_deactivation_transition_is_continuous();
	test_play_arrow_does_not_mutate_private_motion();
	test_play_arrow_initialization_policy();
	test_independent_arrow_retains_private_motion();
	test_mandatory_provider_arrow_retains_existing_behavior();
	test_synchronized_arrow_positions();
	test_level1_code_box_arrow_regression();
	test_play_tutorial_steps_preserve_existing_highlight_configuration();
	test_op2_vertical_clearance();
	test_conditional_jump_tutorial_configuration();
	test_error_arrow_tracks_output_highlight();
	test_resolution(DM_RESOLUTION_1600X900);
	test_tutorial_reentry();
	test_resolution(DM_RESOLUTION_1920X1080);
	test_tutorial_reentry();
	TTF_CloseFont(g_font);
	SDL_DestroyRenderer(g_renderer);
	SDL_FreeSurface(g_screen);
	TTF_Quit();
	SDL_Quit();
	return 0;
}