#ifndef DRAW_DW_H
#define DRAW_DW_H
#include <SDL.h>
#include <stdbool.h>

#define WRAPPED_TEXT_X_OFFSET 5

typedef struct texture_t{
	int w;
	int h;
	SDL_Texture *texture;
} texture_t;

typedef struct texture_array_t{
	int size;
	texture_t **t;
} texture_array_t;

texture_t *load_texture_from_file(char *path);
texture_t *dw_create_text_texture(char *texture_text, SDL_Color text_color);
texture_array_t *dw_new_text_texture_by_h(int w, int h, SDL_Color c, char *t);

void dw_free_texture(texture_t *texture);
void dw_free_texture_array(texture_array_t *t);

void dw_draw_rectangle(SDL_Rect r, SDL_Color c);
void dw_draw_filled_rectangle(SDL_Rect r, SDL_Color in, SDL_Color out);

int dw_draw_texture_fits_height(SDL_Rect r, texture_t *t);
int dw_draw_texture_fits_width(SDL_Rect r, texture_t *t);
void dw_draw_wrapped_texture_by_h(SDL_Rect r, int h, texture_array_t *a);
void dw_draw_rotated_texture_fits_h(int x, int y, int h, double angle, 
															      texture_t *t);

extern SDL_Color C_WHITE;
extern SDL_Color C_BLACK;
extern SDL_Color C_RED;
extern SDL_Color C_GREY;
extern SDL_Color C_ORANGE;
extern SDL_Color C_CYAN;
extern SDL_Color C_YELLOW;
extern SDL_Color C_MAGENTA;
extern SDL_Color C_GREEN;
#endif

