#ifndef AUX_H
#define AUX_H

#include <stdbool.h>
#include "list.h"
#include "draw_dw.h"

// For values not assigned
#define NO_VALUE 0x80000001

// For the return of functions
#define SUCCESS 0
#define FAIL 1


//For string comparisons
#define STRING_EQUAL 0

//Boxes memeber definition used in several functions
enum box_member {INVALID_MEMBER, MEMBER_X, MEMBER_Y, MEMBER_W, MEMBER_H};

typedef struct value_box_t{
	int value;
	int type;
	bool visible_box;
	SDL_Rect box;
} value_box_t;

char *number_to_string(int number);
bool check_if_text_fits_in_width(char *t, float s, int w);
bool check_if_text_fits_in_width_by_height(char *t, int h, int w);
char *number_to_string_with_prepend_zero(int number);
int get_text_height(char *t, float s);
int get_text_width(char *text, float scale);
int get_movement_delta(int v1, int v2, int max_delta);
int get_wrapped_text_height(int w, int h, char *t);
float get_scale_from_w_h(int w, int h, texture_t *texture);
float get_scale_fit_height(int h, texture_t *texture);
float get_scale_fit_width(int w, texture_t *texture);
int get_text_width_fits_height(int h, char *text);
void ax_draw_value_box(value_box_t *box, SDL_Color color);
char *create_string_append_number(char *s,  int n);
char *ax_number_to_string_two_digits(int number);

// ASCII characters
#define CHAR_SPACE 32
#define CHAR_NULL 0
#define CHAR_NEWLINE "\n"

// Text for the data reading of the levels
#define CHAR_SIZE 1

extern char *char_space;
extern char *char_comma;
extern char *char_newline;
extern char *char_colon;
extern char *level_text;


#endif


