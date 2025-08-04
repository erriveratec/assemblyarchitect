#include <assert.h>
#include <string.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "draw_dw.h"
#include "sdl_config.h"
#include "aux.h"
#include "dbg.h"

SDL_Color COLOR_WHITE = {255,255,255,255};
SDL_Color COLOR_BLACK = {0, 0, 0, 255};
SDL_Color COLOR_RED = {255, 0, 0, 255};
SDL_Color COLOR_GREY = {127, 127, 127, 255};
SDL_Color COLOR_ORANGE = {0xF8, 0x71, 0x3A, 255};
SDL_Color COLOR_CYAN = {0, 255, 255, 255};

static void draw_text(int x, int y, float s, SDL_Color c, char *t);

/* Function: dw_draw_texture_fits_height
 * -----------------------------------------------------------------------------
 * Draws a texture scaling it correctly to a given height
 * 
 * Arguments:
 *	x: position x of the texture.
 *	y: position y of the texture.
 *	s: scaling factor of the texture.
 *	t: texture object that is going to be drawn.
 *
 * Return:
 *	SUCCESS or FAIL
 */
int dw_draw_texture_fits_height(int x, int y, int h, texture_t *t)
{
	assert(h >= 0 && "The height value is invalid");
	assert(t != NULL && "The texture pointer cannot be NULL");

	// The scaling of the image resolution
	float w = (float) (t->w * h)/t->h;

	SDL_Rect d;

	d.x = x;
	d.y = y;
	d.w =(int)w;
	d.h = h;
	
	if (SDL_RenderCopy(g_renderer, t->texture, NULL, &d) < 0){
		printf("Texture could not be copied SDL_Error: %s\n", 
				SDL_GetError());
		return FAIL;
	}
	return SUCCESS;
}

/* Function: dw_draw_texture_fits_width
 * -----------------------------------------------------------------------------
 * Draws a texture scaling it correctly to a given height
 * 
 * Arguments:
 *	x: position x of the texture.
 *	y: position y of the texture.
 *	s: scaling factor of the texture.
 *	t: texture object that is going to be drawn.
 *
 * Return:
 *	SUCCESS or FAIL
 */
int dw_draw_texture_fits_width(int x, int y, int w, texture_t *t)
{
	assert(w >= 0 && "The width value is invalid");
	assert(t != NULL && "The texture pointer cannot be NULL");

	// The scaling of the image resolution
	float h = (float) (t->h * w)/t->w;

	SDL_Rect d;

	d.x = x;
	d.y = y;
	d.w = w;
	d.h = (int)h;
	
	if (SDL_RenderCopy(g_renderer, t->texture, NULL, &d) < 0){
		printf("Texture could not be copied SDL_Error: %s\n", 
				SDL_GetError());
		return FAIL;
	}
	return SUCCESS;
}

/* Function: draw_scaled_texture
 * -------------------------------------------------------------
 * This function receives as a argument a texture, the positions
 * where the texture is going to be located and a scaling factor
 * that will be used.
 * 
 * Arguments:
 *	x: position x of the texture.
 *	y: position y of the texture.
 *	s: scaling factor of the texture.
 *	t: texture object that is going to be drawn.
 *
 * Return:
 *	SUCCESS or FAIL
 */
int draw_scaled_texture(int x, int y, float s, texture_t *t)
{
	assert(s >= 0 && "The scaling factor is negative");
	assert(t != NULL && "The texture pointer cannot be NULL");

	// image resolution
	float w = (float) t->w;
	float h = (float) t->h;
	float ratio = w/h;

	float scaled_w = w - (h - h*s)*ratio;
	float scaled_h = h*s;

	SDL_Rect d;

	d.x = x;
	d.y = y;
	d.w = scaled_w;
	d.h = scaled_h;
	
	if (SDL_RenderCopy(g_renderer, t->texture, NULL, &d) < 0){
		printf("Texture could not be copied SDL_Error: %s\n", 
				SDL_GetError());
		return FAIL;
	}
	return SUCCESS;
}

/* Function: load_texture_from_file
 * ----------------------------------------
 * This function receives as an argument the path of the texture
 * to be loaded and returns the pointer to the loaded texture.
 * 
 * Arguments:
 *	Path to the img file.
 * 
 * Return:
 * 	Pointer to the texture object.
 */
texture_t *load_texture_from_file(char *path)
{
	assert(path != NULL && "The path pointer can't be NULL");

	int width, height, access;
	unsigned int format;

	texture_t *new_texture = (texture_t*) malloc(sizeof(texture_t));
	SDL_Texture *loaded_texture = NULL;

	SDL_Surface *loaded_surface = IMG_Load(path);
	
	if (loaded_surface == NULL){
		printf("Unable to create texture from %s!, Error: %s\n",
				path, SDL_GetError());
	} else{
		loaded_texture = SDL_CreateTextureFromSurface(g_renderer,
							loaded_surface);
		if (loaded_texture == NULL){
			printf("Unable to create texture from %s!, Error: %s\n",
					path, SDL_GetError());
		}
		SDL_FreeSurface(loaded_surface);
	}
	
	SDL_QueryTexture(loaded_texture, &format, &access, &width, &height);
	new_texture->w = width;
	new_texture->h = height;
	new_texture->texture = loaded_texture;

	return new_texture;
}
/* Function: load_texture_from_rendered_text
 * ----------------------------------------
 * This function receives as an argument the path of the texture
 * to be loaded and returns the pointer to the loaded texture.
 * 
 * Arguments:
 *	*texture_text: Text of the texture
 *	color: Color in which the texture will be rendered.
 * 
 * Return:
 * 	Pointer to the texture object.
 */
texture_t *load_texture_from_rendered_text(char *texture_text, 
									   	   SDL_Color text_color)
{
	assert(NULL != texture_text && "The path pointer can't be NULL");
	
	int width, height, access;
	unsigned int format;

	texture_t *new_texture = (texture_t*) malloc(sizeof(texture_t));
	SDL_Texture *created_texture = NULL;

	//Render text surface
	SDL_Surface *text_surface = TTF_RenderText_Solid(g_font, 
													 texture_text,
													 text_color);
	if (text_surface == NULL){
		printf("Unable to render text surface! SDL_ttf Error: %s\n", 
			  TTF_GetError());
		return NULL;
	} else {
		created_texture = SDL_CreateTextureFromSurface(g_renderer, 
													   text_surface);
		if (NULL == created_texture){
			printf("Unable to create texture from rendered text!"
				   "SDL Error: %s\n", SDL_GetError());	
			return NULL;
		} 
		
		SDL_FreeSurface(text_surface);
	}

	SDL_QueryTexture(created_texture, &format, &access, &width, &height);
	new_texture->w = width;
	new_texture->h = height;
	new_texture->texture = created_texture;

	//Return success
	return new_texture;
}

/* Function: dw_free_texture
 * ----------------------------------------------------------------------------
 * This function receives as an argument the pointer of the texture that
 * is going to be free
 * 
 * Arguments:
 *	*texture: Pointer of the texture to be destroyed
 * 
 * Return:
 * 	Void
 */
void dw_free_texture(texture_t *texture)
{
	if (NULL != texture->texture){
		SDL_DestroyTexture(texture->texture);
		texture->texture = NULL;
		texture->w = 0;
		texture->h = 0;
	} //end if
	free(texture);
}



/* Function: dw_draw_wrapped_text_fits_height
 * This function draws text wrapped as images in places in the screen, 
 * the function wraps text accordingly to the scaling factor needed.
 * It has the haight as a parameter to do the drawing
 *
 * Arguments:
 *	x: position x of the text.
 *	y: position y of the text.
 *	w: width to the column of the text.
 *	s: scaling factor of the text.
 *  c: color of the drawn text
 *	t: text to be drawn on screen.
 *
 * Return:
 *	Void
 */
void dw_draw_wrapped_text_fits_height(int x, int y, int w, int h, SDL_Color c, 
							       char *t)
{
	assert(0 < w && "The width of the text is negative");
	assert(0 < h && "The height of the text is negative");
	assert( NULL != t && "Text pointer is NULL");

	int x_pos = x + WRAPPED_TEXT_X_OFFSET;
	int y_pos = y + WRAPPED_TEXT_Y_OFFSET;
	int y_offset = h;
	int string_size = strlen(t);
	char *text = malloc(sizeof(char)*string_size);
	check_mem(text);

	int last_successful_fit = 0;
	int already_drawn_offset = 0;

		for (int i = 0; i <= string_size; i++){
			if (t[i] == CHAR_SPACE|| t[i] ==  CHAR_NULL){
				
				strncpy(text, t + already_drawn_offset, 
						i - already_drawn_offset);
			
				if (check_if_text_fits_in_width_by_height(text, h, w) == true){
					last_successful_fit = i;
				}

				if (check_if_text_fits_in_width_by_height(text, h, w) == false|| 
					t[i] == CHAR_NULL){
					if (last_successful_fit == already_drawn_offset){
						puts("The size of the font does not fits width");
						last_successful_fit = string_size;
					}
					memset(text, 0, string_size);
					strncpy(text, t + already_drawn_offset, 
						   	last_successful_fit - already_drawn_offset);
					dw_draw_text_fits_height(x_pos, y_pos, h, c, text);
					y_pos += y_offset;
					already_drawn_offset = last_successful_fit;
					i = last_successful_fit;
					memset(text, 0, string_size);
			}	 
		}
	} 
	free(text);
error:
	return;
}

/* Function: draw_text
 * This function draws text as images in places in the screen, the 
 * function is created to reduce the ammount of verbose in other
 * functions
 *
 * Arguments:
 *	x: position x of the text.
 *	y: position y of the text.
 *	s: scaling factor of the text.
 *  c: color of the drawn text
 *	t: text to be drawn on screen.
 *
 * Return:
 *	Void
 */
static void draw_text(int x, int y, float s, SDL_Color c, char *t)
{
	assert(NULL != t && "The text pointer is NULL");

	texture_t *text_texture = NULL;

	text_texture = load_texture_from_rendered_text(t, c);
	assert(text_texture != NULL && "Failed to load texture from rendered text");

	int status = SUCCESS;	
	status = draw_scaled_texture(x, y, s, text_texture);
	assert(FAIL != status && "The texture could not be drawn");

	dw_free_texture(text_texture);
}

/* Function: dw?draw_text_fits_height
 * -----------------------------------------------------------------------------
 * This function draws text as images, the scale of the image is adjusted
 * to fit the given height.
 *
 * Arguments:
 *	x: position x of the text.
 *	y: position y of the text.
 *	h: height value that the text must fit.
 *  color: color of the drawn text
 *	text: text to be drawn on screen.
 *
 * Return:
 *	Void
 */
void dw_draw_text_fits_height(int x, int y, int h, SDL_Color color, char *text)
{
	assert(text != NULL && "The text pointer is NULL");

	texture_t *text_texture = NULL;

	text_texture = load_texture_from_rendered_text(text, color);
	assert(NULL != text_texture && 
		   "Failed to load texture from rendered text");

	float scale = get_scale_fit_height(h, text_texture);

	int status = SUCCESS;	
	status = draw_scaled_texture(x, y, scale, text_texture);
	assert(FAIL != status && "The texture could not be drawn");

	dw_free_texture(text_texture);
}

/* Function: dw_draw_text_fits_width
 * -----------------------------------------------------------------------------
 * This function draws text as images, the scale of the image is adjusted
 * to fit the given width.
 *
 * Arguments:
 *	x: position x of the text.
 *	y: position y of the text.
 *	w: width value that the text must fit.
 *  color: color of the drawn text
 *	text: text to be drawn on screen.
 *
 * Return:
 *	Void
 */
void dw_draw_text_fits_width(int x, int y, int w, SDL_Color color, char *text)
{
	assert(text != NULL && "The text pointer is NULL");

	texture_t *text_texture = NULL;

	text_texture = load_texture_from_rendered_text(text, color);
	assert(NULL != text_texture && 
		   "Failed to load texture from rendered text");

	float scale = get_scale_fit_width(w, text_texture);

	int status = SUCCESS;	
	status = draw_scaled_texture(x, y, scale, text_texture);
	assert(FAIL != status && "The texture could not be drawn");

	dw_free_texture(text_texture);
}

/* Function: dw_draw_rectangle
 *----------------------------------------------------------------------------- 
 * Arguments:
 *	x: position x of the upper left corner.
 *	y: position y of the upper left corner.
 *	w: width of the rectangle.
 *  h: height of the rectangle.
 *	c: color of the rectangle.
 *
 * Return:
 *	Void
 */
void dw_draw_rectangle(int x, int y, int w, int h, SDL_Color c)
{
	SDL_Rect rect = {x,y,w,h};
	SDL_SetRenderDrawColor(g_renderer, c.r, c.g, c.b, c.a);
	SDL_RenderDrawRect(g_renderer, &rect);
	SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
}

/* Function: dw_ draw_filled_rectangle
 *----------------------------------------------------------------------------- 
 * Draw rectangle with filled content, a color for tthe outline can be 
 * specified
 *
 * Arguments:
 *	x: position x of the upper left corner.
 *	y: position y of the upper left corner.
 *	w: width of the rectangle.
 *  h: height of the rectangle.
 *	inside: color of the inside of the rectangle.
 * 	outline: color of the outline of the rectangle.
 *
 * Return:
 *	Void
 */
void dw_draw_filled_rectangle(int x, int y, int w, int h, SDL_Color inside,
							  SDL_Color outline)
{
	SDL_Rect rect = {x,y,w,h};
	SDL_SetRenderDrawColor(g_renderer, inside.r, inside.g, inside.b, inside.a);
	SDL_RenderFillRect(g_renderer, &rect);
	SDL_SetRenderDrawColor(g_renderer, outline.r, outline.g, outline.b, 
						   outline.a);
	SDL_RenderDrawRect(g_renderer, &rect);
	SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
}

/* Function: draw_wrapped_text
 * This function draws text wrapped as images in places in the screen, 
 * the function wraps text accordingly to the scaling factor needed.
 *
 * Arguments:
 *	x: position x of the text.
 *	y: position y of the text.
 *	w: width to the column of the text.
 *	s: scaling factor of the text.
 *  c: color of the drawn text
 *	t: text to be drawn on screen.
 *
 * Return:
 *	Void
 */
/*void draw_wrapped_text(int x, int y, int w, float s, SDL_Color c, char *t)
{
	assert(0 < w && "The width of the text is negative");
	assert(0 < s && "The scale of the text is negative");
	assert( NULL != t && "Text pointer is NULL");

	int x_pos = x + WRAPPED_TEXT_X_OFFSET;
	int y_pos = y + WRAPPED_TEXT_Y_OFFSET;
	int y_offset = get_text_height(t, s);
	int string_size = strlen(t);
	char *text = malloc(sizeof(char)*string_size);
	check_mem(text);

	int last_successful_fit = 0;
	int already_drawn_offset = 0;

	printf("Original text %s\n", t);
		for (int i = 0; i <= string_size; i++){
			if (t[i] == CHAR_SPACE|| t[i] ==  CHAR_NULL){
				
				strncpy(text, t + already_drawn_offset, 
						i - already_drawn_offset);
				printf("The value of text %s\n", text);
				if (check_if_text_fits_in_width(text, s, w) == true){
					last_successful_fit = i;
				}

				if (check_if_text_fits_in_width(text, s, w) == false|| 
					t[i] == CHAR_NULL){
					memset(text, 0, string_size);
					strncpy(text, t + already_drawn_offset, 
						   	last_successful_fit - already_drawn_offset);
					draw_text(x_pos, y_pos, s, c, text);
					y_pos += y_offset;
					already_drawn_offset = last_successful_fit;
					i = last_successful_fit;
					memset(text, 0, string_size);
			}	 
		}
	} 
	free(text);
error:
	return;
}
*/

