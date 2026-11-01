#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include <SDL.h>

#include "aux.h"
#include "buffers_bf.h"
#include "dimensions_dm.h"
#include "draw_dw.h"
#include "immediates_im.h"
#include "instruction_layout_il.h"
#include "registers_rg.h"
#include "sdl_config.h"
#include "sdl_config_internal.h"
#include "stage_buttons_sb.h"
#include "ui/escape_menu_em.h"

static bool rect_equal(SDL_Rect first, SDL_Rect second)
{
	return first.x == second.x && first.y == second.y &&
	       first.w == second.w && first.h == second.h;
}

static bool rect_inside(SDL_Rect inner, SDL_Rect outer)
{
	return inner.w >= 0 && inner.h >= 0 &&
	       inner.x >= outer.x && inner.y >= outer.y &&
	       inner.x + inner.w <= outer.x + outer.w &&
	       inner.y + inner.h <= outer.y + outer.h;
}

typedef struct display_transition_fixture {
	bool fullscreen;
	SDL_Rect saved_rect;
	SDL_Rect current_rect;
	int set_failures;
	int apply_failures;
	int set_calls;
	int restore_calls;
	int apply_calls;
	int refresh_calls;
	int rollback_logs;
} display_transition_fixture_t;

static int mock_set_fullscreen(void *context, bool enabled)
{
	display_transition_fixture_t *fixture = context;
	fixture->set_calls++;
	if (fixture->set_failures > 0) {
		fixture->set_failures--;
		return FAIL;
	}
	fixture->fullscreen = enabled;
	if (enabled) {
		fixture->current_rect = (SDL_Rect){.x = 0, .y = 0, .w = 3200, .h = 1800};
	}
	return SUCCESS;
}

static int mock_restore_windowed_rect(void *context)
{
	display_transition_fixture_t *fixture = context;
	fixture->restore_calls++;
	fixture->current_rect = fixture->saved_rect;
	return SUCCESS;
}

static int mock_apply_display_configuration(void *context)
{
	display_transition_fixture_t *fixture = context;
	fixture->apply_calls++;
	if (fixture->apply_failures > 0) {
		fixture->apply_failures--;
		return FAIL;
	}
	return SUCCESS;
}

static int mock_refresh_display_metrics(void *context)
{
	display_transition_fixture_t *fixture = context;
	fixture->refresh_calls++;
	return SUCCESS;
}

static void mock_log_rollback_failure(void *context, const char *operation)
{
	display_transition_fixture_t *fixture = context;
	assert(operation != NULL && operation[0] != '\0');
	fixture->rollback_logs++;
}

static sdl_fullscreen_transition_operations_t mock_operations(
	display_transition_fixture_t *fixture)
{
	return (sdl_fullscreen_transition_operations_t){
		.context = fixture,
		.set_fullscreen = mock_set_fullscreen,
		.restore_windowed_rect = mock_restore_windowed_rect,
		.apply_display_configuration = mock_apply_display_configuration,
		.refresh_display_metrics = mock_refresh_display_metrics,
		.log_rollback_failure = mock_log_rollback_failure
	};
}

static void assert_logical_canvas_unchanged(void)
{
	assert(dm_get_logical_width() == 1920);
	assert(dm_get_logical_height() == 1080);
}

static void test_fullscreen_transition_round_trip(void)
{
	display_transition_fixture_t fixture = {
		.saved_rect = {.x = 42, .y = 68, .w = 1280, .h = 720},
		.current_rect = {.x = 42, .y = 68, .w = 1280, .h = 720}
	};
	sdl_fullscreen_transition_operations_t operations = mock_operations(&fixture);
	assert_logical_canvas_unchanged();
	assert(sdl_run_fullscreen_transition(false, true, &operations) == SUCCESS);
	assert(fixture.fullscreen);
	assert_logical_canvas_unchanged();
	assert(fixture.apply_calls == 1 && fixture.refresh_calls == 1);

	assert(sdl_run_fullscreen_transition(true, false, &operations) == SUCCESS);
	assert(!fixture.fullscreen);
	assert(rect_equal(fixture.current_rect, fixture.saved_rect));
	assert_logical_canvas_unchanged();
	assert(fixture.restore_calls == 1);
	assert(fixture.apply_calls == 2 && fixture.refresh_calls == 2);
	assert(fixture.rollback_logs == 0);
}

static void test_fullscreen_transition_rolls_back_configuration_failure(void)
{
	display_transition_fixture_t fixture = {
		.saved_rect = {.x = 15, .y = 25, .w = 1440, .h = 810},
		.current_rect = {.x = 15, .y = 25, .w = 1440, .h = 810},
		.apply_failures = 1
	};
	sdl_fullscreen_transition_operations_t operations = mock_operations(&fixture);
	assert(sdl_run_fullscreen_transition(false, true, &operations) == FAIL);
	assert(!fixture.fullscreen);
	assert(rect_equal(fixture.current_rect, fixture.saved_rect));
	assert(fixture.set_calls == 2 && fixture.restore_calls == 1);
	assert(fixture.apply_calls == 2 && fixture.refresh_calls == 1);
	assert(fixture.rollback_logs == 0);
	assert_logical_canvas_unchanged();
}

static void test_fullscreen_transition_failure_paths(void)
{
	display_transition_fixture_t set_failure = {
		.saved_rect = {.x = 8, .y = 9, .w = 1000, .h = 600},
		.current_rect = {.x = 8, .y = 9, .w = 1000, .h = 600},
		.set_failures = 1
	};
	sdl_fullscreen_transition_operations_t operations = mock_operations(&set_failure);
	assert(sdl_run_fullscreen_transition(false, true, &operations) == FAIL);
	assert(!set_failure.fullscreen && set_failure.set_calls == 1);
	assert(set_failure.apply_calls == 0 && set_failure.refresh_calls == 0);
	assert(rect_equal(set_failure.current_rect, set_failure.saved_rect));

	display_transition_fixture_t rollback_failure = {
		.saved_rect = {.x = 11, .y = 13, .w = 1024, .h = 576},
		.current_rect = {.x = 11, .y = 13, .w = 1024, .h = 576},
		.apply_failures = 2
	};
	operations = mock_operations(&rollback_failure);
	assert(sdl_run_fullscreen_transition(false, true, &operations) == FAIL);
	assert(!rollback_failure.fullscreen);
	assert(rect_equal(rollback_failure.current_rect, rollback_failure.saved_rect));
	assert(rollback_failure.refresh_calls == 1);
	assert(rollback_failure.rollback_logs == 1);
	assert_logical_canvas_unchanged();

	display_transition_fixture_t exit_failure = {
		.fullscreen = true,
		.saved_rect = {.x = 21, .y = 34, .w = 1360, .h = 765},
		.current_rect = {.x = 0, .y = 0, .w = 3200, .h = 1800},
		.apply_failures = 1
	};
	operations = mock_operations(&exit_failure);
	assert(sdl_run_fullscreen_transition(true, false, &operations) == FAIL);
	assert(exit_failure.fullscreen);
	assert(exit_failure.current_rect.x == 0 && exit_failure.current_rect.y == 0);
	assert(exit_failure.restore_calls == 1);
	assert(exit_failure.apply_calls == 2 && exit_failure.refresh_calls == 1);
	assert(exit_failure.rollback_logs == 0);
	assert_logical_canvas_unchanged();
}

static void test_fullscreen_argument_parsing(void)
{
	char *windowed[] = {"assemblyArchitect"};
	char *short_fullscreen[] = {"assemblyArchitect", "-f"};
	char *long_fullscreen[] = {"assemblyArchitect", "--fullscreen"};
	char *unrelated[] = {"assemblyArchitect", "--help", "-x"};
	char *later_fullscreen[] = {"assemblyArchitect", "--help", "-f"};

	assert(!sdl_fullscreen_requested(1, windowed));
	assert(sdl_fullscreen_requested(2, short_fullscreen));
	assert(sdl_fullscreen_requested(2, long_fullscreen));
	assert(!sdl_fullscreen_requested(3, unrelated));
	assert(sdl_fullscreen_requested(3, later_fullscreen));
	assert(!sdl_fullscreen_requested(0, NULL));
}

static void assert_minimum_size(int preferred_width, int preferred_height,
	                            int usable_width, int usable_height,
	                            int expected_width, int expected_height)
{
	sdl_window_size_limits_t result;
	assert(sdl_calculate_adaptive_minimum_size(
		preferred_width, preferred_height,
		(SDL_Rect){.w = usable_width, .h = usable_height}, &result));
	assert(result.minimum_width == expected_width);
	assert(result.minimum_height == expected_height);
	assert(result.minimum_width > 0 && result.minimum_height > 0);
	assert(result.minimum_width <= preferred_width);
	assert(result.minimum_height <= preferred_height);
	assert(result.minimum_width <= usable_width);
	assert(result.minimum_height <= usable_height);
}

static void test_adaptive_minimum_size_calculation(void)
{
	assert_minimum_size(1280, 720, 1920, 1040, 1280, 720);
	assert_minimum_size(1280, 720, 1280, 720, 1280, 720);
	assert_minimum_size(1280, 720, 1280, 680, 1209, 680);
	assert_minimum_size(1280, 720, 1024, 768, 1024, 576);
	assert_minimum_size(1280, 720, 800, 600, 800, 450);
	assert_minimum_size(1280, 720, 600, 900, 600, 338);
	assert_minimum_size(1280, 720, 1, 1, 1, 1);
	assert_minimum_size(1280, 720, INT_MAX, INT_MAX, 1280, 720);
	assert_minimum_size(INT_MAX, INT_MAX, INT_MAX - 1, INT_MAX - 1,
	                    INT_MAX - 1, INT_MAX - 1);

	sdl_window_size_limits_t result;
	SDL_Rect usable = {.w = 1280, .h = 720};
	assert(!sdl_calculate_adaptive_minimum_size(1280, 720, usable, NULL));
	assert(!sdl_calculate_adaptive_minimum_size(0, 720, usable, &result));
	assert(!sdl_calculate_adaptive_minimum_size(-1, 720, usable, &result));
	assert(!sdl_calculate_adaptive_minimum_size(1280, 0, usable, &result));
	assert(!sdl_calculate_adaptive_minimum_size(1280, -1, usable, &result));
	assert(!sdl_calculate_adaptive_minimum_size(
		1280, 720, (SDL_Rect){.w = 0, .h = 720}, &result));
	assert(!sdl_calculate_adaptive_minimum_size(
		1280, 720, (SDL_Rect){.w = -1, .h = 720}, &result));
	assert(!sdl_calculate_adaptive_minimum_size(
		1280, 720, (SDL_Rect){.w = 1280, .h = 0}, &result));
	assert(!sdl_calculate_adaptive_minimum_size(
		1280, 720, (SDL_Rect){.w = 1280, .h = -1}, &result));
	assert(dm_get_logical_width() == 1920);
	assert(dm_get_logical_height() == 1080);
}

static void test_physical_resize_preserves_logical_layout(void)
{
	SDL_Rect canvas = {.x = 0, .y = 0, .w = 1920, .h = 1080};
	SDL_Rect geometry[] = {
		dw_get_iface_big_center_box(),
		em_get_button_rect(EM_BUTTON_RESUME),
		em_get_button_rect(EM_BUTTON_FULLSCREEN),
		em_get_button_rect(EM_BUTTON_EXIT),
		sb_get_stage_btns(),
		im_get_grid_bounds(),
		il_get_initial_instruction_bounds(),
		rg_get_register_box(),
		bf_get_input_buffer_bounds(),
		bf_get_output_buffer_bounds()
	};
	for (size_t index = 0; index < 8; index++) {
		assert(rect_inside(geometry[index], canvas));
	}
	assert(rect_inside(geometry[1], geometry[0]));
	assert(rect_inside(geometry[2], geometry[0]));
	assert(rect_inside(geometry[3], geometry[0]));

	static const int physical_sizes[][2] = {
		{1920, 1080}, {1600, 900}, {1366, 768}, {1280, 720},
		{1024, 768}, {1920, 1200}, {2560, 1440}, {3440, 1440}
	};
	for (size_t index = 0;
	     index < sizeof(physical_sizes) / sizeof(physical_sizes[0]); index++) {
		assert(sdl_handle_window_size_changed(physical_sizes[index][0],
		                                      physical_sizes[index][1]) == SUCCESS);
		assert(dm_get_logical_width() == 1920);
		assert(dm_get_logical_height() == 1080);
		assert(sdl_is_fullscreen() == false);
		for (size_t rect_index = 0;
		     rect_index < sizeof(geometry) / sizeof(geometry[0]); rect_index++) {
			SDL_Rect current;
			switch (rect_index) {
			case 0: current = dw_get_iface_big_center_box(); break;
			case 1: current = em_get_button_rect(EM_BUTTON_RESUME); break;
			case 2: current = em_get_button_rect(EM_BUTTON_FULLSCREEN); break;
			case 3: current = em_get_button_rect(EM_BUTTON_EXIT); break;
			case 4: current = sb_get_stage_btns(); break;
			case 5: current = im_get_grid_bounds(); break;
			case 6: current = il_get_initial_instruction_bounds(); break;
			case 7: current = rg_get_register_box(); break;
			case 8: current = bf_get_input_buffer_bounds(); break;
			default: current = bf_get_output_buffer_bounds(); break;
			}
			assert(rect_equal(current, geometry[rect_index]));
		}
	}
	assert(sdl_handle_window_size_changed(0, 768) == FAIL);
	assert(dm_get_logical_width() == 1920);
	assert(dm_get_logical_height() == 1080);
	assert(sdl_set_fullscreen(false) == FAIL);
	assert(dm_get_logical_width() == 1920);
	assert(dm_get_logical_height() == 1080);
}

int main(void)
{
	assert(dm_get_logical_width() == DM_LOGICAL_CANVAS_WIDTH);
	assert(dm_get_logical_height() == DM_LOGICAL_CANVAS_HEIGHT);
	test_fullscreen_argument_parsing();
	test_adaptive_minimum_size_calculation();
	test_physical_resize_preserves_logical_layout();
	test_fullscreen_transition_round_trip();
	test_fullscreen_transition_rolls_back_configuration_failure();
	test_fullscreen_transition_failure_paths();
	puts("display resize tests passed");
	return 0;
}
