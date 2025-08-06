#ifndef DRAW_H
#define DRAW_H
#include <SDL.h>

#define WRAPPED_TEXT_X_OFFSET 5
#define WRAPPED_TEXT_Y_OFFSET 5

typedef struct texture_t{
	int w;
	int h;
	float scale;
	SDL_Texture *texture;
} texture_t;

enum movement{
	DW_MOV_MIN,
	DW_UP,
	DW_DOWN,
	DW_LEFT,
	DW_RIGHT
};

extern texture_t *g_arrow;

texture_t *load_texture_from_file(char *path);
texture_t *load_texture_from_rendered_text(char *texture_text, 
									   	   SDL_Color text_color);
void dw_free_texture(texture_t *texture);
void dw_draw_text_fits_height(int x, int y, int h, SDL_Color color, char *text);
void dw_draw_text_fits_width(int x, int y, int w, SDL_Color color, char *text);
void dw_draw_rectangle(int x, int y, int w, int h, SDL_Color c);
void dw_draw_filled_rectangle(int x, int y, int w, int h, SDL_Color inside,
						       SDL_Color outline);
void dw_draw_wrapped_text_fits_height(int x, int y, int w, int h, SDL_Color c, 
								   char *t);
int dw_draw_texture_fits_height(int x, int y, int h, texture_t *t);
int dw_draw_texture_fits_width(int x, int y, int w, texture_t *t);
void dw_draw_animated_arrow(int x, int y, int w, int h, int dir, int travel);
static void dw_draw_rotated_texture_fits_h(int x, int y, int h, double angle, 
															     texture_t *t);

extern SDL_Color COLOR_WHITE;
extern SDL_Color COLOR_BLACK;
extern SDL_Color COLOR_RED;
extern SDL_Color COLOR_GREY;
extern SDL_Color COLOR_ORANGE;
extern SDL_Color COLOR_CYAN;
#endif

