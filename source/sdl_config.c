#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <stdio.h>
#include <string.h>
#include "sdl_config.h"
#include "sdl_config_internal.h"
#include "aux.h"
#include "draw_dw.h"
#include "stage_buttons_sb.h"
#include "arrow_ar.h"
#include "media/audio_au.h"
#include "dimensions_dm.h"

#define ERROR -1

SDL_Window *g_window = NULL;
SDL_Renderer *g_renderer = NULL;
SDL_Surface *g_screen = NULL;
SDL_Texture *g_screen_texture = NULL;

static int g_window_width;
static int g_window_height;
static int g_renderer_output_width;
static int g_renderer_output_height;
static SDL_Rect g_windowed_rect;
static bool g_windowed_rect_valid;
static bool g_sdl_initialized;
static bool g_ttf_initialized;

TTF_Font *g_font = NULL;


/* Function: init_sdl
 * -------------------------------------
 * This function initialises what is necessary for SDL
 * to work.
 *
 * Arguments:
 *	None.
 *
 * Return:
 *	Void.
 */
int init_sdl(int width, int height, int argc, char *args[])
{
	bool fullscreen_requested = sdl_fullscreen_requested(argc, args);
	Uint32 window_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
	int logical_width = dm_get_screen_width();
	int logical_height = dm_get_screen_height();

	if (width != logical_width || height != logical_height) {
		SDL_Log("Requested window size %dx%d differs from logical canvas %dx%d; using the logical canvas size",
		        width, height, logical_width, logical_height);
	}
	if (fullscreen_requested) {
		window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	}

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
		SDL_Log("SDL initialization failed: %s", SDL_GetError());
		return FAIL;
	}
	g_sdl_initialized = true;
	if (SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1") == SDL_FALSE) {
		SDL_Log("Linear texture filtering could not be enabled");
	}

	g_window = SDL_CreateWindow("Assembly Architect",
	                            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
	                            logical_width, logical_height, window_flags);
	if (g_window == NULL) {
		SDL_Log("Window creation failed: %s", SDL_GetError());
		goto failure;
	}
	int display_index = SDL_GetWindowDisplayIndex(g_window);
	SDL_Rect usable_bounds;
	g_windowed_rect = (SDL_Rect){
		.x = SDL_WINDOWPOS_CENTERED,
		.y = SDL_WINDOWPOS_CENTERED,
		.w = logical_width,
		.h = logical_height
	};
	g_windowed_rect_valid = true;
	if (display_index >= 0 &&
	    SDL_GetDisplayUsableBounds(display_index, &usable_bounds) == 0) {
		g_windowed_rect = (SDL_Rect){
			.x = usable_bounds.x + (usable_bounds.w - logical_width) / 2,
			.y = usable_bounds.y + (usable_bounds.h - logical_height) / 2,
			.w = logical_width,
			.h = logical_height
		};
		g_windowed_rect_valid = true;
	}
	if (!fullscreen_requested) {
		SDL_GetWindowPosition(g_window, &g_windowed_rect.x, &g_windowed_rect.y);
		SDL_GetWindowSize(g_window, &g_windowed_rect.w, &g_windowed_rect.h);
		g_windowed_rect_valid = true;
	}

	g_renderer = SDL_CreateRenderer(g_window, -1,
	                                SDL_RENDERER_ACCELERATED |
	                                SDL_RENDERER_PRESENTVSYNC);
	if (g_renderer == NULL) {
		SDL_Log("Renderer creation failed: %s", SDL_GetError());
		goto failure;
	}
	if (sdl_apply_display_configuration() != SUCCESS) {
		goto failure;
	}

	g_screen = SDL_CreateRGBSurfaceWithFormat(
		0, logical_width, logical_height, 32, SDL_PIXELFORMAT_RGBA32);
	if (g_screen == NULL) {
		SDL_Log("Logical framebuffer creation failed: %s", SDL_GetError());
		goto failure;
	}
	g_screen_texture = SDL_CreateTextureFromSurface(g_renderer, g_screen);
	if (g_screen_texture == NULL) {
		SDL_Log("Framebuffer texture creation failed: %s", SDL_GetError());
		goto failure;
	}
	if (TTF_Init() == ERROR) {
		SDL_Log("SDL_ttf initialization failed: %s", TTF_GetError());
		goto failure;
	}
	g_ttf_initialized = true;
	return SUCCESS;

failure:
	close_sdl();
	return FAIL;
}

static int apply_renderer_logical_presentation(void)
{
	if (g_window == NULL || g_renderer == NULL) {
		SDL_Log("Cannot configure display without a window and renderer");
		return FAIL;
	}
	if (SDL_RenderSetLogicalSize(g_renderer, dm_get_screen_width(),
	                             dm_get_screen_height()) != 0) {
		SDL_Log("Could not set renderer logical size: %s", SDL_GetError());
		return FAIL;
	}
	if (SDL_RenderSetIntegerScale(g_renderer, SDL_FALSE) != 0) {
		SDL_Log("Could not enable proportional renderer scaling: %s", SDL_GetError());
		return FAIL;
	}
	return SUCCESS;
}

static int refresh_display_metrics(void)
{
	if (g_window == NULL || g_renderer == NULL) {
		SDL_Log("Cannot refresh display metrics without a window and renderer");
		return FAIL;
	}
	SDL_GetWindowSize(g_window, &g_window_width, &g_window_height);
	if (SDL_GetRendererOutputSize(g_renderer, &g_renderer_output_width,
	                              &g_renderer_output_height) != 0) {
		SDL_Log("Could not query renderer output size: %s", SDL_GetError());
		return FAIL;
	}
	return SUCCESS;
}

int sdl_apply_display_configuration(void)
{
	if (apply_renderer_logical_presentation() != SUCCESS) {
		return FAIL;
	}
	return refresh_display_metrics();
}

int sdl_handle_window_size_changed(int window_width, int window_height)
{
	if (window_width <= 0 || window_height <= 0) {
		return FAIL;
	}
	if (window_width == g_window_width && window_height == g_window_height) {
		return SUCCESS;
	}
	g_window_width = window_width;
	g_window_height = window_height;
	if (g_window != NULL && !sdl_is_fullscreen()) {
		SDL_GetWindowPosition(g_window, &g_windowed_rect.x, &g_windowed_rect.y);
		g_windowed_rect.w = window_width;
		g_windowed_rect.h = window_height;
		g_windowed_rect_valid = true;
	}
	if (g_renderer != NULL &&
	    SDL_GetRendererOutputSize(g_renderer, &g_renderer_output_width,
	                              &g_renderer_output_height) != 0) {
		SDL_Log("Could not query renderer output size after resize: %s", SDL_GetError());
		return FAIL;
	}
	return SUCCESS;
}

bool sdl_is_fullscreen(void)
{
	return g_window != NULL &&
	       (SDL_GetWindowFlags(g_window) & SDL_WINDOW_FULLSCREEN) != 0;
}

bool sdl_fullscreen_requested(int argc, char *args[])
{
	for (int index = 1; args != NULL && index < argc; index++) {
		if (strcmp(args[index], "-f") == 0 ||
		    strcmp(args[index], "--fullscreen") == 0) {
			return true;
		}
	}
	return false;
}

static int restore_windowed_rectangle(void)
{
	if (g_window == NULL) {
		return FAIL;
	}
	if (!g_windowed_rect_valid) {
		return SUCCESS;
	}
	int display_index = SDL_GetWindowDisplayIndex(g_window);
	SDL_Rect usable_bounds;
	if (display_index >= 0 &&
	    SDL_GetDisplayUsableBounds(display_index, &usable_bounds) == 0) {
		int width = g_windowed_rect.w < usable_bounds.w ?
		            g_windowed_rect.w : usable_bounds.w;
		int height = g_windowed_rect.h < usable_bounds.h ?
		             g_windowed_rect.h : usable_bounds.h;
		int max_x = usable_bounds.x + usable_bounds.w - width;
		int max_y = usable_bounds.y + usable_bounds.h - height;
		int x = g_windowed_rect.x;
		int y = g_windowed_rect.y;
		if (x < usable_bounds.x) x = usable_bounds.x;
		if (y < usable_bounds.y) y = usable_bounds.y;
		if (x > max_x) x = max_x;
		if (y > max_y) y = max_y;
		SDL_SetWindowSize(g_window, width, height);
		SDL_SetWindowPosition(g_window, x, y);
	} else {
		SDL_SetWindowSize(g_window, g_windowed_rect.w, g_windowed_rect.h);
		SDL_SetWindowPosition(g_window, g_windowed_rect.x, g_windowed_rect.y);
	}
	return SUCCESS;
}

static int transition_set_fullscreen(void *context, bool enabled)
{
	(void)context;
	Uint32 flags = enabled ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0;
	if (SDL_SetWindowFullscreen(g_window, flags) != 0) {
		SDL_Log("Could not set fullscreen state: %s", SDL_GetError());
		return FAIL;
	}
	return SUCCESS;
}

static int transition_restore_windowed_rect(void *context)
{
	(void)context;
	return restore_windowed_rectangle();
}

static int transition_apply_display_configuration(void *context)
{
	(void)context;
	return apply_renderer_logical_presentation();
}

static int transition_refresh_display_metrics(void *context)
{
	(void)context;
	return refresh_display_metrics();
}

static void transition_log_rollback_failure(void *context,
	                                        const char *operation)
{
	(void)context;
	SDL_Log("Fullscreen rollback failed while %s: %s", operation,
	        SDL_GetError());
}

int sdl_run_fullscreen_transition(
	bool was_fullscreen, bool enabled,
	const sdl_fullscreen_transition_operations_t *operations)
{
	if (operations == NULL || operations->set_fullscreen == NULL ||
	    operations->restore_windowed_rect == NULL ||
	    operations->apply_display_configuration == NULL ||
	    operations->refresh_display_metrics == NULL ||
	    operations->log_rollback_failure == NULL) {
		return FAIL;
	}
	if (was_fullscreen == enabled) {
		return SUCCESS;
	}
	if (operations->set_fullscreen(operations->context, enabled) != SUCCESS) {
		return FAIL;
	}
	if (!enabled &&
	    operations->restore_windowed_rect(operations->context) != SUCCESS) {
		SDL_Log("Could not restore the windowed rectangle after fullscreen");
		goto rollback;
	}
	if (operations->apply_display_configuration(operations->context) != SUCCESS) {
		SDL_Log("Could not apply display configuration after fullscreen change");
		goto rollback;
	}
	if (operations->refresh_display_metrics(operations->context) != SUCCESS) {
		SDL_Log("Could not refresh display metrics after fullscreen change");
		goto rollback;
	}
	return SUCCESS;

rollback:
	if (operations->set_fullscreen(operations->context, was_fullscreen) != SUCCESS) {
		operations->log_rollback_failure(operations->context,
		                                 "restoring fullscreen state");
	}
	if (!was_fullscreen &&
	    operations->restore_windowed_rect(operations->context) != SUCCESS) {
		operations->log_rollback_failure(operations->context,
		                                 "restoring the windowed rectangle");
	}
	if (operations->apply_display_configuration(operations->context) != SUCCESS) {
		operations->log_rollback_failure(operations->context,
		                                 "reapplying display configuration");
	}
	if (operations->refresh_display_metrics(operations->context) != SUCCESS) {
		operations->log_rollback_failure(operations->context,
		                                 "refreshing physical display metrics");
	}
	return FAIL;
}

int sdl_set_fullscreen(bool enabled)
{
	if (g_window == NULL) {
		SDL_Log("Cannot change fullscreen state without a window");
		return FAIL;
	}
	bool was_fullscreen = sdl_is_fullscreen();
	if (enabled == was_fullscreen) {
		return SUCCESS;
	}
	if (!was_fullscreen) {
		SDL_GetWindowPosition(g_window, &g_windowed_rect.x, &g_windowed_rect.y);
		SDL_GetWindowSize(g_window, &g_windowed_rect.w, &g_windowed_rect.h);
		g_windowed_rect_valid = g_windowed_rect.w > 0 && g_windowed_rect.h > 0;
	}
	const sdl_fullscreen_transition_operations_t operations = {
		.set_fullscreen = transition_set_fullscreen,
		.restore_windowed_rect = transition_restore_windowed_rect,
		.apply_display_configuration = transition_apply_display_configuration,
		.refresh_display_metrics = transition_refresh_display_metrics,
		.log_rollback_failure = transition_log_rollback_failure
	};
	return sdl_run_fullscreen_transition(was_fullscreen, enabled, &operations);
}

int toggle_fullscreen(void)
{
	return sdl_set_fullscreen(!sdl_is_fullscreen());
}

int load_media(void)
{
	char path[512];

	ax_get_resource_path(path, sizeof(path), "img/stop.png");
	stop_button = dw_load_texture(path);
	if (stop_button == NULL){
		printf("Could not load the level image SDL_Error: %s\n",
				SDL_GetError());
		return FAIL;
	}

	ax_get_resource_path(path, sizeof(path), "img/fast.png");
	fast_button = dw_load_texture(path);
	if (fast_button == NULL){
		printf("Could not load the level image SDL_Error: %s\n",
				SDL_GetError());
		return FAIL;
	}

	ax_get_resource_path(path, sizeof(path), "img/play.png");
	play_button = dw_load_texture(path);
	if (play_button == NULL){
		printf("Could not load the level image SDL_Error: %s\n",
				SDL_GetError());
		return FAIL;
	}

	ax_get_resource_path(path, sizeof(path), "img/step-forward.png");
	step_forward_button = dw_load_texture(path);
	if (step_forward_button == NULL){
		printf("Could not load the level image SDL_Error: %s\n",
				SDL_GetError());
		return FAIL;
	}

	ax_get_resource_path(path, sizeof(path), "img/return.png");
	return_button = dw_load_texture(path);
	if (return_button == NULL){
		printf("Could not load the level image SDL_Error: %s\n",
				SDL_GetError());
		return FAIL;
	}

	ax_get_resource_path(path, sizeof(path), "img/reset.png");
	reset_button = dw_load_texture(path);
	if (reset_button == NULL){
		printf("Could not load the level image SDL_Error: %s\n",
				SDL_GetError());
		return FAIL;
	}

	ax_get_resource_path(path, sizeof(path), "img/rightarrow.png");
	g_reg_arrow = dw_load_texture(path);
	if (g_reg_arrow == NULL){
		printf("Could not load the level image SDL_Error: %s\n",
				SDL_GetError());
		return FAIL;
	}
	
	ax_get_resource_path(path, sizeof(path), "img/rightarrow.png");
	g_imm_up_arrow = dw_load_texture(path);
	if (g_imm_up_arrow == NULL){
		printf("Could not load the level image SDL_Error: %s\n",
				SDL_GetError());
		return FAIL;
	}

	ax_get_resource_path(path, sizeof(path), "img/rightarrow.png");
	g_lv_arrow = dw_load_texture(path);
	if (g_lv_arrow == NULL){
		printf("Could not load the level image SDL_Error: %s\n",
				SDL_GetError());
		return FAIL;
	}

	ax_get_resource_path(path, sizeof(path), "img/rightarrow.png");
	g_ib_arrow = dw_load_texture(path);
	if (g_ib_arrow == NULL){
		printf("Could not load the level image SDL_Error: %s\n",
				SDL_GetError());
		return FAIL;
	}
	
	ax_get_resource_path(path, sizeof(path), "img/rightarrow.png");
	g_ob_arrow = dw_load_texture(path);
	if (g_ob_arrow == NULL){
		printf("Could not load the level image SDL_Error: %s\n",
				SDL_GetError());
		return FAIL;
	}

	ax_get_resource_path(path, sizeof(path), "img/rightarrow.png");
	g_exec_arrow = dw_load_texture(path);
	if (g_exec_arrow == NULL){
		printf("Could not load the level image SDL_Error: %s\n",
				SDL_GetError());
		return FAIL;
	}



	//Open the font
	ax_get_resource_path(path, sizeof(path), "fonts/DOSVGA437.ttf");
	g_font = TTF_OpenFont(path, 130); //96 old value point size
	if (NULL == g_font){
		printf("Failed to load font! SDL_ttf Error: %s\n", 
				TTF_GetError());
		return FAIL;
	} 
	
	
	return SUCCESS;
}

/* Function: close_sdl
 * ----------------------------------------------------------------------------
 * 	Closes all the resources regarding sdl.
 *
 * Arguments: 
 * 	None.
 *
 * Return:
 *	SUCCESS or FAIL
 */
void close_sdl(void)
{

//    if (g_studio_sfx)  Mix_FreeChunk(g_studio_sfx);
    if (g_sfx_type) Mix_FreeChunk(g_sfx_type);
    if (g_sfx_ready) Mix_FreeChunk(g_sfx_ready);
	if (g_font != NULL) {
		TTF_CloseFont(g_font);
		g_font = NULL;
	}
	if (g_screen_texture != NULL) {
		SDL_DestroyTexture(g_screen_texture);
		g_screen_texture = NULL;
	}
	if (g_screen != NULL) {
		SDL_FreeSurface(g_screen);
		g_screen = NULL;
	}
	if (g_renderer != NULL) {
		SDL_DestroyRenderer(g_renderer);
		g_renderer = NULL;
	}
	if (g_window != NULL) {
		SDL_DestroyWindow(g_window);
		g_window = NULL;
	}
    Mix_CloseAudio();
	if (g_ttf_initialized) {
		TTF_Quit();
		g_ttf_initialized = false;
	}
	if (g_sdl_initialized) {
		SDL_Quit();
		g_sdl_initialized = false;
	}
}
