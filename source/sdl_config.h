#ifndef SDL_CONFIG_H
#define SDL_CONFIG_H

#include <stdbool.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>

extern SDL_Window *g_window;
extern SDL_Renderer *g_renderer;
extern SDL_Surface *g_screen;
extern SDL_Texture *g_screen_texture;


extern TTF_Font *g_font;


int load_media(void);
void close_sdl(void);
int init_sdl(int logical_width, int logical_height, int argc, char *args[]);
int toggle_fullscreen(void);
bool sdl_is_fullscreen(void);
bool sdl_fullscreen_requested(int argc, char *args[]);
int sdl_set_fullscreen(bool enabled);
int sdl_handle_window_size_changed(int window_width, int window_height);
int sdl_handle_window_moved(int window_x, int window_y);
int sdl_apply_display_configuration(void);
int sdl_apply_window_size_policy(void);
int sdl_handle_display_changed(void);


#endif





