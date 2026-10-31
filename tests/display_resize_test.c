#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <SDL.h>

#include "aux.h"
#include "dimensions_dm.h"
#include "draw_dw.h"
#include "immediates_im.h"
#include "instruction_layout_il.h"
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
	assert(dm_get_screen_width() == 1920);
	assert(dm_get_screen_height() == 1080);
}

static void test_fullscreen_transition_round_trip(void)
{
	dm_set_screen_resolution(DM_RESOLUTION_1920X1080);
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
	dm_set_screen_resolution(DM_RESOLUTION_1920X1080);
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

static void test_physical_resize_preserves_logical_layout(void)
{
	dm_set_screen_resolution(DM_RESOLUTION_1920X1080);
	dm_set_logical_dimensions(1920, 1080);
	SDL_Rect canvas = {.x = 0, .y = 0, .w = 1920, .h = 1080};
	SDL_Rect geometry[] = {
		dw_get_iface_big_center_box(),
		em_get_button_rect(EM_BUTTON_RESUME),
		em_get_button_rect(EM_BUTTON_FULLSCREEN),
		em_get_button_rect(EM_BUTTON_EXIT),
		sb_get_stage_btns(),
		im_get_grid_bounds(),
		il_get_initial_instruction_bounds()
	};
	for (size_t index = 0; index < sizeof(geometry) / sizeof(geometry[0]); index++) {
		assert(rect_inside(geometry[index], canvas));
	}
	assert(rect_inside(geometry[1], geometry[0]));
	assert(rect_inside(geometry[2], geometry[0]));
	assert(rect_inside(geometry[3], geometry[0]));

	static const int physical_sizes[][2] = {
		{1366, 768}, {2560, 1440}, {1920, 1200}, {3440, 1440}
	};
	for (size_t index = 0;
	     index < sizeof(physical_sizes) / sizeof(physical_sizes[0]); index++) {
		assert(sdl_handle_window_size_changed(physical_sizes[index][0],
		                                      physical_sizes[index][1]) == SUCCESS);
		assert(dm_get_screen_width() == 1920);
		assert(dm_get_screen_height() == 1080);
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
			default: current = il_get_initial_instruction_bounds(); break;
			}
			assert(rect_equal(current, geometry[rect_index]));
		}
	}
	assert(sdl_handle_window_size_changed(0, 768) == FAIL);
	assert(dm_get_screen_width() == 1920);
	assert(dm_get_screen_height() == 1080);
	assert(sdl_set_fullscreen(false) == FAIL);
	assert(dm_get_screen_width() == 1920);
	assert(dm_get_screen_height() == 1080);
}

int main(void)
{
	test_fullscreen_argument_parsing();
	test_physical_resize_preserves_logical_layout();
	test_fullscreen_transition_round_trip();
	test_fullscreen_transition_rolls_back_configuration_failure();
	test_fullscreen_transition_failure_paths();
	puts("display resize tests passed");
	return 0;
}
