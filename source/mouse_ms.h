#ifndef MOUSE_MS_H
#define MOUSE_MS_H

#include <stdbool.h>
#include <SDL.h>


void ms_mouse_button_handler(SDL_Event e);
void ms_mouse_motion_handler(SDL_Event e);
void ms_mouse_wheel_handler(SDL_Event e);

int ms_get_mouse_x(void);
int ms_get_mouse_y(void);
int ms_get_mouse_scroll_x(void);
int ms_get_mouse_scroll_y(void);

bool ms_left_pressed(void);
bool ms_left_released(void);
bool ms_right_pressed(void);
bool ms_right_released(void);
bool ms_chord_pressed(void);
bool ms_chord_released(void);

bool ms_click_inside_rect(SDL_Rect r);

void ms_disable_mouse_button(void);
void ms_init_mouse(void);
void ms_clear_mouse_values(void);
void ms_reset_mouse_values(void);
void ms_consume_left_press(void);
void ms_consume_left_release(void);
void ms_consume_pointer_input(bool suppress_held_releases);
#endif
