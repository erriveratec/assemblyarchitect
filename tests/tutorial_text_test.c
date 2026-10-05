#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include <SDL_ttf.h>

#include "dimensions_dm.h"
#include "ui/ui_metrics_um.h"
#include "draw_dw.h"
#include "aux.h"
#include "sdl_config.h"
#include "text_tx.h"

static tx_text_box_options_t g_options = {
	.position = TX_CENTER_BOX,
	.large_box = true,
	.large_text = false,
};

static tx_styled_text_t *create_message(const char *text)
{
	tx_styled_text_t *message =
	    tx_create_styled_text_box_message(&g_options, text, NULL);
	assert(message != NULL);
	return message;
}

static bool fragment_has_color(const tx_text_fragment_t *fragment,
							   SDL_Color expected)
{
	texture_t *texture = fragment->texture;
	int text_height = um_message_text_height();
	int width = (int)((int64_t)texture->w * text_height / texture->h);
	SDL_Rect destination = {.x = 4, .y = 4, .w = width, .h = text_height};
	Uint32 *pixels = malloc((size_t)g_screen->pitch * g_screen->h);
	assert(pixels != NULL);
	SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
	assert(SDL_RenderClear(g_renderer) == 0);
	assert(dw_draw_texture_fit_h(destination, texture) == SUCCESS);
	assert(SDL_RenderReadPixels(g_renderer, NULL, g_screen->format->format,
	                           pixels, g_screen->pitch) == 0);
	bool found = false;
	for (int y = 0; y < g_screen->h && !found; y++) {
		for (int x = 0; x < g_screen->w; x++) {
			Uint8 red, green, blue, alpha;
			SDL_GetRGBA(pixels[(size_t)y * g_screen->w + x], g_screen->format,
			            &red, &green, &blue, &alpha);
			if (red == expected.r && green == expected.g &&
			    blue == expected.b && alpha == expected.a) {
				found = true;
				break;
			}
		}
	}
	free(pixels);
	return found;
}

static void assert_no_text_outside_content(SDL_Rect content)
{
	Uint32 *pixels = malloc((size_t)g_screen->pitch * g_screen->h);
	assert(pixels != NULL);
	assert(SDL_RenderReadPixels(g_renderer, NULL, g_screen->format->format,
	                           pixels, g_screen->pitch) == 0);
	const SDL_Color text_colors[] = {
		C_WHITE, C_TERMINAL_GREEN, C_AMBER, C_ORANGE};
	for (int y = 0; y < g_screen->h; y++) {
		for (int x = 0; x < g_screen->w; x++) {
			if (x >= content.x && x < content.x + content.w &&
			    y >= content.y && y < content.y + content.h) {
				continue;
			}
			Uint8 red, green, blue, alpha;
			SDL_GetRGBA(pixels[(size_t)y * g_screen->w + x], g_screen->format,
			            &red, &green, &blue, &alpha);
			for (size_t color = 0;
			     color < sizeof(text_colors) / sizeof(text_colors[0]); color++) {
				assert(red != text_colors[color].r ||
				       green != text_colors[color].g ||
				       blue != text_colors[color].b ||
				       alpha != text_colors[color].a);
			}
		}
	}
	free(pixels);
}

static void assert_color_sequence(const tx_styled_text_t *message,
								  const SDL_Color *expected, size_t expected_count)
{
	size_t color_index = 0;
	SDL_Color previous = {0};
	bool have_previous = false;
	for (int row_index = 0; row_index < message->row_count; row_index++) {
		const tx_styled_row_t *row = &message->rows[row_index];
		for (int fragment_index = 0; fragment_index < row->fragment_count;
		     fragment_index++) {
			const tx_text_fragment_t *fragment = &row->fragments[fragment_index];
			SDL_Color color = {0};
			const SDL_Color candidates[] = {
				C_WHITE, C_TERMINAL_GREEN, C_AMBER, C_ORANGE};
			bool classified = false;
			for (size_t candidate = 0;
			     candidate < sizeof(candidates) / sizeof(candidates[0]); candidate++) {
				if (fragment_has_color(fragment, candidates[candidate])) {
					color = candidates[candidate];
					classified = true;
					break;
				}
			}
			assert(classified);
			if (!have_previous || color.r != previous.r ||
			    color.g != previous.g || color.b != previous.b) {
				assert(color_index < expected_count);
				assert(color.r == expected[color_index].r &&
				       color.g == expected[color_index].g &&
				       color.b == expected[color_index].b);
				color_index++;
				previous = color;
				have_previous = true;
			}
		}
	}
	assert(color_index == expected_count);
}

static void test_inline_styles(void)
{
	const SDL_Color body_colors[] = {
		C_TERMINAL_GREEN, C_WHITE, C_TERMINAL_GREEN,
		C_WHITE, C_TERMINAL_GREEN};
	tx_styled_text_t *body = create_message("MOV transfers [ib] into rax");
	assert(body->row_count == 1);
	assert(body->rows[0].fragment_count == 5);
	assert_color_sequence(body, body_colors,
	                      sizeof(body_colors) / sizeof(body_colors[0]));
	tx_free_styled_text(body);

	const SDL_Color directive_colors[] = {
		C_AMBER, C_TERMINAL_GREEN, C_AMBER};
	tx_styled_text_t *directive =
	    create_message("@directive Assign rax operand");
	assert(directive->row_count == 1);
	assert_color_sequence(directive, directive_colors,
	                      sizeof(directive_colors) / sizeof(directive_colors[0]));
	tx_free_styled_text(directive);

	const SDL_Color warning_colors[] = {
		C_TERMINAL_GREEN, C_ORANGE, C_TERMINAL_GREEN, C_ORANGE};
	tx_styled_text_t *warning =
	    create_message("@warning [ob] remains empty after MOV execution");
	assert(warning->row_count >= 1);
	assert_color_sequence(warning, warning_colors,
	                      sizeof(warning_colors) / sizeof(warning_colors[0]));
	tx_free_styled_text(warning);

	tx_styled_text_t *syntax = create_message("@syntax MOV rax, [ib]");
	for (int row = 0; row < syntax->row_count; row++) {
		for (int fragment = 0; fragment < syntax->rows[row].fragment_count;
		     fragment++) {
			assert(fragment_has_color(&syntax->rows[row].fragments[fragment],
			                         C_TERMINAL_GREEN));
		}
	}
	tx_free_styled_text(syntax);
}

static void test_token_boundaries(void)
{
	const char *ordinary[] = {"MOVEMENT", "paraxial", "ib ob", "AZF"};
	for (size_t index = 0; index < sizeof(ordinary) / sizeof(ordinary[0]); index++) {
		tx_styled_text_t *message = create_message(ordinary[index]);
		assert(message->rows[0].fragment_count == 1);
		assert(fragment_has_color(&message->rows[0].fragments[0], C_WHITE));
		tx_free_styled_text(message);
	}
	tx_styled_text_t *unknown_prefix = create_message("@unknown MOVEMENT");
	assert(unknown_prefix->rows[0].fragment_count == 1);
	assert(fragment_has_color(&unknown_prefix->rows[0].fragments[0], C_WHITE));
	tx_free_styled_text(unknown_prefix);

	tx_styled_text_t *punctuated = create_message("rax,");
	assert(punctuated->rows[0].fragment_count == 2);
	assert(fragment_has_color(&punctuated->rows[0].fragments[0],
	                         C_TERMINAL_GREEN));
	assert(fragment_has_color(&punctuated->rows[0].fragments[1], C_WHITE));
	tx_free_styled_text(punctuated);

	tx_styled_text_t *adjacent = create_message("MOV[ib]");
	assert(adjacent->rows[0].fragment_count == 2);
	assert(fragment_has_color(&adjacent->rows[0].fragments[0],
	                         C_TERMINAL_GREEN));
	assert(fragment_has_color(&adjacent->rows[0].fragments[1],
	                         C_TERMINAL_GREEN));
	tx_free_styled_text(adjacent);

	tx_styled_text_t *spaced = create_message("MOV rax");
	assert(spaced->rows[0].fragment_count == 2);
	assert(spaced->rows[0].fragments[1].x_offset >
	       spaced->rows[0].fragments[0].x_offset);
	tx_free_styled_text(spaced);

	tx_styled_text_t *all_tokens =
	    create_message("MOV ADD line JMP CMP JE JNE rax rbx rcx rdx rdi [ib] [ob] ZF");
	for (int row = 0; row < all_tokens->row_count; row++) {
		for (int fragment = 0; fragment < all_tokens->rows[row].fragment_count;
		     fragment++) {
			assert(fragment_has_color(&all_tokens->rows[row].fragments[fragment],
			                         C_TERMINAL_GREEN));
		}
	}
	tx_free_styled_text(all_tokens);
}

static void test_blank_wrap_and_overflow(void)
{
	tx_styled_text_t *blank = create_message("first\n\nlast");
	assert(blank->row_count == 3);
	assert(blank->rows[1].fragment_count == 0);
	tx_free_styled_text(blank);

	SDL_Rect content;
	int text_height = 0;
	assert(tx_get_text_box_rects(&g_options, NULL, &content, &text_height));
	char wrapped_text[512] = {0};
	for (int index = 0; index < 8; index++) {
		strcat(wrapped_text, "prose MOV rax [ib] explanatory text ");
	}
	tx_styled_text_t *wrapped = create_message(wrapped_text);
	assert(wrapped->row_count > 1);
	for (int row = 0; row < wrapped->row_count; row++) {
		assert(wrapped->rows[row].rendered_width <= content.w);
	}
	tx_free_styled_text(wrapped);

	char long_word[401];
	memset(long_word, 'x', sizeof(long_word) - 1);
	long_word[sizeof(long_word) - 1] = '\0';
	tx_styled_text_t *long_message = create_message(long_word);
	assert(long_message->row_count > 1);
	for (int row = 0; row < long_message->row_count; row++) {
		assert(long_message->rows[row].rendered_width <= content.w);
	}
	tx_free_styled_text(long_message);

	char overflow_text[256] = {0};
	for (int index = 0; index < 20; index++) {
		strcat(overflow_text, "row");
		if (index != 19) {
			strcat(overflow_text, "\n");
		}
	}
	tx_text_layout_info_t layout = {0};
	tx_styled_text_t *overflow = tx_create_styled_text_box_message(
	    &g_options, overflow_text, &layout);
	assert(overflow != NULL);
	assert(layout.required_rows == 20);
	assert(layout.visible_rows == content.h / text_height);
	assert(layout.overflow_rows == layout.required_rows - layout.visible_rows);
	assert(layout.overflowed);
	SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
	assert(SDL_RenderClear(g_renderer) == 0);
	tx_draw_styled_text_box_message(&g_options, overflow, TX_NONE);
	assert(SDL_RenderIsClipEnabled(g_renderer) == SDL_FALSE);
	assert_no_text_outside_content(content);
	SDL_RenderPresent(g_renderer);
	tx_free_styled_text(overflow);
}

int main(void)
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

	test_inline_styles();
	test_token_boundaries();
	test_blank_wrap_and_overflow();

	TTF_CloseFont(g_font);
	SDL_DestroyRenderer(g_renderer);
	SDL_FreeSurface(g_screen);
	TTF_Quit();
	SDL_Quit();
	puts("tutorial text tests passed");
	return 0;
}