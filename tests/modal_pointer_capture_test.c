#include <assert.h>
#include <stdbool.h>

#include "mouse_ms.h"
#include "ui/button_bt.h"
#include "ui/escape_menu_em.h"

static SDL_Event button_event(Uint32 type, Uint8 button)
{
	SDL_Event event = {0};
	event.type = type;
	event.button.button = button;
	event.button.state = type == SDL_MOUSEBUTTONDOWN ? SDL_PRESSED : SDL_RELEASED;
	return event;
}

static void test_frame_ownership_transitions(void)
{
	assert(!em_owns_frame_input(false, false));
	assert(em_owns_frame_input(false, true));
	assert(em_owns_frame_input(true, true));
	assert(em_owns_frame_input(true, false));
}

static void test_aggregate_pointer_consumption(void)
{
	ms_reset_mouse_values();
	SDL_Event left_down = button_event(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT);
	SDL_Event right_down = button_event(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_RIGHT);
	ms_mouse_button_handler(left_down);
	ms_mouse_button_handler(right_down);
	SDL_Event wheel = {0};
	wheel.type = SDL_MOUSEWHEEL;
	wheel.wheel.x = 1;
	wheel.wheel.y = 2;
	ms_mouse_wheel_handler(wheel);
	assert(ms_get_mouse_scroll_x() == 1);
	assert(ms_get_mouse_scroll_y() == 2);
	ms_mouse_wheel_handler(wheel);
	assert(ms_left_pressed());
	assert(ms_right_pressed());
	assert(ms_chord_pressed());
	ms_consume_pointer_input(false);
	assert(!ms_left_pressed());
	assert(!ms_left_released());
	assert(!ms_right_pressed());
	assert(!ms_right_released());
	assert(!ms_chord_pressed());
	assert(!ms_chord_released());
	assert(ms_get_mouse_scroll_x() == 0);
	assert(ms_get_mouse_scroll_y() == 0);

	SDL_Event left_up = button_event(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT);
	SDL_Event right_up = button_event(SDL_MOUSEBUTTONUP, SDL_BUTTON_RIGHT);
	ms_mouse_button_handler(left_up);
	ms_mouse_button_handler(right_up);
	assert(ms_left_released());
	assert(ms_right_released());
	assert(ms_chord_released());
	ms_consume_pointer_input(false);
	assert(!ms_left_released());
	assert(!ms_right_released());
	assert(!ms_chord_released());
}

static void test_held_release_after_modal_close_is_suppressed(void)
{
	ms_reset_mouse_values();
	SDL_Event left_down = button_event(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT);
	ms_mouse_button_handler(left_down);
	ms_consume_pointer_input(true);
	SDL_Event left_up = button_event(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT);
	ms_mouse_button_handler(left_up);
	assert(!ms_left_released());

	ms_mouse_button_handler(left_down);
	assert(ms_left_pressed());
	ms_mouse_button_handler(left_up);
	assert(ms_left_released());
}

static void test_held_right_release_after_modal_close_is_suppressed(void)
{
	ms_reset_mouse_values();
	SDL_Event right_down = button_event(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_RIGHT);
	SDL_Event right_up = button_event(SDL_MOUSEBUTTONUP, SDL_BUTTON_RIGHT);
	ms_mouse_button_handler(right_down);
	assert(ms_right_pressed());
	ms_consume_pointer_input(true);
	ms_mouse_button_handler(right_up);
	assert(!ms_right_released());

	ms_mouse_button_handler(right_down);
	assert(ms_right_pressed());
	ms_mouse_button_handler(right_up);
	assert(ms_right_released());
}

static void test_background_button_release_is_captured(void)
{
	ms_reset_mouse_values();
	btn_t background_button = {
		.r = {.x = 100, .y = 100, .w = 80, .h = 40},
		.enabled = true
	};
	SDL_Event release = button_event(SDL_MOUSEBUTTONUP, SDL_BUTTON_LEFT);
	release.button.x = 120;
	release.button.y = 120;
	ms_mouse_button_handler(release);
	assert(bt_chk_rel_btn(&background_button, NULL));
	ms_consume_pointer_input(false);
	assert(!bt_chk_rel_btn(&background_button, NULL));

	SDL_Event fresh_down = button_event(SDL_MOUSEBUTTONDOWN, SDL_BUTTON_LEFT);
	fresh_down.button.x = 120;
	fresh_down.button.y = 120;
	SDL_Event fresh_release = release;
	ms_mouse_button_handler(fresh_down);
	ms_mouse_button_handler(fresh_release);
	assert(bt_chk_rel_btn(&background_button, NULL));
}

int main(void)
{
	test_frame_ownership_transitions();
	test_aggregate_pointer_consumption();
	test_held_release_after_modal_close_is_suppressed();
	test_held_right_release_after_modal_close_is_suppressed();
	test_background_button_release_is_captured();
	return 0;
}