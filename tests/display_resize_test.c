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
	puts("display resize tests passed");
	return 0;
}
