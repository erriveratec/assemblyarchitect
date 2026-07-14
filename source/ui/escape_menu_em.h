#ifndef ESCAPE_MENU_EM_H
#define ESCAPE_MENU_EM_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


typedef enum EscapeMenuAction {
	ESC_MENU_NONE,
	ESC_MENU_RESUME,
	ESC_MENU_FULLSCREEN,
	ESC_MENU_EXIT
} EscapeMenuAction;

void em_toggle_escape_menu();
bool em_get_escape_state();
void em_init_escape_menu();
void em_render_escape_menu(bool show_menu);
int em_update_escape_menu(bool show_menu);

//void sb_display_escape_menu(bool menu_variable_state);


#endif
