#ifndef WIN_CONDITION_WC_H
#define WIN_CONDITION_WC_H
#include <stdbool.h>
#include "list.h"

List *wc_get_expected_output();
int wc_get_expected_output_size();
void wc_destroy_expected_output();
bool wc_create_expected_output();
bool wc_reset_expected_output();
void wc_print_expected_output();
bool lv_chk_correct_output();
void lv_set_level_win_condition_text(char *win_condition);
void lv_reset_level_win_condition();

#endif
