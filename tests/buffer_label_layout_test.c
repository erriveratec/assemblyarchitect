#include <assert.h>

/* Include the implementation to exercise private geometry helpers without
 * exposing them publicly; the Makefile excludes buffers_bf.c to avoid duplicate
 * definitions. */
#include "../source/buffers_bf.c"

static void assert_centered(int screen_width, SDL_Rect buffer,
							SDL_Rect label, int expected_x)
{
	dm_set_screen_dimensions(screen_width, 1080);
	center_label_on_visible_buffer(&label, buffer);
	assert(label.x == expected_x);
	assert(label.w == 200);
	assert(label.y == 17);
	assert(label.h == 30);
}

int main(void)
{
	SDL_Rect label = {.x = 1400, .y = 17, .w = 200, .h = 30};
	assert_centered(1920, (SDL_Rect){.x = 500, .y = 300, .w = 1000, .h = 75},
	                label, 900);
	assert_centered(1920, (SDL_Rect){.x = 1400, .y = 300, .w = 1000, .h = 75},
	                label, 1560);
	assert_centered(1600, (SDL_Rect){.x = -200, .y = 300, .w = 1000, .h = 75},
	                label, 300);

	dm_set_screen_dimensions(1920, 1080);
	SDL_Rect offscreen_buffer = {
		.x = 2000, .y = 300, .w = 1000, .h = 75
	};
	SDL_Rect visible = get_visible_horizontal_rect(offscreen_buffer);
	assert(visible.w == 0);
	label.x = 1777;
	center_label_on_visible_buffer(&label, offscreen_buffer);
	assert(label.x == 1777);

	visible = get_visible_horizontal_rect((SDL_Rect){
		.x = 500, .y = 300, .w = -100, .h = 75
	});
	assert(visible.w == 0);

	label = (SDL_Rect){.x = 1400, .y = 17, .w = 700, .h = 30};
	center_label_on_visible_buffer(&label, (SDL_Rect){
		.x = 1400, .y = 300, .w = 1000, .h = 75
	});
	assert(label.x == 1310);
	assert(label.w == 700);

	SDL_Rect body = {.x = 1400, .y = 50, .w = 1000, .h = 75};
	SDL_Rect centered_label = {.x = 1560, .y = 0, .w = 200, .h = 50};
	SDL_FRect animated_body = transform_buffer_body_rect(
		body, centered_label,
		(buffer_animation_t){.active = true, .boxes_active = true, .growth = 0});
	assert(animated_body.x == body.x);
	return 0;
}
