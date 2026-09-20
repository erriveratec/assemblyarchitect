#ifndef WIN_CONDITION_WC_H
#define WIN_CONDITION_WC_H
#include <stdbool.h>

int wc_get_expected_output_size();
void wc_destroy_expected_output();
bool wc_create_expected_output();
bool wc_reset_expected_output();
void wc_print_expected_output();
bool wc_matches_expected_prefix();
bool wc_set_condition_from_text(const char *text);
bool wc_reset_condition(void);
bool wc_is_satisfied(void);
#endif
