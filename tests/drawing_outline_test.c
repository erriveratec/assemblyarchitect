#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include "draw_dw.h"
#include "sdl_config.h"

static SDL_Surface *create_target(int width, int height)
{
	SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormat(
		0, width, height, 32, SDL_PIXELFORMAT_RGBA32);
	assert(surface != NULL);
	g_renderer = SDL_CreateSoftwareRenderer(surface);
	assert(g_renderer != NULL);
	return surface;
}

static void destroy_target(SDL_Surface *surface)
{
	SDL_DestroyRenderer(g_renderer);
	g_renderer = NULL;
	SDL_FreeSurface(surface);
}

static bool rect_contains(SDL_Rect outer, SDL_Rect inner)
{
	return (int64_t)inner.x >= outer.x && (int64_t)inner.y >= outer.y &&
	       (int64_t)inner.x + inner.w <= (int64_t)outer.x + outer.w &&
	       (int64_t)inner.y + inner.h <= (int64_t)outer.y + outer.h;
}

static bool point_in_rect(SDL_Rect rect, int x, int y)
{
	return x >= rect.x && y >= rect.y &&
	       x < rect.x + rect.w && y < rect.y + rect.h;
}

static void test_outline_geometry(void)
{
	SDL_Rect rect = {.x = 20, .y = 30, .w = 100, .h = 50};
	SDL_Rect edges[4];
	assert(dw_calculate_rect_outline_edges(rect, 2, edges));
	assert(edges[0].x == 20 && edges[0].y == 30 &&
	       edges[0].w == 100 && edges[0].h == 2);
	assert(edges[1].x == 118 && edges[1].y == 32 &&
	       edges[1].w == 2 && edges[1].h == 46);
	assert(edges[2].x == 20 && edges[2].y == 78 &&
	       edges[2].w == 100 && edges[2].h == 2);
	assert(edges[3].x == 20 && edges[3].y == 32 &&
	       edges[3].w == 2 && edges[3].h == 46);

	for (size_t edge = 0; edge < 4; edge++) {
		assert(rect_contains(rect, edges[edge]));
	}
	for (int y = rect.y; y < rect.y + rect.h; y++) {
		for (int x = rect.x; x < rect.x + rect.w; x++) {
			bool boundary = x < rect.x + 2 || x >= rect.x + rect.w - 2 ||
			                y < rect.y + 2 || y >= rect.y + rect.h - 2;
			int covered = 0;
			for (size_t edge = 0; edge < 4; edge++) {
				covered += point_in_rect(edges[edge], x, y) ? 1 : 0;
			}
			assert(covered == (boundary ? 1 : 0));
		}
	}

	assert(!dw_calculate_rect_outline_edges(rect, 0, edges));
	assert(!dw_calculate_rect_outline_edges(rect, -1, edges));
	assert(dw_calculate_rect_outline_edges(rect, 100, edges));
	assert(edges[0].h == 25 && edges[1].h == 0 &&
	       edges[2].h == 25 && edges[3].h == 0);
	for (size_t edge = 0; edge < 4; edge++) {
		assert(rect_contains(rect, edges[edge]));
	}

	static const SDL_Rect small_rects[] = {
		{.x = 0, .y = 0, .w = 1, .h = 1},
		{.x = 0, .y = 0, .w = 1, .h = 8},
		{.x = 0, .y = 0, .w = 8, .h = 1},
		{.x = 0, .y = 0, .w = 2, .h = 2},
		{.x = 0, .y = 0, .w = 2, .h = 7},
		{.x = 0, .y = 0, .w = 7, .h = 2}
	};
	for (size_t index = 0;
	     index < sizeof(small_rects) / sizeof(small_rects[0]); index++) {
		assert(dw_calculate_rect_outline_edges(small_rects[index], 20, edges));
		int64_t area = 0;
		for (size_t edge = 0; edge < 4; edge++) {
			assert(rect_contains(small_rects[index], edges[edge]));
			area += (int64_t)edges[edge].w * edges[edge].h;
		}
		assert(area == (int64_t)small_rects[index].w * small_rects[index].h);
	}

	SDL_Rect realistic = {.x = 1800, .y = 990, .w = 120, .h = 80};
	assert(dw_calculate_rect_outline_edges(realistic, 2, edges));
	SDL_Rect overflowing = {.x = INT_MAX - 5, .y = 0, .w = 20, .h = 10};
	assert(!dw_calculate_rect_outline_edges(overflowing, 2, edges));
}

static Uint32 *read_surface(SDL_Surface *surface)
{
	Uint32 *pixels = malloc((size_t)surface->pitch * (size_t)surface->h);
	assert(pixels != NULL);
	assert(SDL_RenderReadPixels(g_renderer, NULL, SDL_PIXELFORMAT_RGBA32,
	                            pixels, surface->pitch) == 0);
	return pixels;
}

static void test_outline_pixel_readback(void)
{
	SDL_Surface *surface = create_target(100, 50);
	SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
	assert(SDL_RenderClear(g_renderer) == 0);
	SDL_Rect rect = {.x = 10, .y = 8, .w = 80, .h = 34};
	dw_draw_filled_rectangle_with_outline(rect, C_BLUE, C_SUCCESS_GREEN, 2);
	dw_draw_filled_rectangle_f_with_outline(
		(SDL_FRect){.x = 72.0f, .y = 44.0f, .w = 18.0f, .h = 5.0f},
		C_BLACK, C_FAILURE_RED, 2.0f);
	Uint32 *pixels = read_surface(surface);
	Uint32 border = SDL_MapRGBA(surface->format, C_SUCCESS_GREEN.r,
	                            C_SUCCESS_GREEN.g, C_SUCCESS_GREEN.b,
	                            C_SUCCESS_GREEN.a);
	Uint32 background = SDL_MapRGBA(surface->format, 0, 0, 0, 255);
	int pitch_pixels = surface->pitch / (int)sizeof(*pixels);
	for (int x = rect.x; x < rect.x + rect.w; x++) {
		assert(pixels[rect.y * pitch_pixels + x] == border);
		assert(pixels[(rect.y + rect.h - 1) * pitch_pixels + x] == border);
	}
	for (int y = rect.y; y < rect.y + rect.h; y++) {
		assert(pixels[y * pitch_pixels + rect.x] == border);
		assert(pixels[y * pitch_pixels + rect.x + rect.w - 1] == border);
	}
	Uint32 fill = SDL_MapRGBA(surface->format, C_BLUE.r, C_BLUE.g, C_BLUE.b,
	                         C_BLUE.a);
	Uint32 red = SDL_MapRGBA(surface->format, C_FAILURE_RED.r,
	                         C_FAILURE_RED.g, C_FAILURE_RED.b, C_FAILURE_RED.a);
	assert(pixels[(rect.y + 10) * pitch_pixels + rect.x + 10] == fill);
	assert(pixels[1 * pitch_pixels + 1] == background);
	assert(pixels[44 * pitch_pixels + 72] == red);
	assert(pixels[46 * pitch_pixels + 77] == background);
	free(pixels);
	destroy_target(surface);
}

static Uint32 *render_diagnostic_scene(const char *sampling_hint,
	                                  SDL_Surface **out_surface)
{
	assert(SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, sampling_hint) == SDL_TRUE);
	SDL_Surface *surface = create_target(100, 50);
	SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
	assert(SDL_RenderClear(g_renderer) == 0);

	SDL_SetRenderDrawColor(g_renderer, C_FAILURE_RED.r, C_FAILURE_RED.g,
	                       C_FAILURE_RED.b, C_FAILURE_RED.a);
	SDL_RenderDrawLine(g_renderer, 2, 3, 97, 3);
	SDL_RenderDrawLine(g_renderer, 2, 3, 2, 47);
	SDL_SetRenderDrawColor(g_renderer, C_CYAN.r, C_CYAN.g,
	                       C_CYAN.b, C_CYAN.a);
	SDL_RenderDrawLine(g_renderer, 2, 5, 97, 5);
	SDL_RenderDrawLine(g_renderer, 2, 6, 97, 6);
	SDL_RenderDrawLine(g_renderer, 4, 5, 4, 47);
	SDL_RenderDrawLine(g_renderer, 5, 5, 5, 47);
	SDL_SetRenderDrawColor(g_renderer, C_AMBER.r, C_AMBER.g,
	                       C_AMBER.b, C_AMBER.a);
	SDL_RenderDrawRect(g_renderer, &(SDL_Rect){
		.x = 30, .y = 18, .w = 20, .h = 10
	});
	dw_draw_rect_outline((SDL_Rect){.x = 10, .y = 10, .w = 16, .h = 12},
	                     2, C_SUCCESS_GREEN);
	dw_draw_rect_outline((SDL_Rect){.x = 60, .y = 10, .w = 16, .h = 12},
	                     2, C_FAILURE_RED);
	dw_draw_rect_outline((SDL_Rect){.x = 0, .y = 0, .w = 4, .h = 4},
	                     2, C_AMBER);
	dw_draw_rect_outline((SDL_Rect){.x = 96, .y = 46, .w = 4, .h = 4},
	                     2, C_AMBER);

	*out_surface = surface;
	return read_surface(surface);
}

static void test_sampling_hint_characterization(void)
{
	SDL_Surface *linear_surface = NULL;
	Uint32 *linear_pixels = render_diagnostic_scene("1", &linear_surface);
	SDL_Surface *nearest_surface = NULL;
	Uint32 *nearest_pixels = render_diagnostic_scene("0", &nearest_surface);
	assert(linear_surface->pitch == nearest_surface->pitch);
	assert(linear_surface->h == nearest_surface->h);
	assert(memcmp(linear_pixels, nearest_pixels,
	              (size_t)linear_surface->pitch * linear_surface->h) == 0);
	Uint32 *pixels = linear_pixels;
	int pitch_pixels = linear_surface->pitch / (int)sizeof(*pixels);
	Uint32 red = SDL_MapRGBA(linear_surface->format, C_FAILURE_RED.r,
	                         C_FAILURE_RED.g, C_FAILURE_RED.b, C_FAILURE_RED.a);
	Uint32 cyan = SDL_MapRGBA(linear_surface->format, C_CYAN.r,
	                          C_CYAN.g, C_CYAN.b, C_CYAN.a);
	Uint32 amber = SDL_MapRGBA(linear_surface->format, C_AMBER.r,
	                           C_AMBER.g, C_AMBER.b, C_AMBER.a);
	Uint32 green = SDL_MapRGBA(linear_surface->format, C_SUCCESS_GREEN.r,
	                           C_SUCCESS_GREEN.g, C_SUCCESS_GREEN.b,
	                           C_SUCCESS_GREEN.a);
	assert(pixels[3 * pitch_pixels + 40] == red);
	assert(pixels[5 * pitch_pixels + 40] == cyan);
	assert(pixels[6 * pitch_pixels + 40] == cyan);
	assert(pixels[18 * pitch_pixels + 30] == amber);
	assert(pixels[10 * pitch_pixels + 10] == green);
	assert(pixels[0] == amber);
	assert(pixels[49 * pitch_pixels + 99] == amber);
	free(linear_pixels);
	free(nearest_pixels);
	destroy_target(linear_surface);
	destroy_target(nearest_surface);
}

int main(void)
{
	assert(SDL_Init(0) == 0);
	test_outline_geometry();
	test_outline_pixel_readback();
	test_sampling_hint_characterization();
	SDL_Quit();
	puts("drawing outline tests passed");
	return 0;
}
