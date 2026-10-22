#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dimensions_dm.h"
#include "ui/value_box_vb.h"

SDL_Color C_WHITE = {255, 255, 255, 255};
SDL_Color C_BLACK = {0, 0, 0, 255};

static int g_texture_create_count;
static int g_texture_free_count;
static char g_last_texture_text[32];

char *ax_number_to_string(int number)
{
	char *text = malloc(32);
	assert(text != NULL);
	snprintf(text, 32, "%d", number);
	return text;
}

texture_t *dw_create_text_tex(char *text, SDL_Color color)
{
	(void)color;
	texture_t *texture = calloc(1, sizeof(*texture));
	assert(texture != NULL);
	snprintf(g_last_texture_text, sizeof(g_last_texture_text), "%s", text);
	g_texture_create_count++;
	return texture;
}

void dw_free_texture(texture_t *texture)
{
	if (texture != NULL) {
		g_texture_free_count++;
		free(texture);
	}
}

int ax_get_texture_w_fit_h(int height, texture_t *texture)
{
	(void)height;
	(void)texture;
	return 0;
}

void dw_draw_filled_rectangle(SDL_Rect rect, SDL_Color fill, SDL_Color border)
{
	(void)rect;
	(void)fill;
	(void)border;
}

int dw_draw_texture_fit_h(SDL_Rect rect, texture_t *texture)
{
	(void)rect;
	(void)texture;
	return 0;
}

static void assert_centers_match(SDL_Rect source, SDL_Rect destination)
{
	int64_t source_center_x = (int64_t)2 * source.x + source.w;
	int64_t source_center_y = (int64_t)2 * source.y + source.h;
	int64_t destination_center_x = (int64_t)2 * destination.x + destination.w;
	int64_t destination_center_y = (int64_t)2 * destination.y + destination.h;
	assert(llabs(source_center_x - destination_center_x) <= 1);
	assert(llabs(source_center_y - destination_center_y) <= 1);
}

static void test_transport_at_resolution(dm_resolution_t resolution)
{
	dm_set_screen_resolution(resolution);
	SDL_Rect canonical = vb_get_size();
	SDL_Rect source_box = {
		.x = -37,
		.y = -23,
		.w = canonical.w + dm_scale_to_res(10),
		.h = canonical.h + dm_scale_to_res(10)
	};
	vb_value_box_t source = {
		.value = -17,
		.type = 23,
		.visible_box = false,
		.box = source_box,
		.t = dw_create_text_tex("source", C_WHITE)
	};
	vb_value_box_t destination = {
		.box = {.x = 0, .y = 0, .w = 1, .h = 1},
		.t = dw_create_text_tex("old", C_WHITE)
	};
	texture_t *source_texture = source.t;
	int frees_before_copy = g_texture_free_count;
	int creates_before_copy = g_texture_create_count;

	vb_copy_for_transport(&destination, &source);
	assert(destination.value == source.value);
	assert(destination.type == source.type);
	assert(destination.visible_box == source.visible_box);
	assert(destination.box.w == canonical.w);
	assert(destination.box.h == canonical.h);
	assert_centers_match(source_box, destination.box);
	assert(memcmp(&source.box, &source_box, sizeof(source_box)) == 0);
	assert(destination.t != NULL && destination.t != source_texture);
	assert(g_texture_create_count == creates_before_copy + 1);
	assert(strcmp(g_last_texture_text, "-17") == 0);
	assert(g_texture_free_count == frees_before_copy + 1);

	source.box = (SDL_Rect){
		.x = -41,
		.y = -29,
		.w = canonical.w + dm_scale_to_res(9),
		.h = canonical.h + dm_scale_to_res(11)
	};
	vb_copy_for_transport(&destination, &source);
	assert(destination.box.w == canonical.w);
	assert(destination.box.h == canonical.h);
	assert_centers_match(source.box, destination.box);

	SDL_Rect normal_source_box = {
		.x = -11,
		.y = -9,
		.w = canonical.w,
		.h = canonical.h
	};
	source.box = normal_source_box;
	source.visible_box = true;
	vb_copy_for_transport(&destination, &source);
	assert(destination.box.w == canonical.w);
	assert(destination.box.h == canonical.h);
	assert(memcmp(&destination.box, &normal_source_box,
	              sizeof(normal_source_box)) == 0);
	assert(destination.visible_box);
	assert(destination.t != NULL);

	dw_free_texture(source.t);
	dw_free_texture(destination.t);
}

int main(void)
{
	test_transport_at_resolution(DM_RESOLUTION_1600X900);
	test_transport_at_resolution(DM_RESOLUTION_1920X1080);
	assert(g_texture_create_count == g_texture_free_count);
	vb_copy_for_transport(NULL, NULL);
	return 0;
}