#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
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
#include "mouse_ms.h"
#include "registers_rg.h"
#include "sdl_config.h"
#include "stage_buttons_sb.h"
#include "tutorial_tr.h"
#include "tutorial_tr_internal.h"
#include "ui/ui_metrics_um.h"

enum { TARGET_PROGRESS_COUNT = 10 };

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
	rg_set_zero_flag_highlight_source(UI_HIGHLIGHT_SOURCE_TUTORIAL, false);
	rg_set_zero_flag_highlight_source(UI_HIGHLIGHT_SOURCE_AVAILABILITY, false);
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
	rg_set_zero_flag_highlight_source(UI_HIGHLIGHT_SOURCE_TUTORIAL, true);
}

static void read_active_target_progress(float progress[TARGET_PROGRESS_COUNT])
{
	const float animation_limit = (float)dw_get_animation_limit();
	assert(cw_get_code_box_highlight_progress(&progress[0], animation_limit));
	assert(cw_get_challenge_highlight_progress(&progress[1], animation_limit));
	assert(bf_get_input_buffer_highlight_progress(&progress[2], animation_limit));
	assert(bf_get_output_buffer_highlight_progress(&progress[3], animation_limit));
	assert(rg_get_register_highlight_progress(&progress[4], animation_limit));
	assert(rg_get_zero_flag_highlight_progress(&progress[5], animation_limit));
	assert(iw_get_highlight_instruction_progress(&progress[6]));
	assert(im_get_highlight_progress(&progress[7], animation_limit));
	assert(sb_get_button_highlight_progress(STEP, &progress[8],
	                                       animation_limit));
	assert(sb_get_button_highlight_progress(FAST, &progress[9],
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

static void test_fixture_tutorial_transition(void)
{
	ha_reset();
	clear_highlight_targets();
	static const char fixture[] =
	    "[tutorial]\nformat = 1\n"
	    "[fixture_input]\nbox = center\nheader = none\ndismiss = none\n"
	    "highlight = input_buffer\nwhen.code_size = 1\n"
	    "when.operand_pending = 1\nwhen.operand_2_pending = 1\n"
	    "text_begin\nInput fixture\ntext_end\n"
	    "[fixture_review]\nbox = center\nheader = none\ndismiss = none\n"
	    "highlight = code_box,input_buffer,register\nwhen.code_size = 1\n"
	    "when.operand_pending = 0\nwhen.operation_error = 0\n"
	    "text_begin\nReview fixture\ntext_end\n";
	FILE *stream = tmpfile();
	assert(stream != NULL);
	assert(fwrite(fixture, 1, sizeof(fixture) - 1, stream) == sizeof(fixture) - 1);
	rewind(stream);
	assert(tr_load_tutorial_stream_for_testing(stream,
	                                           "highlight-sync-fixture"));
	fclose(stream);

	cs_context_t select_input_context = {
		.code_size = 1,
		.operation_flag = NO_OPERATION,
		.code_sorted = true,
		.operand_pending = true,
		.operand_2_pending = true
	};
	const tutorial_step_t *step = tr_update(&select_input_context);
	assert(step == tr_get_named_step("fixture_input"));
	float input_progress = -1.0f;
	assert(bf_get_input_buffer_highlight_progress(
	    &input_progress, (float)dw_get_animation_limit()));
	float interior_phase = advance_until_interior_phase();
	assert(input_progress == 0.0f);
	assert(bf_get_input_buffer_highlight_progress(
	    &input_progress, (float)dw_get_animation_limit()));
	assert(input_progress == interior_phase);

	/* Active fixture targets must join the same normalized animation phase. */
	cs_context_t review_context = {
		.code_size = 1,
		.operation_flag = NO_OPERATION
	};
	step = tr_update(&review_context);
	assert(step == tr_get_named_step("fixture_review"));
	ha_begin_frame();
	assert_level0_review_progress(ha_get_progress());

	for (int frame = 0; frame < 5; frame++) {
		step = tr_update(&review_context);
		assert(step == tr_get_named_step("fixture_review"));
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

static void test_zero_flag_source_ownership_and_progress(void)
{
	ha_reset();
	clear_highlight_targets();
	const float animation_limit = (float)dw_get_animation_limit();
	float progress = -1.0f;
	assert(!rg_get_zero_flag_highlight_progress(&progress, animation_limit));

	rg_set_zero_flag_highlight_source(UI_HIGHLIGHT_SOURCE_TUTORIAL, true);
	rg_set_zero_flag_highlight_source(UI_HIGHLIGHT_SOURCE_TUTORIAL, true);
	ha_begin_frame();
	assert(rg_get_zero_flag_highlight_progress(&progress, animation_limit));
	assert(progress == ha_get_progress());
	assert(progress >= 0.0f && progress <= 1.0f);
	float previous = progress;
	ha_begin_frame();
	assert(rg_get_zero_flag_highlight_progress(&progress, animation_limit));
	assert(progress == ha_get_progress());
	assert(progress != previous);

	rg_set_zero_flag_highlight_source(UI_HIGHLIGHT_SOURCE_AVAILABILITY, true);
	rg_set_zero_flag_highlight_source(UI_HIGHLIGHT_SOURCE_TUTORIAL, false);
	rg_set_zero_flag_highlight_source(UI_HIGHLIGHT_SOURCE_TUTORIAL, false);
	assert(rg_get_zero_flag_highlight_progress(&progress, animation_limit));
	assert(progress == ha_get_progress());
	rg_set_zero_flag_highlight_source(UI_HIGHLIGHT_SOURCE_AVAILABILITY, false);
	assert(!rg_get_zero_flag_highlight_progress(&progress, animation_limit));

	rg_set_zero_flag_highlight_source(UI_HIGHLIGHT_SOURCE_TUTORIAL, true);
	assert(!rg_get_zero_flag_highlight_progress(NULL, animation_limit));
	assert(!rg_get_zero_flag_highlight_progress(&progress, 0.0f));
	assert(!rg_get_zero_flag_highlight_progress(&progress, -1.0f));
	assert(!rg_get_zero_flag_highlight_progress(&progress, NAN));
	assert(!rg_get_zero_flag_highlight_progress(&progress, INFINITY));
	rg_set_zero_flag_highlight_source(UI_HIGHLIGHT_SOURCE_TUTORIAL, false);
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

static void assert_region_contains_color(SDL_Rect region, SDL_Color expected)
{
	Uint32 *pixels = malloc(region.w * region.h * sizeof(*pixels));
	assert(pixels != NULL);
	assert(SDL_RenderReadPixels(g_renderer, &region, SDL_PIXELFORMAT_RGBA32,
	                            pixels, region.w * sizeof(*pixels)) == 0);
	bool found = false;
	for (int index = 0; index < region.w * region.h; index++) {
		Uint8 red, green, blue, alpha;
		SDL_GetRGBA(pixels[index], g_screen->format,
		            &red, &green, &blue, &alpha);
		if (red == expected.r && green == expected.g &&
		    blue == expected.b && alpha == expected.a) {
			found = true;
			break;
		}
	}
	free(pixels);
	assert(found);
}

static void test_button_foreground_override(void)
{
	texture_t *texture = dw_create_text_tex("rax", C_WHITE);
	assert(texture != NULL);
	btn_t button = {
		.r = {.x = 100, .y = 100, .w = 180, .h = 60},
		.t = texture,
		.enabled = true
	};
	SDL_Rect original = button.r;
	ms_init_mouse();
	for (int blocked = 0; blocked < 2; blocked++) {
		for (int hover = 0; hover < 2; hover++) {
			clear_test_renderer();
			bt_draw_btn_with_text_color(&button, blocked, hover, C_WHITE);
			assert_region_contains_color(button.r, C_WHITE);
			assert_rect_equal(button.r, original);
			Uint8 red, green, blue;
			assert(SDL_GetTextureColorMod(texture->texture, &red, &green, &blue) == 0);
			assert(red == C_WHITE.r && green == C_WHITE.g && blue == C_WHITE.b);
		}
	}
	clear_test_renderer();
	bt_draw_btn(&button, false, false);
	assert_region_contains_color(button.r, C_LIGHTGREY);
	dw_free_texture(texture);
}

static void read_renderer_pixels(unsigned char *pixels)
{
	assert(SDL_RenderReadPixels(g_renderer, NULL, SDL_PIXELFORMAT_RGBA32, pixels,
	                            g_screen->pitch) == 0);
}

static void test_button_highlights_use_shared_phase(void)
{
	texture_t *texture = dw_create_text_tex("rax", C_WHITE);
	assert(texture != NULL);
	btn_t first = {
		.r = {.x = 100, .y = 100, .w = 180, .h = 60},
		.t = texture,
		.animated = false,
		.anim_dir = true,
		.anim_state = 7,
		.enabled = true
	};
	btn_t second = {
		.r = {.x = 400, .y = 100, .w = 180, .h = 60},
		.t = texture,
		.animated = true,
		.anim_dir = true,
		.anim_state = 13,
		.enabled = true
	};
	btn_t third = {
		.r = {.x = 700, .y = 100, .w = 180, .h = 60},
		.t = texture,
		.animated = true,
		.anim_dir = false,
		.anim_state = 21,
		.enabled = true
	};
	const size_t pixel_count = (size_t)g_screen->pitch * (size_t)g_screen->h;
	unsigned char *forward_pixels = malloc(pixel_count);
	unsigned char *reverse_pixels = malloc(pixel_count);
	assert(forward_pixels != NULL && reverse_pixels != NULL);
	int animation_limit = dw_get_animation_limit();
	assert(animation_limit > 0);
	ha_reset();
	ms_init_mouse();
	for (int frame = 0; frame < 2 * get_cycle_frame_budget(); frame++) {
		ha_begin_frame();
		float phase = ha_get_progress();
		clear_test_renderer();
		bt_draw_btn_with_highlight_progress(&first, false, false, phase);
		bt_draw_btn_with_highlight_progress(&second, false, false, phase);
		bt_draw_btn_with_highlight_progress(&third, false, false, phase);
		assert(ha_get_progress() == phase);
		read_renderer_pixels(forward_pixels);

		clear_test_renderer();
		bt_draw_btn_with_highlight_progress(&third, false, false, phase);
		bt_draw_btn_with_highlight_progress(&second, false, false, phase);
		bt_draw_btn_with_highlight_progress(&first, false, false, phase);
		assert(ha_get_progress() == phase);
		read_renderer_pixels(reverse_pixels);
		assert(memcmp(forward_pixels, reverse_pixels, pixel_count) == 0);

		clear_test_renderer();
		int skipped_component = frame % 3;
		if (skipped_component != 0) {
			bt_draw_btn_with_highlight_progress(&first, false, false, phase);
		}
		if (skipped_component != 1) {
			bt_draw_btn_with_highlight_progress(&second, false, false, phase);
		}
		if (skipped_component != 2) {
			bt_draw_btn_with_highlight_progress(&third, false, false, phase);
		}
		assert(ha_get_progress() == phase);
		assert(!first.animated && first.anim_dir && first.anim_state == 7);
		assert(second.animated && second.anim_dir && second.anim_state == 13);
		assert(third.animated && !third.anim_dir && third.anim_state == 21);
	}
	free(forward_pixels);
	free(reverse_pixels);
	dw_free_texture(texture);
}

static void test_group_highlight_geometry(void)
{
	const SDL_FRect base[] = {
		{.x = 100.0f, .y = 100.0f, .w = 50.0f, .h = 24.0f},
		{.x = 150.0f, .y = 100.0f, .w = 50.0f, .h = 24.0f}
	};
	const SDL_FRect original[] = {base[0], base[1]};
	const float phases[] = {0.0f, 0.25f, 0.5f, 0.75f, 1.0f};
	const float original_center_x = 150.0f;
	const float original_center_y = 112.0f;
	const float original_center_distance = 50.0f;
	int animation_limit = dw_get_animation_limit();
	assert(animation_limit > 0);
	for (size_t phase_index = 0;
	     phase_index < sizeof(phases) / sizeof(phases[0]); phase_index++) {
		SDL_FRect output[2];
		assert(bt_calculate_highlight_group_rects(
			base, 2, phases[phase_index], animation_limit, output));
		for (int index = 0; index < 2; index++) {
			assert(output[index].w > 0.0f && output[index].h > 0.0f);
		}
		assert(output[0].x + output[0].w <= output[1].x + 0.001f);
		assert(fabsf(output[0].y - output[1].y) < 0.001f);
		float union_left = fminf(output[0].x, output[1].x);
		float union_right = fmaxf(output[0].x + output[0].w,
		                          output[1].x + output[1].w);
		float union_top = fminf(output[0].y, output[1].y);
		float union_bottom = fmaxf(output[0].y + output[0].h,
		                           output[1].y + output[1].h);
		assert(fabsf((union_left + union_right) / 2.0f - original_center_x) < 0.001f);
		assert(fabsf((union_top + union_bottom) / 2.0f - original_center_y) < 0.001f);
		float first_center = output[0].x + output[0].w / 2.0f;
		float second_center = output[1].x + output[1].w / 2.0f;
		float distance = second_center - first_center;
		float scale = output[0].w / base[0].w;
		assert(fabsf(distance - original_center_distance * scale) < 0.001f);
		if (phase_index == 0) {
			assert(memcmp(output, original, sizeof(output)) == 0);
		}
	}
	assert(memcmp(base, original, sizeof(base)) == 0);

	float independent_scale = 1.0f + (float)animation_limit / base[0].w;
	float independent_left_right = base[0].x + base[0].w / 2.0f +
		(base[0].w * independent_scale) / 2.0f;
	float independent_right_left = base[1].x + base[1].w / 2.0f -
		(base[1].w * independent_scale) / 2.0f;
	assert(independent_left_right > independent_right_left);
	SDL_FRect grouped_maximum[2];
	assert(bt_calculate_highlight_group_rects(base, 2, 1.0f,
	                                         animation_limit, grouped_maximum));
	assert(grouped_maximum[0].x + grouped_maximum[0].w <=
	       grouped_maximum[1].x + 0.001f);
	assert(!bt_calculate_highlight_group_rects(NULL, 2, 0.5f,
	                                          animation_limit, grouped_maximum));
	assert(!bt_calculate_highlight_group_rects(base, 0, 0.5f,
	                                          animation_limit, grouped_maximum));
	SDL_FRect invalid[] = {{.x = 0.0f, .y = 0.0f, .w = 0.0f, .h = 10.0f}};
	assert(!bt_calculate_highlight_group_rects(invalid, 1, 0.5f,
	                                          animation_limit, grouped_maximum));
	assert(!bt_calculate_highlight_group_rects(base, 2, NAN,
	                                          animation_limit, grouped_maximum));
	assert(bt_calculate_highlight_group_rects(base, 2, 1.0f, 0,
	                                         grouped_maximum));
	assert(memcmp(grouped_maximum, original, sizeof(grouped_maximum)) == 0);
	assert(bt_calculate_highlight_group_rects(base, 2, 1.0f, -4,
	                                         grouped_maximum));
	assert(memcmp(grouped_maximum, original, sizeof(grouped_maximum)) == 0);
}

static void test_reference_scale_left_anchored_group(void)
{
	btn_t reference = {
		.r = {.x = 10, .y = 20, .w = 50, .h = 24},
		.enabled = true
	};
	texture_t reference_texture = {.w = 100, .h = 24};
	reference.t = &reference_texture;
	const SDL_FRect base[] = {
		{.x = 5.0f, .y = 100.0f, .w = 40.0f, .h = 24.0f},
		{.x = 45.0f, .y = 100.0f, .w = 45.0f, .h = 24.0f}
	};
	const float phases[] = {0.0f, 0.25f, 0.5f, 0.75f, 1.0f};
	int animation_limit = dw_get_animation_limit();
	for (size_t phase_index = 0;
	     phase_index < sizeof(phases) / sizeof(phases[0]); phase_index++) {
		float reference_scale;
		assert(bt_calculate_button_highlight_scale(
			&reference, phases[phase_index], &reference_scale));
		SDL_FRect output[2];
		assert(bt_calculate_highlight_group_rects_with_scale(
			base, 2, reference_scale, BT_HIGHLIGHT_ANCHOR_LEFT, output));
		assert(fabsf(output[0].x - base[0].x) < 0.0001f);
		assert(fabsf(output[0].x - 5.0f) < 0.0001f);
		assert(output[0].x + output[0].w <= output[1].x + 0.0001f);
		assert(fabsf(output[0].w / base[0].w - reference_scale) < 0.0001f);
		assert(fabsf(output[1].w / base[1].w - reference_scale) < 0.0001f);
		assert(fabsf(output[0].h / base[0].h - reference_scale) < 0.0001f);
		assert(fabsf(output[1].h / base[1].h - reference_scale) < 0.0001f);
		float base_spacing = base[1].x - (base[0].x + base[0].w);
		float output_spacing = output[1].x - (output[0].x + output[0].w);
		assert(fabsf(output_spacing - base_spacing * reference_scale) < 0.0001f);
		float base_right = base[1].x + base[1].w;
		float output_right = output[1].x + output[1].w;
		if (phases[phase_index] == 0.0f) {
			assert(fabsf(output_right - base_right) < 0.0001f);
		} else {
			assert(output_right > base_right);
		}
		assert(fabsf(reference_scale -
			(1.0f + phases[phase_index] * (float)animation_limit / 50.0f)) <
		       0.0001f);
	}

	float maximum_scale;
	assert(bt_calculate_button_highlight_scale(&reference, 1.0f, &maximum_scale));
	SDL_FRect partial_base[] = {base[1]};
	SDL_FRect partial_output[1];
	assert(bt_calculate_highlight_group_rects_with_scale(
		partial_base, 1, maximum_scale, BT_HIGHLIGHT_ANCHOR_LEFT,
		partial_output));
	assert(fabsf(partial_output[0].x - partial_base[0].x) < 0.0001f);
	assert(fabsf(partial_output[0].w / partial_base[0].w - maximum_scale) <
	       0.0001f);
	assert(partial_output[0].x + partial_output[0].w >
	       partial_base[0].x + partial_base[0].w);
	assert(!bt_calculate_highlight_group_rects_with_scale(
		base, 2, NAN, BT_HIGHLIGHT_ANCHOR_LEFT, partial_output));
}

static bool region_has_non_black_pixel(SDL_Rect region)
{
	Uint32 *pixels = malloc((size_t)region.w * (size_t)region.h * sizeof(*pixels));
	assert(pixels != NULL);
	assert(SDL_RenderReadPixels(g_renderer, &region, SDL_PIXELFORMAT_RGBA32,
	                            pixels, region.w * (int)sizeof(*pixels)) == 0);
	bool found = false;
	for (int index = 0; index < region.w * region.h; index++) {
		Uint8 red, green, blue, alpha;
		SDL_GetRGBA(pixels[index], g_screen->format, &red, &green, &blue, &alpha);
		if ((red != 0 || green != 0 || blue != 0) && alpha != 0) {
			found = true;
			break;
		}
	}
	free(pixels);
	return found;
}

static void test_group_highlight_rendering(void)
{
	texture_t *instruction_texture = dw_create_text_tex("line", C_WHITE);
	texture_t *operand_texture = dw_create_text_tex("03:", C_WHITE);
	assert(instruction_texture != NULL && operand_texture != NULL);
	btn_t instruction = {
		.r = {.x = 100, .y = 300, .w = 50, .h = 24},
		.t = instruction_texture, .enabled = true
	};
	btn_t operand = {
		.r = {.x = 150, .y = 300, .w = 50, .h = 24},
		.t = operand_texture, .enabled = true
	};
	SDL_Rect original_instruction = instruction.r;
	SDL_Rect original_operand = operand.r;
	bt_group_draw_item_t forward[] = {
		{&instruction, false, false}, {&operand, false, false}
	};
	bt_group_draw_item_t reverse[] = {
		{&operand, false, false}, {&instruction, false, false}
	};
	const size_t pixel_count = (size_t)g_screen->pitch * (size_t)g_screen->h;
	unsigned char *forward_pixels = malloc(pixel_count);
	unsigned char *reverse_pixels = malloc(pixel_count);
	assert(forward_pixels != NULL && reverse_pixels != NULL);
	ha_reset();
	ms_init_mouse();
	for (int frame = 0; frame < 2 * get_cycle_frame_budget(); frame++) {
		ha_begin_frame();
		float phase = ha_get_progress();
		clear_test_renderer();
		assert(bt_draw_btn_group_with_highlight_progress(forward, 2, phase));
		assert(ha_get_progress() == phase);
		read_renderer_pixels(forward_pixels);
		clear_test_renderer();
		assert(bt_draw_btn_group_with_highlight_progress(reverse, 2, phase));
		assert(ha_get_progress() == phase);
		read_renderer_pixels(reverse_pixels);
		assert(memcmp(forward_pixels, reverse_pixels, pixel_count) == 0);
		assert(!instruction.animated && instruction.anim_state == 0);
		assert(!operand.animated && operand.anim_state == 0);
		assert_rect_equal(instruction.r, original_instruction);
		assert_rect_equal(operand.r, original_operand);
	}

	clear_test_renderer();
	float scale;
	assert(bt_calculate_button_highlight_scale(&instruction, 1.0f, &scale));
	assert(bt_draw_btn_group_with_highlight_scale(
		forward, 2, scale, BT_HIGHLIGHT_ANCHOR_LEFT));
	assert(region_has_non_black_pixel((SDL_Rect){90, 285, 60, 50}));
	assert(region_has_non_black_pixel((SDL_Rect){150, 285, 65, 50}));
	bt_group_draw_item_t instruction_only[] = {{&instruction, false, false}};
	bt_group_draw_item_t operand_only[] = {{&operand, false, false}};
	assert(bt_draw_btn_group_with_highlight_progress(instruction_only, 1, 0.5f));
	assert(bt_draw_btn_group_with_highlight_progress(operand_only, 1, 0.5f));
	assert(!bt_draw_btn_group_with_highlight_progress(NULL, 0, 0.5f));
	bt_group_draw_item_t partial_group[] = {
		{NULL, false, false}, {&operand, false, false}
	};
	assert(bt_draw_btn_group_with_highlight_progress(partial_group, 2, 0.5f));
	bt_group_draw_item_t empty_group[] = {{NULL, false, false}, {NULL, true, true}};
	assert(bt_draw_btn_group_with_highlight_progress(empty_group, 2, 0.5f));
	bt_group_draw_item_t duplicate[] = {
		{&instruction, false, false}, {&instruction, false, false}
	};
	clear_test_renderer();
	assert(bt_draw_btn_group_with_highlight_progress(duplicate, 2, 0.5f));
	assert(region_has_non_black_pixel((SDL_Rect){95, 285, 60, 50}));
	free(forward_pixels);
	free(reverse_pixels);
	dw_free_texture(instruction_texture);
	dw_free_texture(operand_texture);
}

static void assert_tinted_label_colors(SDL_Color immediate_color,
	                                  SDL_Color flag_color)
{
	float animation_limit = (float)dw_get_animation_limit();
	clear_test_renderer();
	im_draw_imm(animation_limit);
	assert_region_contains_color(im_get_label_bounds(), immediate_color);
	for (int index = 0; index < 20; index++) {
		vb_value_box_t value = im_get_imm_value_box_by_id(IMMUP0 + index);
		SDL_Rect text_region = value.box;
		text_region.x += text_region.w / 4;
		text_region.y += text_region.h / 4;
		text_region.w /= 2;
		text_region.h /= 2;
		assert_region_contains_color(text_region, C_WHITE);
		Uint8 red, green, blue;
		assert(SDL_GetTextureColorMod(value.t->texture, &red, &green, &blue) == 0);
		assert(red == C_WHITE.r && green == C_WHITE.g && blue == C_WHITE.b);
	}
	clear_test_renderer();
	rg_draw_flag_boxes(animation_limit);
	SDL_Rect label = rg_get_zero_flag_bounds();
	label.h = cl_get_code_button_size().h;
	label.y += label.h;
	label.w = vb_get_size().w;
	assert_region_contains_color(label, flag_color);
}

static void test_highlight_foregrounds_and_normal_tints(void)
{
	clear_highlight_targets();
	rg_init_flag_and_vboxes();
	im_init_imm_assets();
	im_set_imm_up_avail(true);
	assert_tinted_label_colors(C_AMBER, C_GREY);

	const ui_highlight_source_t sources[] = {
		UI_HIGHLIGHT_SOURCE_TUTORIAL, UI_HIGHLIGHT_SOURCE_AVAILABILITY
	};
	for (size_t index = 0; index < sizeof(sources) / sizeof(sources[0]); index++) {
		ui_highlight_source_t source = sources[index];
		ui_highlight_source_t other = sources[1 - index];
		bf_set_buffer_highlight_source(source, true, true);
		bf_set_buffer_highlight_source(other, true, true);
		bf_set_buffer_highlight_source(other, false, false);
		im_set_highlight_source(source, true);
		rg_set_zero_flag_highlight_source(source, true);
		im_set_highlight_source(other, true);
		rg_set_zero_flag_highlight_source(other, true);
		im_set_highlight_source(other, false);
		rg_set_zero_flag_highlight_source(other, false);
		ha_reset();
		for (int frame = 0; frame < get_cycle_frame_budget(); frame++) {
			ha_begin_frame();
			float phase = ha_get_progress();
			assert_tinted_label_colors(C_WHITE, C_WHITE);
			assert(ha_get_progress() == phase);
			float buffer_phase = -1.0f;
			float animation_limit = (float)dw_get_animation_limit();
			assert(bf_get_input_buffer_highlight_progress(&buffer_phase, animation_limit));
			assert(buffer_phase == phase);
			assert(bf_get_output_buffer_highlight_progress(&buffer_phase, animation_limit));
			assert(buffer_phase == phase);
		}
		bf_set_buffer_highlight_source(source, false, false);
		float buffer_phase = -1.0f;
		float animation_limit = (float)dw_get_animation_limit();
		assert(!bf_get_input_buffer_highlight_progress(&buffer_phase, animation_limit));
		assert(!bf_get_output_buffer_highlight_progress(&buffer_phase, animation_limit));
		im_set_highlight_source(source, false);
		rg_set_zero_flag_highlight_source(source, false);
		assert_tinted_label_colors(C_AMBER, C_GREY);
	}
	im_destroy_imm_assets();
	rg_destroy_value_boxes();
	clear_highlight_targets();
}

static void assert_pending_draw_state(int state, int expected_offset)
{
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
	btn_t operand_buttons[2] = {0};
	operand_t operands[2] = {0};
	g_pending_fixture.line.op1 = &operands[0];
	g_pending_fixture.line.op2 = &operands[1];
	ms_init_mouse();
	for (int position = 0; position < 2; position++) {
		int offset = position == 0 ? op1_offset : op2_offset;
		btn_t *button = &operand_buttons[position];
		operands[position].b = button;
		button->r = g_pending_fixture.instruction_button.r;
		button->r.x += offset;
		button->enabled = true;
		button->animated = true;
		button->anim_dir = true;
		button->anim_state = 8;
		SDL_Rect original_rect = button->r;
		SDL_Event event = {0};
		event.type = SDL_MOUSEBUTTONUP;
		event.button.button = SDL_BUTTON_LEFT;
		event.button.state = SDL_RELEASED;
		event.button.x = button->r.x + button->r.w - 5;
		event.button.y = button->r.y + 5;
		ms_mouse_button_handler(event);
		cw_change_clicked_code_line_state();
		int expected_state = position == 0 ? CHANGING_OP1 : CHANGING_OP2;
		assert(g_pending_fixture.line.state == expected_state);
		ha_reset();
		SDL_Rect first_rect;
		assert(cw_calculate_pending_operand_rect(original_rect, ha_get_progress(),
		                                       dw_get_animation_limit(), &first_rect));
		bool geometry_changed = false;
		for (int frame = 0; frame < get_cycle_frame_budget(); frame++) {
			ha_begin_frame();
			SDL_Rect current_rect;
			assert(cw_calculate_pending_operand_rect(original_rect, ha_get_progress(),
			                                       dw_get_animation_limit(), &current_rect));
			if (current_rect.w != first_rect.w || current_rect.h != first_rect.h) {
				geometry_changed = true;
			}
			assert_pending_draw_state(expected_state, offset);
			assert(!button->animated && !button->anim_dir);
			assert(button->anim_state == 0);
			assert_rect_equal(button->r, original_rect);
		}
		assert(geometry_changed);
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
	test_fixture_tutorial_transition();
	test_peaks_directions_and_target_removal();
	test_source_ownership_and_inactive_clock();
	test_zero_flag_source_ownership_and_progress();
	test_button_highlights_use_shared_phase();
	test_group_highlight_geometry();
	test_reference_scale_left_anchored_group();
	test_group_highlight_rendering();
	test_getters_preserve_frame_snapshot();
	test_invalid_getter_inputs();
	test_button_foreground_override();
	test_highlight_foregrounds_and_normal_tints();
	test_pending_rect_geometry_helper();
	test_pending_operand_uses_shared_phase();
	destroy_test_graphics();
	puts("highlight animation synchronization tests passed");
	return 0;
}
