#ifndef RESET_MENU_RM_H
#define RESET_MENU_RM_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


void rm_init_rst_menu();
void rm_render_rst_menu(bool show_menu);
void rm_set_rst_menu(bool state);
bool rm_chk_rst_menu_state();
bool rm_chk_rst_menu_btns(bool show_menu);
#endif
