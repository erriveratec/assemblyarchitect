#ifndef MOUSE_MS_H
#define MOUSE_MS_H

#include <stdbool.h>
#include <SDL.h>


void ms_mouse_button_handler(SDL_Event e);
void ms_mouse_motion_handler(SDL_Event e);
void ms_mouse_wheel_handler(SDL_Event e);

int ms_get_mouse_x();
int ms_get_mouse_y();
int ms_get_mouse_scroll_y();

bool ms_chk_mouse_left_pressed();
bool ms_chk_mouse_left_released(); //not_enables
void ms_disable_mouse_button();

void ms_init_mouse();
void ms_clear_mouse_values();
void ms_reset_mouse_values();
#endif
