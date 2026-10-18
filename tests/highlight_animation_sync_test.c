#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <SDL.h>
#include <SDL_ttf.h>

#include "buffers_bf.h"
#include "arrow_ar.h"
#include "code_window_cw.h"
#include "draw_dw.h"
#include "game_mechanics_mc.h"
#include "dimensions_dm.h"
#include "gameplay/highlight_animation_ha.h"
#include "immediates_im.h"
#include "instruction_window_iw.h"
#include "registers_rg.h"
#include "sdl_config.h"
#include "stage_buttons_sb.h"
#include "tutorial_tr.h"

enum { TARGET_PROGRESS_COUNT = 9 };

static texture_t g_test_input_arrow_texture;

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

int main(void)
{
	initialize_test_graphics();
	test_transition_joins_shared_phase();
	test_level0_tutorial_transition();
	test_peaks_directions_and_target_removal();
	test_source_ownership_and_inactive_clock();
	test_getters_preserve_frame_snapshot();
	test_invalid_getter_inputs();
	destroy_test_graphics();
	puts("highlight animation synchronization tests passed");
	return 0;
}
