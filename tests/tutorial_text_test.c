#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <SDL.h>
#include <SDL_ttf.h>

#include "dimensions_dm.h"
#include "ui/ui_metrics_um.h"
#include "ui/typography_ty.h"
#include "draw_dw.h"
#include "aux.h"
#include "sdl_config.h"
#include "text_tx.h"
#include "code_window_cw.h"
#include "code_window_cw_internal.h"
#include "tutorial_tr.h"
#include "tutorial_tr_internal.h"

extern tx_styled_text_t *g_challenge_text;
static SDL_Color fragment_color(const tx_text_fragment_t *fragment);
static bool fail_challenge_creation;
static int challenge_factory_calls;
static int challenge_factory_option_errors;
static char challenge_factory_text[128];

static tx_styled_text_t *test_challenge_text_factory(
	int width, int height, const char *text,
	const tx_styled_text_options_t *options)
{
	challenge_factory_calls++;
	if (width <= 0 || height <= 0 || text == NULL || options == NULL ||
	    options->highlight_arch_tokens || options->parse_authoring_prefixes ||
	    options->body_color.r != C_SILVERGREY.r ||
	    options->body_color.g != C_SILVERGREY.g ||
	    options->body_color.b != C_SILVERGREY.b ||
	    options->body_color.a != C_SILVERGREY.a) {
		challenge_factory_option_errors++;
	}
	snprintf(challenge_factory_text, sizeof(challenge_factory_text), "%s",
	         text != NULL ? text : "");
	if (fail_challenge_creation) {
		return NULL;
	}
	return tx_create_styled_text_for_width(width, height, text, options);
}

static size_t capture_challenge_diagnostic(const char *text, char *output,
											   size_t output_size)
{
	FILE *capture = tmpfile();
	assert(capture != NULL);
	fflush(stderr);
	int saved_stderr = dup(STDERR_FILENO);
	assert(saved_stderr >= 0);
	assert(dup2(fileno(capture), STDERR_FILENO) >= 0);
	cw_set_challenge_text(text);
	fflush(stderr);
	assert(dup2(saved_stderr, STDERR_FILENO) >= 0);
	close(saved_stderr);
	rewind(capture);
	size_t bytes_read = fread(output, 1, output_size - 1, capture);
	output[bytes_read] = '\0';
	fclose(capture);
	return bytes_read;
}

static bool reconstruct_styled_rows(const tx_styled_text_t *message,
									char *output, size_t output_size)
{
	if (message == NULL || output == NULL || output_size == 0) {
		return false;
	}
	size_t written = 0;
	for (int row = 0; row < message->row_count; row++) {
		const tx_styled_row_t *styled_row = &message->rows[row];
		if (styled_row->fragment_count < 0 ||
		    (styled_row->fragment_count > 0 && styled_row->fragments == NULL)) {
			return false;
		}
		for (int fragment = 0; fragment < styled_row->fragment_count; fragment++) {
			if (styled_row->fragments[fragment].texture == NULL) {
				return false;
			}
		}
#ifdef TX_STYLED_TEXT_TESTING
		if (styled_row->test_text == NULL) {
			return false;
		}
		/* The row source preserves spaces that intentionally have no texture. */
		size_t row_length = strlen(styled_row->test_text);
		if (row_length >= output_size - written) {
			return false;
		}
		memcpy(output + written, styled_row->test_text, row_length);
		written += row_length;
#else
		return false;
#endif
	}
	output[written] = '\0';
	return true;
}

static void assert_production_challenge_recovery(const char *text,
											 char *reconstructed,
											 size_t reconstructed_size)
{
	cw_reset_challenge_text_factory();
	int calls_before = challenge_factory_calls;
	cw_set_challenge_text(text);
	bool created = g_challenge_text != NULL;
	bool content_matches = created && reconstruct_styled_rows(
		g_challenge_text, reconstructed, reconstructed_size) &&
		strcmp(reconstructed, text) == 0;
	bool colors_match = created;
	if (created) {
		for (int row = 0; row < g_challenge_text->row_count; row++) {
			for (int fragment = 0;
			     fragment < g_challenge_text->rows[row].fragment_count; fragment++) {
				SDL_Color color = fragment_color(
				    &g_challenge_text->rows[row].fragments[fragment]);
				colors_match &= color.r == C_SILVERGREY.r &&
				                color.g == C_SILVERGREY.g &&
				                color.b == C_SILVERGREY.b &&
				                color.a == C_SILVERGREY.a;
			}
		}
	}
	bool factory_unchanged = challenge_factory_calls == calls_before;
	cw_set_challenge_text("");
	bool cleared = g_challenge_text == NULL;
	cw_reset_challenge_text_factory();
	assert(created);
	assert(content_matches);
	assert(colors_match);
	assert(factory_unchanged);
	assert(cleared);
}

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

static tx_styled_text_t *create_generic_message(const char *text,
												 SDL_Rect content,
												 const tx_styled_text_options_t *options)
{
	tx_styled_text_t *message = tx_create_styled_text(
		content, ty_message_height(), text, options, NULL);
	assert(message != NULL);
	return message;
}

static bool fragment_has_color(const tx_text_fragment_t *fragment,
							   SDL_Color expected)
{
	texture_t *texture = fragment->texture;
	int text_height = ty_message_height();
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
	const char *ordinary[] = {
		"MOVEMENT", "paraxial", "ib ob", "AZF", "myrax", "rax_value",
		"MOVable", "[ibx]", "[obx]", "ob", "[ib,", "[ib, ib]"
	};
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
	    create_message("MOV ADD JMP CMP JE JNE rax rbx rcx rdx rdi [ib] [ob] ZF");
	for (int row = 0; row < all_tokens->row_count; row++) {
		for (int fragment = 0; fragment < all_tokens->rows[row].fragment_count;
		     fragment++) {
			assert(fragment_has_color(&all_tokens->rows[row].fragments[fragment],
			                         C_TERMINAL_GREEN));
		}
	}
	tx_free_styled_text(all_tokens);

	const char *punctuated_tokens[] = {"rax,", "(rbx)", "[ib].", "[ob]:"};
	for (size_t index = 0;
	     index < sizeof(punctuated_tokens) / sizeof(punctuated_tokens[0]); index++) {
		tx_styled_text_t *message = create_message(punctuated_tokens[index]);
		bool has_green_token = false;
		for (int fragment = 0; fragment < message->rows[0].fragment_count;
		     fragment++) {
			has_green_token |= fragment_has_color(
			    &message->rows[0].fragments[fragment], C_TERMINAL_GREEN);
		}
		assert(has_green_token);
		tx_free_styled_text(message);
	}

	tx_styled_text_t *line_prose = create_message("line of code");
	assert(line_prose->rows[0].fragment_count == 1);
	assert(fragment_has_color(&line_prose->rows[0].fragments[0], C_WHITE));
	tx_free_styled_text(line_prose);
}

static void test_generic_mixed_color_messages(void)
{
	SDL_Rect content = {.x = 0, .y = 0, .w = 1800, .h = 500};
	const tx_styled_text_options_t options = {
		.highlight_arch_tokens = true,
		.parse_authoring_prefixes = false,
		.body_color = C_WHITE
	};
	tx_styled_text_t *message = create_generic_message(
		"Not enough items in the Output Buffer [ob] after run", content,
		&options);
	assert(message->row_count == 1);
	assert(message->rows[0].fragment_count == 3);
	assert(fragment_has_color(&message->rows[0].fragments[0], C_WHITE));
	assert(fragment_has_color(&message->rows[0].fragments[1], C_TERMINAL_GREEN));
	assert(fragment_has_color(&message->rows[0].fragments[2], C_WHITE));
	assert(message->rows[0].fragments[0].x_offset == 0);
	assert(message->rows[0].fragments[1].x_offset >
	       message->rows[0].fragments[0].x_offset);
	assert(message->rows[0].fragments[2].x_offset >
	       message->rows[0].fragments[1].x_offset);
	tx_free_styled_text(message);

	message = create_generic_message(
		"A value cannot be recovered if the Input Buffer [ib] is empty",
		content, &options);
	assert(message->rows[0].fragment_count == 3);
	assert(fragment_has_color(&message->rows[0].fragments[0], C_WHITE));
	assert(fragment_has_color(&message->rows[0].fragments[1], C_TERMINAL_GREEN));
	assert(fragment_has_color(&message->rows[0].fragments[2], C_WHITE));
	tx_free_styled_text(message);

	message = create_generic_message(
		"Move [ib] into rax, then copy rax to [ob]", content, &options);
	assert(message->row_count == 1);
	assert(message->rows[0].fragment_count == 8);
	const SDL_Color expected[] = {
		C_WHITE, C_TERMINAL_GREEN, C_WHITE, C_TERMINAL_GREEN,
		C_WHITE, C_TERMINAL_GREEN, C_WHITE, C_TERMINAL_GREEN
	};
	assert_color_sequence(message, expected,
	                      sizeof(expected) / sizeof(expected[0]));
	tx_free_styled_text(message);
}

static SDL_Color fragment_color(const tx_text_fragment_t *fragment)
{
	const SDL_Color candidates[] = {
		C_WHITE, C_SILVERGREY, C_GREY, C_TERMINAL_GREEN};
	for (size_t index = 0; index < sizeof(candidates) / sizeof(candidates[0]);
	     index++) {
		if (fragment_has_color(fragment, candidates[index])) {
			return candidates[index];
		}
	}
	assert(false);
	return C_WHITE;
}

static void assert_all_fragments_color(const tx_styled_text_t *message,
									  SDL_Color expected)
{
	for (int row = 0; row < message->row_count; row++) {
		for (int fragment = 0; fragment < message->rows[row].fragment_count;
		     fragment++) {
			assert(fragment_color(&message->rows[row].fragments[fragment]).r ==
			       expected.r);
			assert(fragment_color(&message->rows[row].fragments[fragment]).g ==
			       expected.g);
			assert(fragment_color(&message->rows[row].fragments[fragment]).b ==
			       expected.b);
		}
	}
}

static void test_highlighting_policy_and_defaults(void)
{
	const char *text = "Move [ib] into rax, then write rax to [ob]";
	SDL_Rect content = {.x = 0, .y = 0, .w = 360, .h = 500};
	const tx_styled_text_options_t disabled_options = {
		.highlight_arch_tokens = false,
		.parse_authoring_prefixes = false,
		.body_color = C_SILVERGREY
	};
	const tx_styled_text_options_t enabled_options = {
		.highlight_arch_tokens = true,
		.parse_authoring_prefixes = false,
		.body_color = C_SILVERGREY
	};
	tx_styled_text_t *disabled = create_generic_message(
		text, content, &disabled_options);
	tx_styled_text_t *enabled = create_generic_message(
		text, content, &enabled_options);
	assert(disabled->row_count > 1);
	assert(enabled->row_count == disabled->row_count);
	for (int row = 0; row < disabled->row_count; row++) {
		int width_delta = abs(disabled->rows[row].rendered_width -
		                      enabled->rows[row].rendered_width);
		assert(width_delta <= enabled->rows[row].fragment_count + 1);
		assert(disabled->rows[row].rendered_width <= content.w);
		assert(enabled->rows[row].rendered_width <= content.w);
	}
	assert_all_fragments_color(disabled, C_SILVERGREY);
	bool found_green = false;
	for (int row = 0; row < enabled->row_count; row++) {
		for (int fragment = 0; fragment < enabled->rows[row].fragment_count;
		     fragment++) {
			SDL_Color color = fragment_color(&enabled->rows[row].fragments[fragment]);
			if (color.g == C_TERMINAL_GREEN.g &&
			    color.r == C_TERMINAL_GREEN.r &&
			    color.b == C_TERMINAL_GREEN.b) {
				found_green = true;
			} else {
				assert(color.r == C_SILVERGREY.r &&
				       color.g == C_SILVERGREY.g &&
				       color.b == C_SILVERGREY.b);
			}
		}
	}
	assert(found_green);
	tx_free_styled_text(disabled);
	tx_free_styled_text(enabled);

	tx_styled_text_t *default_message = tx_create_styled_text(
		(SDL_Rect){.x = 0, .y = 0, .w = 1000, .h = 200},
		ty_message_height(), "rax", NULL, NULL);
	assert(default_message != NULL);
	assert_all_fragments_color(default_message, C_WHITE);
	tx_free_styled_text(default_message);
}

static void test_challenge_and_hover_policies(void)
{
	cw_reset_challenge_text_factory();
	cw_create_code_list();
	const char *first_challenge =
		"Move [ib] into rax, execute MOV, and write to [ob]";
	char reconstructed[512];
	cw_set_challenge_text(first_challenge);
	assert(g_challenge_text != NULL);
	assert(reconstruct_styled_rows(g_challenge_text, reconstructed,
	                               sizeof(reconstructed)));
	assert(strcmp(reconstructed, first_challenge) == 0);
	assert_all_fragments_color(g_challenge_text, C_SILVERGREY);
	tx_draw_styled_text_pulsed(
		(SDL_Rect){.x = 100, .y = 100, .w = 700, .h = 300},
		ty_message_height(), 4.0f, g_challenge_text);
	assert_all_fragments_color(g_challenge_text, C_SILVERGREY);
	assert(SDL_RenderIsClipEnabled(g_renderer) == SDL_FALSE);

	int first_row_count = g_challenge_text->row_count;
	const char *replacement_challenge = "MOV rax [ib] [ob]";
	cw_set_challenge_text(replacement_challenge);
	assert(g_challenge_text != NULL);
	assert_all_fragments_color(g_challenge_text, C_SILVERGREY);
	assert(reconstruct_styled_rows(g_challenge_text, reconstructed,
	                               sizeof(reconstructed)));
	assert(strcmp(reconstructed, replacement_challenge) == 0);
	assert(strstr(reconstructed, "execute MOV") == NULL);
	assert(g_challenge_text->row_count < first_row_count);
	int calls_before_empty = challenge_factory_calls;
	cw_set_challenge_text_factory(test_challenge_text_factory);
	char diagnostic[256];
	size_t diagnostic_length =
		capture_challenge_diagnostic("", diagnostic, sizeof(diagnostic));
	cw_reset_challenge_text_factory();
	assert(g_challenge_text == NULL);
	assert(diagnostic_length == 0);
	assert(challenge_factory_calls == calls_before_empty);
	cw_set_challenge_text_factory(test_challenge_text_factory);
	diagnostic_length =
		capture_challenge_diagnostic("", diagnostic, sizeof(diagnostic));
	cw_reset_challenge_text_factory();
	assert(g_challenge_text == NULL);
	assert(diagnostic_length == 0);
	assert(challenge_factory_calls == calls_before_empty);
	cw_set_challenge_text_factory(test_challenge_text_factory);
	fail_challenge_creation = true;
	int calls_before_failure = challenge_factory_calls;
	diagnostic_length = capture_challenge_diagnostic(
		"non-empty allocation failure", diagnostic, sizeof(diagnostic));
	cw_reset_challenge_text_factory();
	assert(g_challenge_text == NULL);
	assert(diagnostic_length > 0);
	assert(strcmp(diagnostic,
	              "Unable to create styled challenge text\n") == 0);
	assert(challenge_factory_calls == calls_before_failure + 1);
	assert(challenge_factory_option_errors == 0);
	assert(strcmp(challenge_factory_text,
	              "non-empty allocation failure") == 0);
	fail_challenge_creation = false;
	assert_production_challenge_recovery(replacement_challenge,
	                                     reconstructed, sizeof(reconstructed));
	cw_set_challenge_text_factory(test_challenge_text_factory);
	cw_reset_challenge_text_factory();
	assert_production_challenge_recovery(replacement_challenge,
	                                     reconstructed, sizeof(reconstructed));
	cw_set_challenge_text_factory(test_challenge_text_factory);
	cw_set_challenge_text_factory(NULL);
	assert_production_challenge_recovery(replacement_challenge,
	                                     reconstructed, sizeof(reconstructed));
	cw_destroy_code_window_assets();
	assert(g_challenge_text == NULL);
	cw_reset_challenge_text_factory();

	tx_set_and_allocate_msgs_array(1);
	const tx_styled_text_options_t hover_options = {
		.highlight_arch_tokens = false,
		.parse_authoring_prefixes = false,
		.body_color = C_GREY
	};
	tx_set_message_in_array(0, "rax [ib]", 1000, ty_message_height());
	tx_styled_text_t *legacy_message = tx_get_message_texture(0);
	assert(legacy_message != NULL);
	assert_all_fragments_color(legacy_message, C_WHITE);
	tx_set_single_line_message(0, "rax [ib]");
	legacy_message = tx_get_message_texture(0);
	assert(legacy_message != NULL);
	assert_all_fragments_color(legacy_message, C_WHITE);
	tx_set_styled_message_in_array(0, "Use rax with [ib]", 1000,
	                               ty_message_height(), &hover_options);
	legacy_message = tx_get_message_texture(0);
	assert(legacy_message != NULL);
	assert_all_fragments_color(legacy_message, C_GREY);
	tx_set_styled_single_line_message(0, "Use rax with [ib]",
	                                  ty_message_height(), &hover_options);
	tx_styled_text_t *hover = tx_get_message_texture(0);
	assert(hover != NULL);
	assert_all_fragments_color(hover, C_GREY);
	tx_set_styled_text_alpha(hover, 96);
	for (int row = 0; row < hover->row_count; row++) {
		for (int fragment = 0; fragment < hover->rows[row].fragment_count;
		     fragment++) {
			Uint8 alpha = 255;
			assert(SDL_GetTextureAlphaMod(
			           hover->rows[row].fragments[fragment].texture->texture,
			           &alpha) == 0);
			assert(alpha == 96);
		}
	}
	tx_free_level_text_textures();
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
	bool found_wrapped_architecture_token = false;
	for (int row = 0; row < wrapped->row_count; row++) {
		assert(wrapped->rows[row].rendered_width <= content.w);
		for (int fragment = 0; fragment < wrapped->rows[row].fragment_count;
		     fragment++) {
			found_wrapped_architecture_token |= fragment_has_color(
			    &wrapped->rows[row].fragments[fragment], C_TERMINAL_GREEN);
		}
	}
	assert(found_wrapped_architecture_token);
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

static void test_execution_control_highlights(void)
{
	int targets = 0;
	char zero_flag_highlight[] = "zero_flag";
	assert(tr_parse_highlights(zero_flag_highlight, &targets));
	assert(targets == TUTORIAL_HIGHLIGHT_ZERO_FLAG);

	char composed_highlights[] = "register, zero_flag, output_buffer";
	assert(tr_parse_highlights(composed_highlights, &targets));
	assert(targets == (TUTORIAL_HIGHLIGHT_REGISTERS |
	                   TUTORIAL_HIGHLIGHT_ZERO_FLAG |
	                   TUTORIAL_HIGHLIGHT_OUTPUT_BUFFER));

	char zf_alias[] = "zf";
	assert(!tr_parse_highlights(zf_alias, &targets));
	char zero_flags_near_match[] = "zero_flags";
	assert(!tr_parse_highlights(zero_flags_near_match, &targets));
	char flag_alias[] = "flag";
	assert(!tr_parse_highlights(flag_alias, &targets));

	char step_highlight[] = "step";
	assert(tr_parse_highlights(step_highlight, &targets));
	assert(targets == TUTORIAL_HIGHLIGHT_STEP);

	char fast_highlight[] = "fast";
	assert(tr_parse_highlights(fast_highlight, &targets));
	assert(targets == TUTORIAL_HIGHLIGHT_FAST);

	char both_highlights[] = "step,fast";
	assert(tr_parse_highlights(both_highlights, &targets));
	assert(targets == (TUTORIAL_HIGHLIGHT_STEP | TUTORIAL_HIGHLIGHT_FAST));

	char mixed_highlights[] = "register, step, fast";
	assert(tr_parse_highlights(mixed_highlights, &targets));
	assert(targets == (TUTORIAL_HIGHLIGHT_REGISTERS |
	                   TUTORIAL_HIGHLIGHT_STEP |
	                   TUTORIAL_HIGHLIGHT_FAST));

	char unknown_step[] = "steps";
	assert(!tr_parse_highlights(unknown_step, &targets));
	char unknown_fast[] = "fas";
	assert(!tr_parse_highlights(unknown_fast, &targets));

	assert(tr_load_level(4));
	const tutorial_step_t *step =
	    tr_get_named_step("introduce_execution_controls");
	assert(step != NULL);
	assert(step->highlight_targets & TUTORIAL_HIGHLIGHT_STEP);
	assert(step->highlight_targets & TUTORIAL_HIGHLIGHT_FAST);
	assert(step->arrow_count == 0);
	assert(step->box == TUTORIAL_BOX_CENTER);
	assert(step->header == TUTORIAL_HEADER_SYSTEM_NOTICE);
	assert(step->dismiss == TUTORIAL_DISMISS_MOUSE_PRESS);
	assert(step->when_code_size == 1);
	assert(step->when_operand_pending == 0);
	assert(step->text_message != NULL);
	tr_clear();
}

int main(void)
{
	assert(SDL_Init(0) == 0);
	assert(TTF_Init() == 0);
	dm_set_screen_resolution(DM_RESOLUTION_1920X1080);
	g_screen = SDL_CreateRGBSurfaceWithFormat(
	    0, dm_get_screen_width(), dm_get_screen_height(), 32,
	    SDL_PIXELFORMAT_RGBA32);
	assert(g_screen != NULL);
	g_renderer = SDL_CreateSoftwareRenderer(g_screen);
	assert(g_renderer != NULL);
	g_font = TTF_OpenFont("fonts/DOSVGA437.ttf", 130);
	assert(g_font != NULL);

	test_execution_control_highlights();
	test_inline_styles();
	test_token_boundaries();
	test_generic_mixed_color_messages();
	test_highlighting_policy_and_defaults();
	test_challenge_and_hover_policies();
	test_blank_wrap_and_overflow();

	TTF_CloseFont(g_font);
	SDL_DestroyRenderer(g_renderer);
	SDL_FreeSurface(g_screen);
	TTF_Quit();
	SDL_Quit();
	puts("tutorial text tests passed");
	return 0;
}