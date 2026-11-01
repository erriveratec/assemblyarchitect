#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <SDL.h>
#include <SDL_ttf.h>

#include "sdl_config.h"
#include "../source/ui/screens/select_level_sl.c"

static const int SHIPPED_HOVER_MESSAGE_COUNT = 17;

static void assert_rect_equal(SDL_Rect actual, SDL_Rect expected)
{
	assert(actual.x == expected.x);
	assert(actual.y == expected.y);
	assert(actual.w == expected.w);
	assert(actual.h == expected.h);
}

static void assert_message_complete(int level_id, SDL_Rect bounds,
	                                const char *expected)
{
	tx_styled_text_t *styled = tx_get_message_texture(level_id);
	assert(styled != NULL);
	assert(styled->row_count > 0);
	char reconstructed[512] = {0};
	size_t written = 0;
	for (int row = 0; row < styled->row_count; row++) {
		assert(styled->rows[row].rendered_width <= bounds.w);
		assert(styled->rows[row].test_text != NULL);
		size_t row_length = strlen(styled->rows[row].test_text);
		assert(row_length < sizeof(reconstructed) - written);
		memcpy(reconstructed + written, styled->rows[row].test_text,
		       row_length);
		written += row_length;
	}
	reconstructed[written] = '\0';
	assert(strcmp(reconstructed, expected) == 0);
}

static int test_canonical_hover_messages(void)
{
	SDL_Rect bounds;
	assert(get_level_hover_description_bounds(&bounds));
	assert(bounds.w > 0 && bounds.h > 0);
	assert(bounds.x >= 0 && bounds.y >= 0);
	assert((int64_t)bounds.x + bounds.w <= dm_get_screen_width());
	assert((int64_t)bounds.y + bounds.h <= dm_get_screen_height());
	int text_height = sc_get_selection_description_text_height();
	assert(bounds.h / text_height >= 1);
	SDL_Rect separator = get_sector_0_lower_separator();
	SDL_Rect first_button = get_level_button_box();
	int second_row_bottom = first_button.y + get_sel_level_offset_y() +
	                        first_button.h;
	assert(second_row_bottom <= separator.y);
	assert((int64_t)bounds.y >= (int64_t)separator.y + separator.h +
				   (int)SECTOR_TITLE_SEPARATOR);

	tx_styled_text_options_t options = {
		.highlight_arch_tokens = false,
		.parse_authoring_prefixes = false,
		.body_color = C_GREY
	};
	tx_styled_text_t *short_message = tx_create_styled_text_for_width(
	    bounds.w, text_height, "ARCHITECTURE INTRODUCTION", &options);
	assert(short_message != NULL);
	assert(short_message->row_count == 1);
	assert(short_message->rows[0].rendered_width <= bounds.w);
	tx_free_styled_text(short_message);

	const int sector_starts[] = {0, LV_SECTOR_1_START, LV_SECTOR_2_START};
	int maximum_row_count = 0;
	for (size_t sector = 0;
	     sector < sizeof(sector_starts) / sizeof(sector_starts[0]); sector++) {
		int first_level_id = sector_starts[sector];
		load_sector_hover_messages(first_level_id);
		int end_level_id = first_level_id + LV_SECTOR_LV_QTY;
		if (end_level_id > SHIPPED_HOVER_MESSAGE_COUNT)
			end_level_id = SHIPPED_HOVER_MESSAGE_COUNT;
		for (int level_id = first_level_id; level_id < end_level_id; level_id++) {
			char expected[512];
			assert(lc_load_hover_message(level_id, expected, sizeof(expected)) ==
			       SUCCESS);
			assert_message_complete(level_id, bounds, expected);
			tx_styled_text_t *styled = tx_get_message_texture(level_id);
			assert(styled != NULL);
			assert(styled->row_count <= bounds.h / text_height);
			if (styled->row_count > maximum_row_count)
				maximum_row_count = styled->row_count;
		}
	}
	assert(maximum_row_count > 1);
	assert(bounds.h / text_height >= maximum_row_count);

	tx_styled_text_t *draw_message =
	    tx_get_message_texture(LV_SECTOR_2_START);
	assert(draw_message != NULL);
	assert(SDL_RenderSetClipRect(g_renderer, NULL) == 0);
	tx_draw_styled_text(bounds, text_height, draw_message, 0);
	assert(SDL_RenderIsClipEnabled(g_renderer) == SDL_FALSE);

	SDL_Rect prior_clip = {
		.x = bounds.x + 5,
		.y = bounds.y + 5,
		.w = bounds.w / 2,
		.h = bounds.h / 2
	};
	assert(SDL_RenderSetClipRect(g_renderer, &prior_clip) == 0);
	tx_draw_styled_text(bounds, text_height, draw_message, 0);
	assert(SDL_RenderIsClipEnabled(g_renderer) == SDL_TRUE);
	SDL_Rect restored_clip;
	SDL_RenderGetClipRect(g_renderer, &restored_clip);
	assert_rect_equal(restored_clip, prior_clip);
	assert(SDL_RenderSetClipRect(g_renderer, NULL) == 0);
	tx_free_level_text_textures();
	printf("%dx%d hover bounds: x=%d y=%d w=%d h=%d; max rows=%d\n",
	       dm_get_screen_width(), dm_get_screen_height(), bounds.x, bounds.y,
	       bounds.w, bounds.h, maximum_row_count);
	return maximum_row_count;
}

static void test_canonical_hover_bounds(void)
{
	SDL_Rect bounds;
	assert(get_level_hover_description_bounds(&bounds));
	assert(bounds.h >= sc_get_selection_description_text_height());
	assert(bounds.x >= 0 && bounds.y >= 0);
	assert((int64_t)bounds.x + bounds.w <= dm_get_screen_width());
	assert((int64_t)bounds.y + bounds.h <= dm_get_screen_height());

}

int main(void)
{
	assert(SDL_Init(0) == 0);
	assert(TTF_Init() == 0);
	g_screen = SDL_CreateRGBSurfaceWithFormat(
	    0, dm_get_screen_width(), dm_get_screen_height(), 32,
	    SDL_PIXELFORMAT_RGBA32);
	assert(g_screen != NULL);
	g_renderer = SDL_CreateSoftwareRenderer(g_screen);
	assert(g_renderer != NULL);
	g_font = TTF_OpenFont("fonts/DOSVGA437.ttf", 130);
	assert(g_font != NULL);

	assert(dm_get_screen_width() == DM_LOGICAL_CANVAS_WIDTH);
	assert(dm_get_screen_height() == DM_LOGICAL_CANVAS_HEIGHT);
	int maximum_rows = test_canonical_hover_messages();
	assert(maximum_rows <= DM_LOGICAL_CANVAS_HEIGHT /
	       sc_get_selection_description_text_height());
	test_canonical_hover_bounds();

	TTF_CloseFont(g_font);
	SDL_DestroyRenderer(g_renderer);
	SDL_FreeSurface(g_screen);
	TTF_Quit();
	SDL_Quit();
	puts("level hover layout tests passed");
	return 0;
}