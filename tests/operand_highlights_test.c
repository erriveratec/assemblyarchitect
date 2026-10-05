#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_ttf.h>

#include "buffers_bf.h"
#include "dimensions_dm.h"
#include "gameplay/level_presentation_lp.h"
#include "gameplay/operand_highlights_oh.h"
#include "gameplay/ui_highlight_source.h"
#include "immediates_im.h"
#include "mouse_ms.h"
#include "registers_rg.h"
#include "sdl_config.h"
#include "tutorial_tr.h"
#include "tutorial_tr_internal.h"

static void init_test_graphics(void)
{
	assert(SDL_Init(0) == 0);
	assert(TTF_Init() == 0);
	dm_set_screen_resolution(R1920X1080);
	g_screen = SDL_CreateRGBSurfaceWithFormat(
	    0, dm_get_screen_width(), dm_get_screen_height(), 32,
	    SDL_PIXELFORMAT_RGBA32);
	assert(g_screen != NULL);
	g_renderer = SDL_CreateSoftwareRenderer(g_screen);
	assert(g_renderer != NULL);
	g_font = TTF_OpenFont("fonts/DOSVGA437.ttf", 130);
	assert(g_font != NULL);
}

static void clear_test_renderer(void)
{
	SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
	assert(SDL_RenderClear(g_renderer) == 0);
}

static void capture_number_bounds(const SDL_Rect *cells, SDL_Rect *bounds,
								 SDL_Color color)
{
	Uint32 *pixels = malloc((size_t)g_screen->pitch * g_screen->h);
	assert(pixels != NULL);
	assert(SDL_RenderReadPixels(g_renderer, NULL, g_screen->format->format,
	                           pixels, g_screen->pitch) == 0);
	Uint32 expected = SDL_MapRGBA(g_screen->format, color.r, color.g, color.b,
	                             color.a);
	int pitch_pixels = g_screen->pitch / (int)sizeof(*pixels);

	for (int index = 0; index < 20; index++) {
		int min_x = cells[index].x + cells[index].w;
		int min_y = cells[index].y + cells[index].h;
		int max_x = cells[index].x;
		int max_y = cells[index].y;
		for (int y = cells[index].y; y < cells[index].y + cells[index].h; y++) {
			for (int x = cells[index].x; x < cells[index].x + cells[index].w; x++) {
				if (pixels[y * pitch_pixels + x] == expected) {
					if (x < min_x) min_x = x;
					if (y < min_y) min_y = y;
					if (x > max_x) max_x = x;
					if (y > max_y) max_y = y;
				}
			}
		}
		assert(min_x <= max_x && min_y <= max_y);
		bounds[index] = (SDL_Rect){
			.x = min_x,
			.y = min_y,
			.w = max_x - min_x + 1,
			.h = max_y - min_y + 1
		};
	}
	free(pixels);
}

static void test_operand_availability(void)
{
	int register_ids[] = {RAX, RBX};
	operand_availability_t availability = oh_get_operand_availability(
	    NULL, register_ids, sizeof(register_ids) / sizeof(register_ids[0]),
	    true, true, true);
	assert(!availability.registers && !availability.input_buffer &&
	       !availability.output_buffer && !availability.immediates);

	instruction_t instruction = {.id = MOV};
	code_line_t line = {.ins = &instruction, .state = MISSING_BOTH};
	availability = oh_get_operand_availability(
	    &line, register_ids, sizeof(register_ids) / sizeof(register_ids[0]),
	    true, true, true);
	assert(availability.registers);
	assert(!availability.input_buffer);
	assert(availability.output_buffer);
	assert(!availability.immediates);

	operand_t source = {.id = RAX};
	line.op1 = &source;
	line.state = MISSING_OP2;
	availability = oh_get_operand_availability(
	    &line, register_ids, sizeof(register_ids) / sizeof(register_ids[0]),
	    true, true, true);
	assert(availability.registers);
	assert(availability.input_buffer);
	assert(!availability.output_buffer);
	assert(availability.immediates);

	instruction.id = ADD;
	im_set_imm_up_avail(true);
	availability = oh_get_operand_availability(
	    &line, register_ids, sizeof(register_ids) / sizeof(register_ids[0]),
	    true, true, im_are_imm_up_available());
	assert(availability.immediates);
	availability = oh_get_operand_availability(
	    &line, register_ids, sizeof(register_ids) / sizeof(register_ids[0]),
	    true, true, false);
	assert(!availability.immediates);
	im_set_imm_up_avail(false);
	availability = oh_get_operand_availability(
	    &line, register_ids, sizeof(register_ids) / sizeof(register_ids[0]),
	    true, true, im_are_imm_up_available());
	assert(!availability.immediates);
	im_set_imm_up_avail(true);

	availability = oh_get_operand_availability(
	    &line, register_ids, sizeof(register_ids) / sizeof(register_ids[0]),
	    false, false, false);
	assert(!availability.registers);
	assert(!availability.input_buffer);
	assert(!availability.output_buffer);
	assert(!availability.immediates);

	instruction.id = JMP;
	availability = oh_get_operand_availability(
	    &line, register_ids, sizeof(register_ids) / sizeof(register_ids[0]),
	    true, true, true);
	assert(!availability.registers);
	assert(!availability.input_buffer);
	assert(!availability.output_buffer);
	assert(!availability.immediates);
}

static void test_highlight_source_composition(void)
{
	im_set_highlight_source(UI_HIGHLIGHT_SOURCE_TUTORIAL, false);
	im_set_highlight_source(UI_HIGHLIGHT_SOURCE_AVAILABILITY, false);
	assert(!im_are_immediates_highlighted());

	im_set_highlight_source(UI_HIGHLIGHT_SOURCE_TUTORIAL, true);
	assert(im_are_immediates_highlighted());
	im_set_highlight_source(UI_HIGHLIGHT_SOURCE_TUTORIAL, false);
	assert(!im_are_immediates_highlighted());

	im_set_highlight_source(UI_HIGHLIGHT_SOURCE_AVAILABILITY, true);
	assert(im_are_immediates_highlighted());
	im_set_highlight_source(UI_HIGHLIGHT_SOURCE_TUTORIAL, true);
	assert(im_are_immediates_highlighted());
	im_set_highlight_source(UI_HIGHLIGHT_SOURCE_TUTORIAL, false);
	assert(im_are_immediates_highlighted());
	im_set_highlight_source(UI_HIGHLIGHT_SOURCE_AVAILABILITY, false);
	assert(!im_are_immediates_highlighted());

	im_set_highlight_source(UI_HIGHLIGHT_SOURCE_TUTORIAL, true);
	im_set_highlight_source(UI_HIGHLIGHT_SOURCE_AVAILABILITY, true);
	im_set_highlight_source(UI_HIGHLIGHT_SOURCE_AVAILABILITY, false);
	assert(im_are_immediates_highlighted());
	im_set_highlight_source(UI_HIGHLIGHT_SOURCE_TUTORIAL, false);
	assert(!im_are_immediates_highlighted());
}

static void test_disabled_highlighting_clears_availability(void)
{
	float progress = 0.0f;
	rg_set_register_highlight_source(UI_HIGHLIGHT_SOURCE_AVAILABILITY, true);
	bf_set_buffer_highlight_source(UI_HIGHLIGHT_SOURCE_AVAILABILITY, true, true);
	im_set_highlight_source(UI_HIGHLIGHT_SOURCE_AVAILABILITY, true);
	im_set_highlight_source(UI_HIGHLIGHT_SOURCE_TUTORIAL, true);
	assert(rg_get_register_highlight_progress(&progress));
	assert(bf_get_input_buffer_highlight_progress(&progress));
	assert(bf_get_output_buffer_highlight_progress(&progress));
	assert(im_are_immediates_highlighted());

	oh_update(false, true);
	assert(!rg_get_register_highlight_progress(&progress));
	assert(!bf_get_input_buffer_highlight_progress(&progress));
	assert(!bf_get_output_buffer_highlight_progress(&progress));
	assert(im_are_immediates_highlighted());

	im_set_highlight_source(UI_HIGHLIGHT_SOURCE_TUTORIAL, false);
	assert(!im_are_immediates_highlighted());
}

static void test_tutorial_highlight_parsing(void)
{
	int targets = 0;
	char immediate[] = "immediate";
	assert(tr_parse_highlights(immediate, &targets));
	assert(targets == TUTORIAL_HIGHLIGHT_IMMEDIATES);

	char combined[] = "register,immediate";
	assert(tr_parse_highlights(combined, &targets));
	assert(targets == (TUTORIAL_HIGHLIGHT_REGISTERS |
	                   TUTORIAL_HIGHLIGHT_IMMEDIATES));

	char unknown[] = "immedate";
	assert(!tr_parse_highlights(unknown, &targets));

	assert(tr_load_level(10));
	const tutorial_step_t *step = tr_get_named_step("introduce_immediates");
	assert(step != NULL);
	assert(step->highlight_targets & TUTORIAL_HIGHLIGHT_IMMEDIATES);
	step = tr_get_named_step("explain_immediate_addition");
	assert(step != NULL);
	assert(step->highlight_targets & TUTORIAL_HIGHLIGHT_IMMEDIATES);
	tr_clear();
}

static void test_immediate_grid_geometry(void)
{
	im_init_imm_assets();
	SDL_Rect cell_size = dm_get_imm_cell_wh();
	SDL_Rect value_size = dm_get_value_box_wh();
	assert(cell_size.w > value_size.w);
	assert(cell_size.h > value_size.h);

	value_box_t cells[20];
	for (int index = 0; index < 20; index++) {
		cells[index] = im_get_imm_value_box_by_id(IMMUP0 + index);
		assert(cells[index].box.w == cell_size.w);
		assert(cells[index].box.h == cell_size.h);
		assert(cells[index].value == (index <= 10 ? index : 10 - index));
	}
	for (int index = 0; index < 10; index++) {
		assert(cells[index].box.x + cells[index].box.w ==
		       cells[index + 1].box.x);
		assert(cells[index].box.y == cells[index + 1].box.y);
	}
	for (int index = 11; index < 19; index++) {
		assert(cells[index].box.x + cells[index].box.w ==
		       cells[index + 1].box.x);
		assert(cells[index].box.y == cells[index + 1].box.y);
	}
	assert(cells[11].box.x == cells[0].box.x + cell_size.w);
	assert(cells[11].box.y == cells[0].box.y + cell_size.h);
	SDL_Rect cell_rects[20];
	SDL_Rect normal_bounds[20];
	SDL_Rect repeated_normal_bounds[20];
	SDL_Rect highlighted_bounds[20];
	SDL_Rect restored_bounds[20];
	for (int index = 0; index < 20; index++) {
		cell_rects[index] = cells[index].box;
	}

	SDL_Event release = {0};
	release.button.x = cells[19].box.x + cells[19].box.w - 2;
	release.button.y = cells[19].box.y + cells[19].box.h - 2;
	release.button.button = SDL_BUTTON_LEFT;
	release.button.state = SDL_RELEASED;
	ms_init_mouse();
	ms_mouse_button_handler(release);
	assert(im_ms_rel_in_upimm());

	im_set_imm_up_avail(true);
	clear_test_renderer();
	im_draw_imm();
	capture_number_bounds(cell_rects, normal_bounds, C_WHITE);
	clear_test_renderer();
	im_draw_imm();
	capture_number_bounds(cell_rects, repeated_normal_bounds, C_WHITE);
	for (int index = 0; index < 20; index++) {
		assert(normal_bounds[index].w == repeated_normal_bounds[index].w);
		assert(normal_bounds[index].h == repeated_normal_bounds[index].h);
	}

	im_set_highlight_source(UI_HIGHLIGHT_SOURCE_TUTORIAL, true);
	clear_test_renderer();
	im_draw_imm();
	clear_test_renderer();
	im_draw_imm();
	capture_number_bounds(cell_rects, highlighted_bounds, C_LIGHTGREY);
	for (int index = 0; index < 20; index++) {
		value_box_t current = im_get_imm_value_box_by_id(IMMUP0 + index);
		assert(current.box.x == cell_rects[index].x);
		assert(current.box.y == cell_rects[index].y);
		assert(current.box.w == cell_rects[index].w);
		assert(current.box.h == cell_rects[index].h);
		assert(highlighted_bounds[index].h > normal_bounds[index].h);
		int normal_center_x = 2 * normal_bounds[index].x + normal_bounds[index].w;
		int highlighted_center_x =
		    2 * highlighted_bounds[index].x + highlighted_bounds[index].w;
		int normal_center_y = 2 * normal_bounds[index].y + normal_bounds[index].h;
		int highlighted_center_y =
		    2 * highlighted_bounds[index].y + highlighted_bounds[index].h;
		assert(abs(normal_center_x - highlighted_center_x) <= 4);
		assert(abs(normal_center_y - highlighted_center_y) <= 4);
	}

	im_set_highlight_source(UI_HIGHLIGHT_SOURCE_TUTORIAL, false);
	clear_test_renderer();
	im_draw_imm();
	capture_number_bounds(cell_rects, restored_bounds, C_WHITE);
	for (int index = 0; index < 20; index++) {
		assert(restored_bounds[index].w == normal_bounds[index].w);
		assert(restored_bounds[index].h == normal_bounds[index].h);
	}
}

static void test_operand_highlight_configuration(void)
{
	assert(!lp_are_operand_highlights_enabled());
	lp_configure(false, false, false, false, true);
	assert(lp_are_operand_highlights_enabled());
	lp_configure(false, false, false, false, false);
	assert(!lp_are_operand_highlights_enabled());
}

int main(void)
{
	init_test_graphics();
	test_operand_availability();
	test_highlight_source_composition();
	test_disabled_highlighting_clears_availability();
	test_operand_highlight_configuration();
	test_tutorial_highlight_parsing();
	test_immediate_grid_geometry();
	TTF_CloseFont(g_font);
	SDL_DestroyRenderer(g_renderer);
	SDL_FreeSurface(g_screen);
	TTF_Quit();
	SDL_Quit();
	puts("operand highlight tests passed");
	return 0;
}