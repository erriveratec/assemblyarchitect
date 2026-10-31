#ifndef ESCAPE_MENU_EM_H
#define ESCAPE_MENU_EM_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL.h>


typedef enum EscapeMenuAction {
	ESC_MENU_NONE,
	ESC_MENU_RESUME,
	ESC_MENU_FULLSCREEN,
	ESC_MENU_EXIT
} EscapeMenuAction;

typedef enum EscapeMenuButton {
	EM_BUTTON_RESUME,
	EM_BUTTON_FULLSCREEN,
	EM_BUTTON_EXIT
} EscapeMenuButton;

void em_toggle_escape_menu(void);
bool em_get_escape_state(void);
bool em_owns_frame_input(bool was_active, bool is_active);
void em_init_escape_menu(void);
void em_render_escape_menu(bool show_menu);
int em_update_escape_menu(bool show_menu);
SDL_Rect em_get_button_rect(EscapeMenuButton button);

//void sb_display_escape_menu(bool menu_variable_state);


#endif
