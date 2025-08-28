#ifndef SDL_CONFIG_H
#define SDL_CONFIG_H

#include <SDL.h>
#include <SDL_ttf.h>
#include "draw_dw.h"
#include "dimensions_dm.h"

extern SDL_Window *g_window;
extern SDL_Renderer *g_renderer;
extern SDL_Surface *g_screen;
extern SDL_Texture *g_screen_texture;


extern int g_width;
extern int g_height;

int init_sdl(int width, int height, int argc, char *args[]);

extern TTF_Font *g_font;

int load_media();


#endif





