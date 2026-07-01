#include <assert.h>
#include <string.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include "draw_dw.h"
#include "sdl_config.h"
#include "aux.h"
#include "dimensions_dm.h"

SDL_Color C_BLACK = {0, 0, 0, 255};
SDL_Color C_RED = {255, 0, 0, 255};
SDL_Color C_BLUE = {0, 0, 255, 255};
SDL_Color C_ORANGE = {0xF8, 0x71, 0x3A, 255};
SDL_Color C_CYAN = {0, 255, 255, 255};
SDL_Color C_YELLOW = {255, 255, 0, 255};
SDL_Color C_MAGENTA = {255, 0, 255, 255};
SDL_Color C_GREEN = {0, 255, 0, 255};
SDL_Color C_WHITE = {255,255,255,255};
SDL_Color C_VERYLIGHTGREY = {234, 234, 234, 255};
SDL_Color C_LIGHTGREY = {224, 224, 224, 255};
SDL_Color C_SILVERGREY = {192, 192, 192, 255};
SDL_Color C_GREY = {127, 127, 127, 255};
SDL_Color C_SHADOWGREY = {92, 92, 92, 255};
SDL_Color C_CHARCOALGREY = {74, 74, 74, 255};
SDL_Color C_DIMGREY = {64, 64, 64, 255};
SDL_Color C_DARKGRAPHITE = {40, 40, 40, 255};
SDL_Color C_DARKGREY = {31, 31, 31, 255};
SDL_Color C_SOFTBLACK = {20, 20, 20, 255};
SDL_Color C_NEARBLACK = {16, 16, 16, 255};
SDL_Color C_AMBER = {0xFF, 0xBF, 0x00, 255};

texture_t *g_arrow = NULL;

static Uint32 IFACE_BOX_BIG_W = 420;
static Uint32 IFACE_BOX_BIG_H = 330;

const Uint32 IFACE_FILLED_OFS = 10; // Used for Interface Messages
const Uint32 IFACE_INNER_BORDER = 4; // Used for Interface Messages
static const Uint32 IFACE_HEADER_BOX_H = 50;
static const Uint32 IFACE_HEADER_TEXT_H = 30;

static void draw_text(int x, int y, float s, SDL_Color c, char *t);
static int draw_scaled_texture(int x, int y, float s, texture_t *t);
static int get_ofs_iface_inner_border();
static int get_h_iface_header();
static SDL_Rect get_iface_box_big_wh();

/* Function: tx_get_iface_box_big_wh
 * -----------------------------------------------------------------------------
 * Return the width and heigth value of the text box
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the dimension of the object
 */
static SDL_Rect get_iface_box_big_wh()
{
	SDL_Rect b;
	b.w = dm_scale_to_res(IFACE_BOX_BIG_W);
	b.h = dm_scale_to_res(IFACE_BOX_BIG_H);
	b.x = 0;
	b.y = 0;
	return b;
}
/* Function: dw_get_iface_big_lower_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dw_get_iface_big_lower_box()
{
	SDL_Rect wh = get_iface_box_big_wh();
	SDL_Rect b;
	b.w = wh.w;
	b.h = wh.h;
	b.x = dm_get_screen_width()/2 - b.w/2;
	b.y = dm_get_screen_height() - b.h -
		  dw_get_ofs_iface_filled_border();
	return b;
}

/* Function: dw_get_iface_big_center_box
 * -----------------------------------------------------------------------------
 * Returns the box of the center screen for menus
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dw_get_iface_big_center_box()
{
	int w = dm_get_screen_width();
	int h = dm_get_screen_height();
	SDL_Rect wh = get_iface_box_big_wh();
	SDL_Rect b;
	b.w = wh.w;
	b.h = wh.h;
	b.x = (w - b.w)/2;
	b.y = (h - b.h)/2;
	return b;
}

/* Function: dm_get_ofs_iface_filled_border
 * -----------------------------------------------------------------------------
 *	Returns the interface filled border used for messages.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	The offset that will be use of the rectangle padding of the interface
 */
int dw_get_ofs_iface_filled_border()
{
	return dm_scale_to_res(IFACE_FILLED_OFS);
}

/* Function: get_get_h_iface_header_txt
 * -----------------------------------------------------------------------------
 *	Returns the height of the iface header_text
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	The height of the iface header text
 */
int dw_get_h_iface_header_txt()
{
	return dm_scale_to_res(IFACE_HEADER_TEXT_H);
}

/* Function: get_ofs_iface_filled_border
 * -----------------------------------------------------------------------------
 *	Returns the interface filled border used for messages.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	The offset that will be use of the rectangle padding of the interface
 */
static int get_ofs_iface_inner_border()
{
	return dm_scale_to_res(IFACE_INNER_BORDER);
}

/* Function: get_h_iface_title
 * -----------------------------------------------------------------------------
 *	Returns the height for the iface title
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	The height of the title
 */
static int get_h_iface_header()
{
	return dm_scale_to_res(IFACE_HEADER_BOX_H);
}

/* Function: dw_set_array_texture_color_mod
 * -----------------------------------------------------------------------------
 *	Set the texture color according to a color
 *
 * Arguments:
 *	t: texture;
 *  c: color 
 *
 * Return:
 *	Void.
 */
void dw_set_array_texture_color_mod(texture_array_t *t, SDL_Color c)
{
	for (int i = 0; i < t->size; i++){
		SDL_SetTextureColorMod(t->t[i]->texture, c.r, c.g, c.b);
	}
}

/* Function: dw_set_texture_color_mod
 * -----------------------------------------------------------------------------
 *	Set the texture color according to a color
 *
 * Arguments:
 *	t: texture;
 *  c: color 
 *
 * Return:
 *	Void.
 */
void dw_set_texture_color_mod(texture_t *t, SDL_Color c)
{	
	SDL_SetTextureColorMod(t->texture, c.r, c.g, c.b);
}

/* Function: dw_draw_inner_shadow_lines
 * -----------------------------------------------------------------------------
 * Draws a rectangle of inner shadow lines and two colors
 * 
 * Arguments:
 *	r: The box where the rectangle will be drawn
 * 	top_left: The color for the top left lines
 * 	bottom_right: The color of the bottom right lines
 *	 
 * Return:
 *	Void.
 */
void dw_draw_inner_shadow_lines(SDL_Rect r, 
								SDL_Color top_left, 
								SDL_Color bottom_right)
{
	SDL_SetRenderDrawColor(g_renderer, 
						   top_left.r, 	
						   top_left.g, 
						   top_left.b, 
						   255);
	SDL_RenderDrawLine(g_renderer, r.x, r.y, r.x + r.w - 1, r.y);
	SDL_RenderDrawLine(g_renderer, r.x, r.y, r.x, r.y + r.h - 1);
	SDL_SetRenderDrawColor(g_renderer, 
						   bottom_right.r, 	
						   bottom_right.g, 
						   bottom_right.b, 
						   255);
	SDL_RenderDrawLine(g_renderer, r.x + r.w - 1, 
								   r.y, 
								   r.x + r.w - 1, 
								   r.y + r.h - 1);
	SDL_RenderDrawLine(g_renderer, r.x, 
								   r.y + r.h - 1, 
								   r.x + r.w - 1, 
								   r.y + r.h - 1);
	SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
}

/* Function: dw_draw_thick_rect
 * -----------------------------------------------------------------------------
 * Draws a rectangle with a given thickness
 * 
 * Arguments:
 *	b: The box where the rectangle will be drawn
 * 	w: The width of the rectangle line
 * 	c: The color of the line of the rectangle
 *	 
 * Return:
 *	Void.
 */
void dw_draw_thick_rect(SDL_Rect b, int w, SDL_Color c)
{
	dw_draw_filled_rectangle(b, c, c);
	b.x += w;
	b.y += w;
	b.w -= 2*w;
	b.h -= 2*w;
	dw_draw_filled_rectangle(b, C_BLACK, C_BLACK);
}

/* Function: dw_get_iface_content_box
 * -----------------------------------------------------------------------------
 * Returns the content box available for use of an iface_box
 * 
 * Arguments:
 *	b: The whole box of the iface box
 *	 
 * Return:
 *	Void.
 */
SDL_Rect dw_get_iface_content_box(SDL_Rect b)
{
	
	int offset = dw_get_ofs_iface_filled_border();
	SDL_Rect in = ax_pad_rectangle(b, offset, true);
	int inner_border = get_ofs_iface_inner_border();
	
	in = ax_pad_rectangle(in, inner_border, true);
	
	in = ax_pad_rectangle(in, inner_border, true);

	int header_box_h = get_h_iface_header();
	
	in = ax_pad_rectangle(in, inner_border, true);

	in.y += (header_box_h + inner_border);
	in.h -= (header_box_h + inner_border);

	return in;
}


/* Function: dw_draw_iface_box
 * -----------------------------------------------------------------------------
 * Drawns an interface box with the assigned colors
 * 
 * Arguments:
 *	b: The box where the rectangle will be drawn
 *	 
 * Return:
 *	Void.
 */
void dw_draw_iface_box(SDL_Rect b, texture_t *header)
{
	dw_draw_filled_rectangle(b, C_GREY, C_DARKGREY);
	
	int offset = dw_get_ofs_iface_filled_border();
	SDL_Rect in = ax_pad_rectangle(b, offset, true);
	int inner_border = get_ofs_iface_inner_border();
	dw_draw_thick_rect(in, inner_border, C_SOFTBLACK);
	
	in = ax_pad_rectangle(in, inner_border, true);
	dw_draw_thick_rect(in, inner_border, C_CHARCOALGREY);
	
	in = ax_pad_rectangle(in, inner_border, true);
	dw_draw_thick_rect(in, inner_border, C_SOFTBLACK);

	SDL_Rect header_box = in; 
	header_box.h = get_h_iface_header();
	
	in = ax_pad_rectangle(in, inner_border, true);
	
//	dw_draw_filled_rectangle(in, C_DARKGRAPHITE, C_DARKGRAPHITE);// INNER_REC
	dw_draw_filled_rectangle(dw_get_iface_content_box(b), 
							 C_DARKGRAPHITE, 
							 C_DARKGRAPHITE);
	dw_draw_filled_rectangle(header_box, C_SOFTBLACK, C_SOFTBLACK);
	
	// upper title-header section
	SDL_Rect header_bar = header_box;
	header_bar.y += header_box.h;
	header_bar.h = inner_border;

	// lower bars
	dw_draw_filled_rectangle(header_bar, C_CHARCOALGREY, C_CHARCOALGREY);
	header_bar.y += inner_border;
	dw_draw_filled_rectangle(header_bar, C_SOFTBLACK, C_SOFTBLACK);

	if (header != NULL){
 		int text_h = dw_get_h_iface_header_txt();
		int text_w = ax_get_texture_w_fit_h(text_h, header);
	
		SDL_Rect header_text = {.x = header_box.x + (header_box.w - text_w)/2,
								.y = header_box.y + (header_box.h - text_h)/2,	
								.w = text_w,
								.h = text_h};
		dw_draw_texture_fit_h(header_text, header);
	}

}

/* Function: dw_draw_rotated_texture_fits_height
 * -----------------------------------------------------------------------------
 * Draws a texture that scales correctly to a given height, it has the 
 * transform capabilities of rotating the texture from the center to a given
 * angle
 * 
 * Arguments:
 *	x: position x of the texture.
 *	y: position y of the texture.
 *	h: height to which the texture will be scaled proporcionally.
 *	angle: angle to which the texture will be rotated to.
 *	t: texture object that is going to be drawn.
 *
 * Return:
 *	SUCCESS or FAIL
 */
void dw_draw_rotated_texture_fits_h(int x, int y, int h, double angle, 
															      texture_t *t)
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
	
	SDL_Point center = {d.w/2, d.h/2};
	SDL_RendererFlip flip = SDL_FLIP_NONE;
	
	if (SDL_RenderCopyEx(g_renderer, t->texture, NULL, &d, angle, &center, 
																	flip) < 0){
		printf("Texture could not be copied SDL_Error: %s\n", SDL_GetError());
	}
}

/* Function: dw_draw_texture_fit_h
 * -----------------------------------------------------------------------------
 * Draws a texture scaling it correctly to a given height
 * 
 * Arguments:
 *	r: rectangle position of the texture, height value will be used
 *	t: texture object that is going to be drawn.
 *
 * Return:
 *	SUCCESS or FAIL
 */
int dw_draw_texture_fit_h(SDL_Rect r, texture_t *t)
{
	assert(r.h > 0 && "The height value is invalid");
	assert(t != NULL && "The texture pointer cannot be NULL");

	// The scaling of the image resolution
	float w = (float) (t->w * r.h)/t->h;

	SDL_Rect d;

	d.x = r.x;
	d.y = r.y;
	d.w =(int)w;
	d.h = r.h;
	
	if (SDL_RenderCopy(g_renderer, t->texture, NULL, &d) < 0){
		printf("Texture could not be copied SDL_Error: %s\n", 
				SDL_GetError());
		return FAIL;
	}
	return SUCCESS;
}

/* Function: dw_draw_texture_fit_h
 * -----------------------------------------------------------------------------
 * Draws a texture scaling it correctly to a given height
 * 
 * Arguments:
 *	r: rectangle position of the texture, height value will be used
 *	t: texture object that is going to be drawn.
 *
 * Return:
 *	SUCCESS or FAIL
 */
int dw_draw_texture_center_fit_h(SDL_Rect r, texture_t *t)
{
	assert(r.h > 0 && "The height value is invalid");
	assert(t != NULL && "The texture pointer cannot be NULL");
	

	// The scaling of the image resolution
	float w = (float) (t->w * r.h)/t->h;

	SDL_Rect d;

	d.x = r.x + r.w/2 - (int)w/2;
	d.y = r.y;
	d.w =(int)w;
	d.h = r.h;
	
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
 *	r: rectangle position of the texture, width value will be used
 *	t: texture object that is going to be drawn.
 *
 * Return:
 *	SUCCESS or FAIL
 */
int dw_draw_texture_fits_width(SDL_Rect r, texture_t *t)
{
	assert(r.w > 0 && "The width value is invalid");
	assert(t != NULL && "The texture pointer cannot be NULL");

	// The scaling of the image resolution
	float h = (float) (t->h * r.w)/t->w;

	SDL_Rect d;

	d.x = r.x;
	d.y = r.y;
	d.w = r.w;
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
static int draw_scaled_texture(int x, int y, float s, texture_t *t)
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

/* Function: dw_create_text_tex
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
texture_t *dw_create_text_tex(char *texture_text, 
									   	   SDL_Color text_color)
{
	assert(NULL != texture_text && "The texture text is NULL");
	
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

/* Function: dw_free_texture_array
 * ----------------------------------------------------------------------------
 * Frees a texture array
 * 
 * Arguments:
 *	texture: Pointer of the texture array to be freed
 * 
 * Return:
 * 	Void
 */
void dw_free_texture_array(texture_array_t *t)
{
	if (t != NULL){
	
	for (int i = 0; i < t->size; i++){
		dw_free_texture(t->t[i]);
		t->t[i] = NULL;
	}
	free(t->t);
	t->t = NULL;
	free(t);
	t = NULL;
	}
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
	if (texture != NULL){
		if (texture->texture != NULL){
			SDL_DestroyTexture(texture->texture);
			texture->texture = NULL;
			texture->w = 0;
			texture->h = 0;
		} 	
		free(texture);
	}
}

/* Function: dw_create_text_tex_array_by_h
 *-----------------------------------------------------------------------------
 * Cretes an array texture for showing wrapped text. The dimension of a given
 * line of the texture is fit by height in a give width dimension.
 *
 * Arguments:
 *	w: width to the column of the text.
 *	h: height of the text
 *  c: color of the drawn text
 *	t: text to be drawn on screen.
 *
 * Return:
 *	Void
 */
texture_array_t *dw_create_text_tex_array_by_h(int w, int h, SDL_Color c, char *t)
{
	assert(w > 0 && "The width of the text is negative");
	assert(h > 0 && "The height of the text is negative");
	assert(t != NULL && "Text pointer is NULL");

	texture_array_t *array = malloc(sizeof(texture_array_t));
	array->size = ax_get_wrapped_text_height(w, h, t)/h;
	array->t = malloc(sizeof(texture_t*)*array->size);

	int y_offset = h;

	int string_size = strlen(t);
	char *text = malloc(sizeof(char)*string_size);
	
	int last_fit = 0;
	int already_drawn = 0;

	int pos = 0;
	for (int i = 0; i <= string_size; i++){
		if (t[i] == CHAR_SPACE || t[i] == CHAR_NULL || t[i] == CHAR_NEWLINE){
			
			strncpy(text, t + already_drawn, i - already_drawn);
			
			if (check_text_fits_width_by_height(text, h, w) == true){
				last_fit = i;
			}
			if (check_text_fits_width_by_height(text, h, w) == false || 
				t[i] == CHAR_NEWLINE || t[i] == CHAR_NULL){
				if (last_fit == already_drawn){
					puts("The size of the font does not fits width");
					last_fit = string_size;
				}
				memset(text, 0, string_size);
			
				if (already_drawn == 0){
					strncpy(text, t + already_drawn, last_fit - already_drawn);	
				} else {
					strncpy(text, t + already_drawn+1, last_fit - already_drawn - 1);	
				}	

				texture_t *text_texture = dw_create_text_tex(text, c);
				array->t[pos] = text_texture;
				pos++;
				already_drawn = last_fit;
				i = last_fit;
				memset(text, 0, string_size);
			} 
		}
	} 
	free(text);
error:
	return array;
}

/* Function: dw_draw_wrapped_texture_by_h
 *-----------------------------------------------------------------------------
 * This function draws text wrapped as images in places in the screen, 
 *
 * Arguments:
 *	r: rectangle with the position of the texture
 *	h: height value of the texture
 *  a: array of the texture to be drawn
 *
 * Return:
 *	Void
 */
void dw_draw_wrapped_texture_by_h(SDL_Rect r, int h, texture_array_t *a)
{
	assert(a != NULL && "Text pointer is NULL");

	int y_pos = r.y + (r.h - a->size*h)/2;
	int y_offset = h;
	
	for (int i = 0; i < a->size; i++){
		int w = ax_get_texture_w_fit_h(h, a->t[i]);
		int x_pos = r.x + (r.w - w)/2;
		SDL_Rect r = {.x = x_pos, .y = y_pos, .w = r.w, .h = h};
		dw_draw_texture_fit_h(r, a->t[i]);
		y_pos += y_offset;
	}	 
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

	text_texture = dw_create_text_tex(t, c);
	assert(text_texture != NULL && "Failed to load texture from rendered text");

	int status = SUCCESS;	
	status = draw_scaled_texture(x, y, s, text_texture);
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
void dw_draw_rectangle(SDL_Rect r, SDL_Color c)
{
	SDL_Rect rect = r;
	SDL_SetRenderDrawColor(g_renderer, c.r, c.g, c.b, c.a);
	SDL_RenderDrawRect(g_renderer, &rect);
	SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
}

/* Function: dw_draw_filled_rectangle
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
void dw_draw_filled_rectangle(SDL_Rect r, SDL_Color in, SDL_Color out)
{
	SDL_Rect rect = r;
	SDL_SetRenderDrawColor(g_renderer, in.r, in.g, in.b, in.a);
	SDL_RenderFillRect(g_renderer, &rect);
	SDL_SetRenderDrawColor(g_renderer, out.r, out.g, out.b, out.a);
	SDL_RenderDrawRect(g_renderer, &rect);
	SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
}


