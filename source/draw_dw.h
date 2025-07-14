#ifndef DRAW_H
#define DRAW_H
#include <SDL.h>


typedef struct texture_t{
	int w;
	int h;
	float scale;
	SDL_Texture *texture;
} texture_t;

#define WRAPPED_TEXT_X_OFFSET 5
#define WRAPPED_TEXT_Y_OFFSET 5


texture_t *load_texture_from_file(char *path);
int draw_scaled_texture(int x, int y, float s, texture_t *t);
texture_t *load_texture_from_rendered_text(char *texture_text, 
									   	   SDL_Color text_color);
void free_texture(texture_t *texture);
void draw_text(int x, int y, float s, SDL_Color c, char *t);
void draw_text_fits_height(int x, int y, int h, SDL_Color color, char *text);
void dw_draw_text_fits_width(int x, int y, int w, SDL_Color color, char *text);
void draw_rectangle(int x, int y, int w, int h, SDL_Color c);
void dw_draw_filled_rectangle(int x, int y, int w, int h, SDL_Color inside,
						       SDL_Color outline);
void dw_draw_wrapped_text_fits_height(int x, int y, int w, int h, SDL_Color c, 
								   char *t);

extern SDL_Color COLOR_WHITE;
extern SDL_Color COLOR_BLACK;
extern SDL_Color COLOR_RED;
extern SDL_Color COLOR_GREY;
#endif

