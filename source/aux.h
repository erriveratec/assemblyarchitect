#ifndef AUX_AX_H
#define AUX_AX_H

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

// ASCII characters
#define CHAR_SPACE 32
#define CHAR_NULL 0
#define CHAR_NEWLINE 10

// Text for the data reading of the levels
#define CHAR_SIZE 1

#define MOVEMENT_DELTA 15

#define STR_BACK "Back"
#define STR_CONT "Continue"

extern char *char_space;
extern char *char_comma;
extern char *char_newline;
extern char *char_colon;
extern char *level_text;
extern char *char_dash;

//Boxes memeber definition used in several functions
enum box_member {
	MEMBER_MIN, 
	MEMBER_X, 
	MEMBER_Y, 
	MEMBER_W, 
	MEMBER_H, 
	MEMBER_MAX};

typedef struct value_box_t{
	int value;
	int type;
	bool visible_box;
	SDL_Rect box;
	texture_t *t;
} value_box_t;

char *ax_number_to_string(int number);
bool check_text_fits_width_by_height(char *t, int h, int w);
char *ax_number_to_string_prepend_zero(int number);
int get_movement_delta(int v1, int v2, int max_delta);
int ax_get_wrapped_text_height(int w, int h, char *t);
float get_scale_fit_height(int h, texture_t *texture);
float get_scale_fit_width(int w, texture_t *texture);
int get_text_width_fits_height(int h, char *text);
int get_text_height_fits_width(int w, char *text);
void ax_draw_value_box(value_box_t *box, SDL_Color color);
void ax_copy_vbox(value_box_t *dst, value_box_t src);
char *create_string_append_number(char *s,  int n);
char *ax_number_to_string_two_digits(int number);
int ax_get_texture_w_fit_h(int h, texture_t *t);
int ax_get_texture_h_fit_w(int w, texture_t *t);


#endif


