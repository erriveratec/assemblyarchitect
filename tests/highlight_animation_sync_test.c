#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <SDL.h>
#include <SDL_ttf.h>

#include "buffers_bf.h"
#include "arrow_ar.h"
#include "code_window_cw.h"
#include "code_window_cw_internal.h"
#include "draw_dw.h"
#include "game_mechanics_mc.h"
#include "dimensions_dm.h"
#include "gameplay/highlight_animation_ha.h"
#include "immediates_im.h"
#include "instruction_window_iw.h"
#include "list.h"
#include "registers_rg.h"
#include "sdl_config.h"
#include "stage_buttons_sb.h"
#include "tutorial_tr.h"
#include "ui/ui_metrics_um.h"

enum { TARGET_PROGRESS_COUNT = 9 };

static texture_t g_test_input_arrow_texture;

List *get_code_list(void);

static struct {
	instruction_t instruction;
	btn_t instruction_button;
	code_line_t line;
} g_pending_fixture;

static int get_cycle_frame_budget(void)
{
	int limit = dw_get_animation_limit();
	int step = dw_get_animation_step();
	assert(limit > 0 && step > 0);
	return 2 * ((limit + step - 1) / step + 2);
}

static float advance_until_interior_phase(void)
{
	float progress = ha_get_progress();
	for (int frame = 0; frame < get_cycle_frame_budget(); frame++) {
		if (progress > 0.0f && progress < 1.0f) {
			return progress;
		}
		ha_begin_frame();
		progress = ha_get_progress();
	}
	assert(!"shared pulse did not reach an interior phase within one cycle");
	return progress;
}

static void clear_highlight_targets(void)
{
	cw_set_code_box_highlight(false);
	cw_set_challenge_highlight(false);
	bf_set_buffer_highlight_source(UI_HIGHLIGHT_SOURCE_TUTORIAL, false, false);
	bf_set_buffer_highlight_source(UI_HIGHLIGHT_SOURCE_AVAILABILITY, false,
	                               false);
	rg_set_register_highlight_source(UI_HIGHLIGHT_SOURCE_TUTORIAL, false);
	rg_set_register_highlight_source(UI_HIGHLIGHT_SOURCE_AVAILABILITY, false);
	iw_set_highlight_instructions(0);
	im_set_highlight_source(UI_HIGHLIGHT_SOURCE_TUTORIAL, false);
	im_set_highlight_source(UI_HIGHLIGHT_SOURCE_AVAILABILITY, false);
	im_set_imm_up_avail(false);
	sb_set_highlight_source(UI_HIGHLIGHT_SOURCE_TUTORIAL, STEP, false);
	sb_set_highlight_source(UI_HIGHLIGHT_SOURCE_TUTORIAL, FAST, false);
	sb_set_highlight_source(UI_HIGHLIGHT_SOURCE_AVAILABILITY, STEP, false);
	sb_set_highlight_source(UI_HIGHLIGHT_SOURCE_AVAILABILITY, FAST, false);
}

static void initialize_test_graphics(void)
{
	assert(g_screen == NULL);
	assert(g_renderer == NULL);
	assert(g_font == NULL);
	assert(g_ib_arrow == NULL);
	assert(g_test_input_arrow_texture.texture == NULL);

	int sdl_result = SDL_Init(0);
	assert(sdl_result == 0);
	(void)sdl_result;
	int ttf_result = TTF_Init();
	assert(ttf_result == 0);
	(void)ttf_result;

	g_screen = SDL_CreateRGBSurfaceWithFormat(
	    0, 1600, 900, 32, SDL_PIXELFORMAT_RGBA32);
	assert(g_screen != NULL);
	g_renderer = SDL_CreateSoftwareRenderer(g_screen);
	assert(g_renderer != NULL);
	g_font = TTF_OpenFont("fonts/DOSVGA437.ttf", 130);
	assert(g_font != NULL);
	dm_set_screen_resolution(DM_RESOLUTION_1600X900);
	g_test_input_arrow_texture = (texture_t){
		.w = 45,
		.h = 45,
		.texture = SDL_CreateTexture(g_renderer, SDL_PIXELFORMAT_RGBA8888,
		                             SDL_TEXTUREACCESS_STATIC, 45, 45)
	};
	assert(g_test_input_arrow_texture.texture != NULL);
	g_ib_arrow = &g_test_input_arrow_texture;
	assert(g_ib_arrow == &g_test_input_arrow_texture);
}

static void clear_test_renderer(void)
{
	assert(SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255) == 0);
	assert(SDL_RenderClear(g_renderer) == 0);
}

static void destroy_test_graphics(void)
{
	tr_clear();
	clear_highlight_targets();
	g_ib_arrow = NULL;
	assert(g_ib_arrow == NULL);

	if (g_test_input_arrow_texture.texture != NULL) {
		SDL_DestroyTexture(g_test_input_arrow_texture.texture);
		g_test_input_arrow_texture.texture = NULL;
	}
	assert(g_test_input_arrow_texture.texture == NULL);

	if (g_font != NULL) {
		TTF_CloseFont(g_font);
		g_font = NULL;
	}
	assert(g_font == NULL);

	if (g_renderer != NULL) {
		SDL_DestroyRenderer(g_renderer);
		g_renderer = NULL;
	}
	assert(g_renderer == NULL);

	if (g_screen != NULL) {
		SDL_FreeSurface(g_screen);
		g_screen = NULL;
	}
	assert(g_screen == NULL);

	TTF_Quit();
	SDL_Quit();
}

static void enable_all_targets(void)
{
	cw_set_code_box_highlight(true);
	cw_set_challenge_highlight(true);
	bf_set_buffer_highlight_source(UI_HIGHLIGHT_SOURCE_TUTORIAL, true, true);
	rg_set_register_highlight_source(UI_HIGHLIGHT_SOURCE_TUTORIAL, true);
	iw_set_highlight_instructions(1);
	im_set_imm_up_avail(true);
	im_set_highlight_source(UI_HIGHLIGHT_SOURCE_TUTORIAL, true);
	sb_set_highlight_source(UI_HIGHLIGHT_SOURCE_TUTORIAL, STEP, true);
	sb_set_highlight_source(UI_HIGHLIGHT_SOURCE_TUTORIAL, FAST, true);
}

static void read_active_target_progress(float progress[TARGET_PROGRESS_COUNT])
{
	const float animation_limit = (float)dw_get_animation_limit();
	assert(cw_get_code_box_highlight_progress(&progress[0], animation_limit));
	assert(cw_get_challenge_highlight_progress(&progress[1], animation_limit));
	assert(bf_get_input_buffer_highlight_progress(&progress[2], animation_limit));
	assert(bf_get_output_buffer_highlight_progress(&progress[3], animation_limit));
	assert(rg_get_register_highlight_progress(&progress[4], animation_limit));
	assert(iw_get_highlight_instruction_progress(&progress[5]));
	assert(im_get_highlight_progress(&progress[6], animation_limit));
	assert(sb_get_button_highlight_progress(STEP, &progress[7],
	                                       animation_limit));
	assert(sb_get_button_highlight_progress(FAST, &progress[8],
	                                       animation_limit));
}

static void assert_all_progress_equal(float expected)
{
	float progress[TARGET_PROGRESS_COUNT];
	read_active_target_progress(progress);
	for (int index = 0; index < TARGET_PROGRESS_COUNT; index++) {
		assert(progress[index] == expected);
	}
}

static void test_transition_joins_shared_phase(void)
{
	ha_reset();
	clear_highlight_targets();
	bf_set_buffer_highlight_source(UI_HIGHLIGHT_SOURCE_TUTORIAL, true, false);
	float interior_phase = advance_until_interior_phase();
	float input_progress = -1.0f;
	assert(bf_get_input_buffer_highlight_progress(
	    &input_progress, (float)dw_get_animation_limit()));
	assert(input_progress == interior_phase);

	cw_set_code_box_highlight(true);
	rg_set_register_highlight_source(UI_HIGHLIGHT_SOURCE_TUTORIAL, true);
	enable_all_targets();
	ha_begin_frame();
	assert_all_progress_equal(ha_get_progress());
	for (int frame = 0; frame < 20; frame++) {
		ha_begin_frame();
		assert_all_progress_equal(ha_get_progress());
	}
}

static void assert_level0_review_progress(float expected)
{
	float code_box_progress = -1.0f;
	float input_buffer_progress = -1.0f;
	float register_progress = -1.0f;
	const float animation_limit = (float)dw_get_animation_limit();
	assert(cw_get_code_box_highlight_progress(&code_box_progress,
	                                         animation_limit));
	assert(bf_get_input_buffer_highlight_progress(&input_buffer_progress,
	                                            animation_limit));
	assert(rg_get_register_highlight_progress(&register_progress,
	                                         animation_limit));
	assert(code_box_progress == expected);
	assert(input_buffer_progress == expected);
	assert(register_progress == expected);
}

static void test_level0_tutorial_transition(void)
{
	ha_reset();
	clear_highlight_targets();
	bool loaded = tr_load_level(0);
	assert(loaded);
	(void)loaded;

	cs_context_t select_input_context = {
		.code_size = 1,
		.operation_flag = NO_OPERATION,
		.code_sorted = true,
		.operand_pending = true,
		.operand_2_pending = true
	};
	const tutorial_step_t *step = tr_update(&select_input_context);
	assert(step != NULL && strcmp(step->name, "select_input_buffer") == 0);
	float input_progress = -1.0f;
	assert(bf_get_input_buffer_highlight_progress(
	    &input_progress, (float)dw_get_animation_limit()));
	float interior_phase = advance_until_interior_phase();
	assert(input_progress == 0.0f);
	assert(bf_get_input_buffer_highlight_progress(
	    &input_progress, (float)dw_get_animation_limit()));
	assert(input_progress == interior_phase);

	/* This regression checks real tutorial activation and the shared normalized
	 * phase. Component tests cover target geometry; amplitudes may differ while
	 * all active targets reach normalized endpoints in the same frame.
	 */
	cs_context_t review_context = {
		.code_size = 1,
		.operation_flag = NO_OPERATION
	};
	step = tr_update(&review_context);
	assert(step != NULL && strcmp(step->name, "review_first_mov") == 0);
	ha_begin_frame();
	assert_level0_review_progress(ha_get_progress());

	for (int frame = 0; frame < 5; frame++) {
		step = tr_update(&review_context);
		assert(step != NULL && strcmp(step->name, "review_first_mov") == 0);
		ha_begin_frame();
		assert_level0_review_progress(ha_get_progress());
	}
	tr_clear();
	clear_highlight_targets();
}

static void test_peaks_directions_and_target_removal(void)
{
	ha_reset();
	clear_highlight_targets();
	enable_all_targets();
	float previous = 0.0f;
	int peaks = 0;
	int troughs = 0;
	bool saw_descending = false;
	bool saw_ascending_again = false;
	int frame_budget = 4 * get_cycle_frame_budget();
	for (int frame = 0; frame < frame_budget &&
	     (peaks < 3 || troughs < 3 || !saw_ascending_again); frame++) {
		ha_begin_frame();
		float current = ha_get_progress();
		assert_all_progress_equal(current);
		assert(current >= 0.0f && current <= 1.0f);
		if (current == 1.0f && previous < 1.0f) {
			peaks++;
		}
		if (current < previous) {
			assert(ha_is_descending());
			saw_descending = true;
		}
		if (current == 0.0f && previous > 0.0f) {
			assert(ha_is_descending());
			troughs++;
		}
		if (saw_descending && current > previous) {
			assert(!ha_is_descending());
			saw_ascending_again = true;
		}
		previous = current;
	}
	assert(peaks >= 3);
	assert(troughs >= 3);
	assert(saw_descending && saw_ascending_again);

	float before_removal = ha_get_progress();
	bf_set_buffer_highlight_source(UI_HIGHLIGHT_SOURCE_TUTORIAL, false, false);
	ha_begin_frame();
	assert(ha_get_progress() != before_removal);
	assert(cw_get_code_box_highlight_progress(&previous,
	                                         (float)dw_get_animation_limit()));
	assert(previous == ha_get_progress());
}

static void test_source_ownership_and_inactive_clock(void)
{
	ha_reset();
	clear_highlight_targets();
	float phase_before_activation = advance_until_interior_phase();
	float before_activation = ha_get_progress();
	assert(before_activation == phase_before_activation);
	rg_set_register_highlight_source(UI_HIGHLIGHT_SOURCE_AVAILABILITY, true);
	ha_begin_frame();
	float availability_progress = -1.0f;
	assert(rg_get_register_highlight_progress(
	    &availability_progress, (float)dw_get_animation_limit()));
	assert(availability_progress == ha_get_progress());
	assert(availability_progress != before_activation);

	rg_set_register_highlight_source(UI_HIGHLIGHT_SOURCE_TUTORIAL, true);
	rg_set_register_highlight_source(UI_HIGHLIGHT_SOURCE_TUTORIAL, false);
	float shared_progress = -1.0f;
	assert(rg_get_register_highlight_progress(
	    &shared_progress, (float)dw_get_animation_limit()));
	assert(shared_progress == availability_progress);
	rg_set_register_highlight_source(UI_HIGHLIGHT_SOURCE_AVAILABILITY, false);
	assert(!rg_get_register_highlight_progress(
	    &shared_progress, (float)dw_get_animation_limit()));

	ha_reset();
	clear_highlight_targets();
	float phase_without_targets = advance_until_interior_phase();
	rg_set_register_highlight_source(UI_HIGHLIGHT_SOURCE_TUTORIAL, true);
	ha_begin_frame();
	assert(rg_get_register_highlight_progress(
	    &shared_progress, (float)dw_get_animation_limit()));
	assert(shared_progress == ha_get_progress());
	assert(shared_progress > phase_without_targets);
}

static void test_getters_preserve_frame_snapshot(void)
{
	ha_reset();
	clear_highlight_targets();
	ha_begin_frame();
	float snapshot = ha_get_progress();
	float scaled_snapshot = ha_get_value(7.5f);
	for (int sample = 0; sample < 12; sample++) {
		assert(ha_get_progress() == snapshot);
		assert(ha_get_value(7.5f) == scaled_snapshot);
		assert(ha_get_value(2.0f) == snapshot * 2.0f);
	}
	ha_begin_frame();
	assert(ha_get_progress() != snapshot);
}

static void test_invalid_getter_inputs(void)
{
	ha_reset();
	clear_highlight_targets();
	enable_all_targets();
	float progress = -1.0f;
	assert(!cw_get_code_box_highlight_progress(NULL, 1.0f));
	assert(!cw_get_code_box_highlight_progress(&progress, 0.0f));
	assert(!cw_get_challenge_highlight_progress(&progress, -1.0f));
	assert(!bf_get_input_buffer_highlight_progress(NULL, 1.0f));
	assert(!bf_get_output_buffer_highlight_progress(&progress, 0.0f));
	assert(!rg_get_register_highlight_progress(&progress, -1.0f));
	assert(!iw_get_highlight_instruction_progress(NULL));
	assert(!im_get_highlight_progress(&progress, 0.0f));
	assert(!sb_get_button_highlight_progress(INVALID, &progress, 1.0f));
	assert(!sb_get_button_highlight_progress(STEP, NULL, 1.0f));
	assert(!sb_get_button_highlight_progress(FAST, &progress, -1.0f));
	assert(ha_get_value(0.0f) == 0.0f);
	assert(ha_get_value(-1.0f) == 0.0f);
	assert(ha_get_value(INFINITY) == 0.0f);
	clear_highlight_targets();
}

static void assert_rect_equal(SDL_Rect actual, SDL_Rect expected)
{
	assert(actual.x == expected.x && actual.y == expected.y);
	assert(actual.w == expected.w && actual.h == expected.h);
}

static bool pixel_is_white(int x, int y)
{
	SDL_Rect pixel_rect = {.x = x, .y = y, .w = 1, .h = 1};
	Uint32 pixel = 0;
	assert(SDL_RenderReadPixels(g_renderer, &pixel_rect,
	                            g_screen->format->format, &pixel,
	                            sizeof(pixel)) == 0);
	Uint8 red;
	Uint8 green;
	Uint8 blue;
	Uint8 alpha;
	SDL_GetRGBA(pixel, g_screen->format, &red, &green, &blue, &alpha);
	return red == 255 && green == 255 && blue == 255 && alpha == 255;
}

static void assert_pending_draw_state(int state, int expected_offset)
{
	int op1_offset = cl_get_code_button_size().w;
	int op2_offset = 2 * cl_get_code_button_size().w +
	                  um_padding_horizontal();
	SDL_Rect expected_base = g_pending_fixture.instruction_button.r;
	expected_base.x += expected_offset;
	SDL_Rect expected_rect;
	assert(cw_calculate_pending_operand_rect(expected_base, ha_get_progress(),
	                                       dw_get_animation_limit(),
	                                       &expected_rect));

	g_pending_fixture.line.state = state;
	clear_test_renderer();
	cw_highlight_code_pending_operand();
	assert(pixel_is_white(expected_rect.x, expected_rect.y));
}

static void assert_no_pending_draw(int state)
{
	int offsets[] = {
		cl_get_code_button_size().w,
		2 * cl_get_code_button_size().w + um_padding_horizontal()
	};
	g_pending_fixture.line.state = state;
	clear_test_renderer();
	cw_highlight_code_pending_operand();
	for (size_t index = 0; index < sizeof(offsets) / sizeof(offsets[0]); index++) {
		SDL_Rect base_rect = g_pending_fixture.instruction_button.r;
		SDL_Rect animated_rect;
		base_rect.x += offsets[index];
		assert(cw_calculate_pending_operand_rect(base_rect, ha_get_progress(),
		                                       dw_get_animation_limit(),
		                                       &animated_rect));
		assert(!pixel_is_white(animated_rect.x, animated_rect.y));
	}
}

static void test_pending_rect_geometry_helper(void)
{
	SDL_Rect base_rect = {.x = 30, .y = 40, .w = 120, .h = 80};
	SDL_Rect minimum_rect;
	SDL_Rect maximum_rect;
	SDL_Rect clamped_low;
	SDL_Rect clamped_high;
	SDL_Rect fallback_zero;
	SDL_Rect fallback_negative;
	int animation_limit = dw_get_animation_limit();
	assert(animation_limit > 0);
	assert(cw_calculate_pending_operand_rect(base_rect, 0.0f,
	                                       animation_limit, &minimum_rect));
	assert(cw_calculate_pending_operand_rect(base_rect, 1.0f,
	                                       animation_limit, &maximum_rect));
	assert(minimum_rect.w == base_rect.w - animation_limit);
	assert(minimum_rect.h == base_rect.h - animation_limit);
	assert(maximum_rect.x == base_rect.x && maximum_rect.y == base_rect.y);
	assert(maximum_rect.w == base_rect.w && maximum_rect.h == base_rect.h);
	assert(minimum_rect.w > 0 && minimum_rect.h > 0);

	assert(cw_calculate_pending_operand_rect(base_rect, -0.5f,
	                                       animation_limit, &clamped_low));
	assert(cw_calculate_pending_operand_rect(base_rect, 1.5f,
	                                       animation_limit, &clamped_high));
	assert_rect_equal(clamped_low, minimum_rect);
	assert_rect_equal(clamped_high, maximum_rect);

	assert(cw_calculate_pending_operand_rect(base_rect, 0.4f, 0,
	                                       &fallback_zero));
	assert(cw_calculate_pending_operand_rect(base_rect, 0.4f, -9,
	                                       &fallback_negative));
	assert_rect_equal(fallback_zero, base_rect);
	assert_rect_equal(fallback_negative, base_rect);
	assert(!cw_calculate_pending_operand_rect(base_rect, NAN, animation_limit,
	                                        &maximum_rect));
	assert(!cw_calculate_pending_operand_rect(base_rect, 0.5f, animation_limit,
	                                        NULL));
	SDL_Rect small_rect = {.x = 5, .y = 7, .w = 1, .h = 2};
	assert(cw_calculate_pending_operand_rect(small_rect, 0.0f, 100,
	                                       &minimum_rect));
	assert(minimum_rect.w > 0 && minimum_rect.h > 0);
}

static void test_pending_operand_uses_shared_phase(void)
{
	cw_create_code_list();
	memset(&g_pending_fixture, 0, sizeof(g_pending_fixture));
	g_pending_fixture.instruction.id = MOV;
	g_pending_fixture.instruction.b = &g_pending_fixture.instruction_button;
	g_pending_fixture.instruction_button.r = (SDL_Rect){
		.x = 100, .y = 180, .w = 180, .h = 100
	};
	g_pending_fixture.line.ins = &g_pending_fixture.instruction;
	g_pending_fixture.line.state = COMPLETE;
	List_push(get_code_list(), &g_pending_fixture.line);

	ha_reset();
	clear_highlight_targets();
	float interior_phase = advance_until_interior_phase();
	g_pending_fixture.line.state = MISSING_BOTH;
	rg_set_register_highlight_source(UI_HIGHLIGHT_SOURCE_AVAILABILITY, true);
	float register_progress = -1.0f;
	assert(rg_get_register_highlight_progress(
	    &register_progress, (float)dw_get_animation_limit()));
	assert(register_progress == interior_phase);

	float snapshot = ha_get_progress();
	SDL_Rect base_rect = g_pending_fixture.instruction_button.r;
	base_rect.x += cl_get_code_button_size().w;
	SDL_Rect first_sample;
	assert(cw_calculate_pending_operand_rect(base_rect, snapshot,
	                                       dw_get_animation_limit(),
	                                       &first_sample));
	for (int sample = 0; sample < 8; sample++) {
		SDL_Rect repeated_sample;
		assert(cw_calculate_pending_operand_rect(base_rect, ha_get_progress(),
		                                       dw_get_animation_limit(),
		                                       &repeated_sample));
		assert_rect_equal(first_sample, repeated_sample);
		assert(ha_get_progress() == snapshot);
	}

	bool saw_minimum = false;
	bool saw_maximum = false;
	bool saw_descending = false;
	bool saw_ascending = false;
	for (int frame = 0; frame < 2 * get_cycle_frame_budget(); frame++) {
		ha_begin_frame();
		float progress = ha_get_progress();
		assert(rg_get_register_highlight_progress(
		    &register_progress, (float)dw_get_animation_limit()));
		assert(register_progress == progress);
		SDL_Rect animated_rect;
		assert(cw_calculate_pending_operand_rect(base_rect, progress,
		                                       dw_get_animation_limit(),
		                                       &animated_rect));
		assert(animated_rect.w > 0 && animated_rect.h > 0);
		if (progress == 0.0f) saw_minimum = true;
		if (progress == 1.0f) saw_maximum = true;
		if (ha_is_descending()) saw_descending = true;
		else saw_ascending = true;
	}
	assert(saw_minimum && saw_maximum);
	assert(saw_descending && saw_ascending);

	const float fractional_phases[] = {0.13f, 0.37f, 0.5f, 0.83f, 0.999f};
	SDL_Rect previous_rect;
	assert(cw_calculate_pending_operand_rect(base_rect, fractional_phases[0],
	                                       dw_get_animation_limit(),
	                                       &previous_rect));
	for (size_t index = 0;
	     index < sizeof(fractional_phases) / sizeof(fractional_phases[0]);
	     index++) {
		SDL_Rect current_rect;
		assert(cw_calculate_pending_operand_rect(base_rect,
		                                       fractional_phases[index],
		                                       dw_get_animation_limit(),
		                                       &current_rect));
		assert(current_rect.w > 0 && current_rect.h > 0);
		if (index > 0) assert(current_rect.w >= previous_rect.w);
		previous_rect = current_rect;
	}

	int op1_offset = cl_get_code_button_size().w;
	int op2_offset = 2 * cl_get_code_button_size().w +
	                  um_padding_horizontal();
	const struct {
		int state;
		int expected_offset;
	} active_states[] = {
		{MISSING_BOTH, op1_offset},
		{MISSING_OP1, op1_offset},
		{MISSING_OP2, op2_offset},
		{CHANGING_OP1, op1_offset},
		{CHANGING_OP2, op2_offset}
	};
	for (size_t index = 0;
	     index < sizeof(active_states) / sizeof(active_states[0]); index++) {
		assert_pending_draw_state(active_states[index].state,
		                          active_states[index].expected_offset);
	}
	assert_no_pending_draw(COMPLETE);
	assert_no_pending_draw(EXECUTED);

	List_pop(get_code_list());
	assert_no_pending_draw(MISSING_BOTH);
	clear_highlight_targets();
}

int main(void)
{
	initialize_test_graphics();
	test_transition_joins_shared_phase();
	test_level0_tutorial_transition();
	test_peaks_directions_and_target_removal();
	test_source_ownership_and_inactive_clock();
	test_getters_preserve_frame_snapshot();
	test_invalid_getter_inputs();
	test_pending_rect_geometry_helper();
	test_pending_operand_uses_shared_phase();
	destroy_test_graphics();
	puts("highlight animation synchronization tests passed");
	return 0;
}
