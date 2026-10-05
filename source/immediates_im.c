#include "immediates_im.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "code_line_cl.h"
#include "dimensions_dm.h"
#include "draw_dw.h"
#include "aux.h"
#include "ui/button_bt.h"
#include "registers_rg.h"
#include "ui/typography_ty.h"

#define TOTAL_IMM 20
#define IMM_FIRST_ROW_COUNT 11
#define IMM_SECOND_ROW_COLUMN_OFFSET 1
#define IMM_CELL_BORDER_WIDTH 5
#define IMM_LABEL_HIGHLIGHT_ENABLED 0
static const int IMM_CELL_WIDTH = 60;
static const int IMM_CELL_HEIGHT = 50;
static const int IMM_LABEL_X = 650;
static const int IMM_LABEL_Y = 0;


texture_t *g_imm_txt = NULL;

static bool g_imm_up = false;
static unsigned int g_imm_highlight_sources;
static bool g_imm_highlight;
static bool g_imm_highlight_rendered;
static dw_pulse_t g_imm_pulse;
static bool g_imm_assets_initialized;

typedef struct imm_cell_borders_t {
	bool left;
	bool right;
	bool top;
	bool bottom;
} imm_cell_borders_t;

typedef struct imm_t {
	btn_t *b;
	vb_value_box_t val;
	imm_cell_borders_t borders;
} imm_t;

static void init_imm_texture();
static void draw_imm_txt_up(float pulse);
static void draw_imm_cell(const imm_t *imm, float pulse, bool highlighted);
static void draw_imm_cell_frame(const imm_t *imm);
static void draw_imm_value(const vb_value_box_t *value, float pulse,
						   bool highlighted);
static imm_cell_borders_t get_imm_cell_borders(int index);
static bool rects_overlap_vertically(SDL_Rect first, SDL_Rect second);
static bool rects_overlap_horizontally(SDL_Rect first, SDL_Rect second);
static bool is_valid_imm_id(int id);
static SDL_Point get_grid_origin(void);
static SDL_Rect get_cell_bounds_by_index(int index);

static imm_t g_up_imm[TOTAL_IMM];

SDL_Rect im_get_grid_bounds(void)
{
	SDL_Point origin = get_grid_origin();
	SDL_Rect cell = im_get_cell_size();
	SDL_Rect bounds = {
		.x = origin.x,
		.y = origin.y,
		.w = IMM_FIRST_ROW_COUNT * cell.w,
		.h = 2 * cell.h
	};
	return bounds;
}

SDL_Rect im_get_label_bounds(void)
{
	SDL_Rect grid = im_get_grid_bounds();
	int text_h = ty_stage_label_height();
	int text_w = get_text_width_fits_height(text_h, IMM_TXT);
	SDL_Rect bounds = {
		.x = grid.x + (grid.w - text_w) / 2,
		.y = grid.y + grid.h,
		.w = text_w,
		.h = text_h
	};
	return bounds;
}

SDL_Rect im_get_cell_size(void)
{
	SDL_Rect size = {
		.x = 0,
		.y = 0,
		.w = dm_scale_to_res(IMM_CELL_WIDTH),
		.h = dm_scale_to_res(IMM_CELL_HEIGHT)
	};
	return size;
}

static bool is_valid_imm_id(int id)
{
	return id > IMM_MIN && id < IMM_MAX;
}

static SDL_Point get_grid_origin(void)
{
	SDL_Rect input_bounds = rg_get_initial_input_value_box_bounds();
	SDL_Point origin = {
		.x = dm_scale_to_res(IMM_LABEL_X),
		.y = dm_scale_to_res(IMM_LABEL_Y)
	};
	return origin;
}

static SDL_Rect get_cell_bounds_by_index(int index)
{
	assert(index >= 0 && index < TOTAL_IMM);
	if (index < 0 || index >= TOTAL_IMM) {
		return (SDL_Rect){0};
	}
	SDL_Point origin = get_grid_origin();
	SDL_Rect cell = im_get_cell_size();
	bool first_row = index < IMM_FIRST_ROW_COUNT;
	int column = first_row ? index :
	             index - IMM_FIRST_ROW_COUNT + IMM_SECOND_ROW_COLUMN_OFFSET;
	int row = first_row ? 0 : 1;
	SDL_Rect bounds = {
		.x = origin.x + column * cell.w,
		.y = origin.y + row * cell.h,
		.w = cell.w,
		.h = cell.h
	};
	return bounds;
}

/* Function: im_get_buffer_value_box_x_coord_by_id
 *------------------------------------------------------------------------------
 * Arguments:
 *	op_id: The id of the operand that the x value will get calculated
 *
 * Return:
 *	x coordinate of the first value box
 */
int im_get_imm_value_box_x_coord_by_id(int id)
{
	assert(is_valid_imm_id(id) && "Invalid immediate id");
	if (!is_valid_imm_id(id)) {
		return 0;
	}
	
	int index = id - IMMUP0;

	return g_up_imm[index].val.box.x;
}

/* Function: im_get_buffer_value_box_y_coord_by_id
 *------------------------------------------------------------------------------
 * Arguments:
 *	op_id: The id of the operand that the x value will get calculated
 *
 * Return:
 *	y coordinate of the first value box
 */
int im_get_imm_value_box_y_coord_by_id(int id)
{
	assert(is_valid_imm_id(id) && "Invalid immediate id");
	if (!is_valid_imm_id(id)) {
		return 0;
	}
	
	int index = id - IMMUP0;

	return g_up_imm[index].val.box.y;
}

/* Function: im_create_imm_op_by_id
*------------------------------------------------------------------------------
*  creates an operand of the selected immediate
*
* Arguments:
*	Void.
*	
* Return:
*	The pointer to the created imm operand
*
*/
operand_t *im_create_imm_op_by_id(int op_id)
{
	assert(is_valid_imm_id(op_id) && "Invalid immediate id");
	if (!is_valid_imm_id(op_id)) {
		return NULL;
	}
	operand_t *o = NULL;
	for (int i = IMMUP0; i < IMM_MAX; i++){
		if (i == op_id){
			char *num;
			if (op_id < IMMUP_1){
	   			num = ax_number_to_string(i - IMMUP0);
			} else if (op_id > IMMUP10){
	   			num = ax_number_to_string(-(i - IMMUP10));
			}	
			texture_t *t = dw_create_text_tex(num, C_WHITE);
			free(num);
			SDL_Rect cb = cl_get_code_button_size();
			SDL_Rect r = {.x = 0, .y = 0, .w = cb.w, .h = cb.h};
			btn_t *b = bt_create_btn(r, t);
			o = malloc(sizeof(operand_t));
			o->b = b;
			o->id = op_id; 
			break;
		} 
	}
			
   return o;
}

/* Function: rg_get_imm_value_box_by_id
*------------------------------------------------------------------------------
* Arguments:
*	id: The id of the value box that will be retrieved	
*
* Return:
*	The value box
*/
vb_value_box_t im_get_imm_value_box_by_id(int id)
{
	assert(is_valid_imm_id(id) && "Invalid immediate id");
	if (!is_valid_imm_id(id)) {
		return (vb_value_box_t){0};
	}
	int index = id - IMMUP0;

	vb_value_box_t val = g_up_imm[index].val;

   return val;
}


/* Function: im_create_sel_imm_op
*------------------------------------------------------------------------------
*  creates an operand of the selected immediate
*
* Arguments:
*	Void.
*	
* Return:
*	The pointer to the created imm operand
*
*/
operand_t *im_create_sel_imm_op(void)
{
	operand_t *o = NULL;
	int imm_id = IMMUP0;
	for (int i = 0; i < TOTAL_IMM; i++){
		if (bt_chk_rel_btn(g_up_imm[i].b, NULL) == true){
	   		char *num = ax_number_to_string(g_up_imm[i].val.value);
			texture_t *t = dw_create_text_tex(num, C_WHITE);
			free(num);
			SDL_Rect cb = cl_get_code_button_size();
			SDL_Rect r = {.x = 0, .y = 0, .w = cb.w, .h = cb.h};
			btn_t *b = bt_create_btn(r, t);
			o = malloc(sizeof(operand_t));
			o->b = b;
			o->id = imm_id; 
		} 
		imm_id++;
	}
			
   return o;
}


/* Function: sb_set_imm_up_avail
 * ----------------------------------------------------------------------------
 * Sets the global variable that determines if the up immediates will be 
 * available
 *
 * Arguments:
 * 	state: state that the g_step_btns_avail variable will be set
 *
 * Return:
 *	Void.	
 */
void im_set_imm_up_avail(bool state)
{
	g_imm_up = state;
}

bool im_are_imm_up_available(void)
{
	return g_imm_up;
}

void im_set_highlight_source(ui_highlight_source_t source, bool enabled)
{
	g_imm_highlight_sources = ui_highlight_source_set_enabled(
	    g_imm_highlight_sources, source, enabled);
	g_imm_highlight = g_imm_highlight_sources != 0;
}

bool im_are_immediates_highlighted(void)
{
	return g_imm_highlight;
}

/* Function: im_init_imm_assets
 *------------------------------------------------------------------------------
 * Initializes the assets of the immediates of the level
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	Void.
 */
void im_init_imm_assets(void)
{
	if (g_imm_assets_initialized) {
		return;
	}
	init_imm_texture();
	for (int i = 0; i < TOTAL_IMM; i++){
		int value = i < IMM_FIRST_ROW_COUNT ? i :
		            -(i - IMM_FIRST_ROW_COUNT + 1);
		g_up_imm[i].val.visible_box = true;
		g_up_imm[i].val.value = value;
		char *num = ax_number_to_string(value);
		g_up_imm[i].val.t = dw_create_text_tex(num, C_WHITE);
		free(num);
		g_up_imm[i].b = calloc(1, sizeof(btn_t));
		assert(g_up_imm[i].b != NULL);
		g_up_imm[i].b->enabled = true;
	}
	g_imm_assets_initialized = true;
	im_layout_grid();
}

void im_layout_grid(void)
{
	assert(g_imm_assets_initialized && "Immediate assets must be initialized");
	if (!g_imm_assets_initialized) {
		return;
	}
	SDL_Rect grid = im_get_grid_bounds();
	assert(grid.w > 0 && grid.h > 0);
	for (int index = 0; index < TOTAL_IMM; index++) {
		SDL_Rect bounds = get_cell_bounds_by_index(index);
		assert(g_up_imm[index].b != NULL);
		if (g_up_imm[index].b == NULL) {
			return;
		}
		g_up_imm[index].val.box = bounds;
		g_up_imm[index].b->r = bounds;
	}
	for (int index = 0; index < TOTAL_IMM; index++) {
		g_up_imm[index].borders = get_imm_cell_borders(index);
	}
}

void im_destroy_imm_assets(void)
{
	for (int index = 0; index < TOTAL_IMM; index++) {
		dw_free_texture(g_up_imm[index].val.t);
		g_up_imm[index].val.t = NULL;
		free(g_up_imm[index].b);
		g_up_imm[index].b = NULL;
	}
	dw_free_texture(g_imm_txt);
	g_imm_txt = NULL;
	g_imm_assets_initialized = false;
}

/* Function: im_click_up_imm
 *------------------------------------------------------------------------------
 * Verifies if one of the upper immediates was clicked
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	True if any of the upper immediates was clicked.
 */
bool im_ms_rel_in_upimm(void)
{
	if (!g_imm_assets_initialized) {
		return false;
	}
 	bool rel = false;

	for (int i = 0; i < TOTAL_IMM; i++){

		if (bt_chk_rel_btn(g_up_imm[i].b, NULL) == true){
		   	rel = true;
		   	break;
	   	} 
	}
	return rel;
}

/* Function: init_imm_texture
 *------------------------------------------------------------------------------
 * Creates the instructions texture of the immediate text
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	Void.
 */
static void init_imm_texture()
{
	g_imm_txt = dw_create_text_tex(IMM_TXT, C_AMBER);
}


/* Function: im_draw_up_imm
 * -----------------------------------------------------------------------------
 * Draws the upper immediates
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	Void.
 */
void im_draw_imm(float animation_limit)
{
	bool highlighted = g_imm_up && g_imm_highlight;
	if (highlighted != g_imm_highlight_rendered) {
		dw_pulse_reset(&g_imm_pulse);
		g_imm_highlight_rendered = highlighted;
	}
	if (g_imm_up == true){
		float pulse = highlighted ?
			dw_pulse_value(&g_imm_pulse, animation_limit) : 0.0f;
		draw_imm_txt_up(pulse);
		for (int i = 0; i < TOTAL_IMM; i++){
			draw_imm_cell(&g_up_imm[i], pulse, highlighted);
		}
		if (highlighted) {
			dw_pulse_advance(&g_imm_pulse, animation_limit);
		}
	}	
	

}

/* Function: draw_imm_txt_up
 *------------------------------------------------------------------------------
 * Draws the immediates of the upper section of the screen 
 *
 * Arguments:
 *	Void.
 * 
 * Return:
 *	Void.
 *
 */
static void draw_imm_txt_up(float pulse)
{
	SDL_Rect label_bounds = im_get_label_bounds();

	if (IMM_LABEL_HIGHLIGHT_ENABLED && g_imm_highlight) {
		SDL_FRect highlight_bounds = {
			.x = label_bounds.x,
			.y = label_bounds.y,
			.w = label_bounds.w,
			.h = label_bounds.h
		};
		highlight_bounds = dw_grow_rect_height(
		    highlight_bounds, pulse * DW_TEXT_HIGHLIGHT_GROWTH_FACTOR);
		dw_set_texture_color_mod(g_imm_txt, C_LIGHTGREY);
		dw_draw_texture_fit_h_f(highlight_bounds, g_imm_txt);
		dw_set_texture_color_mod(g_imm_txt, C_AMBER);
	} else {
		dw_draw_texture_fit_h(label_bounds, g_imm_txt);
	}
}

static void draw_imm_cell(const imm_t *imm, float pulse, bool highlighted)
{
	draw_imm_cell_frame(imm);
	draw_imm_value(&imm->val, pulse, highlighted);
}

static void draw_imm_cell_frame(const imm_t *imm)
{
	const SDL_Rect box = imm->val.box;
	int border = dm_scale_to_res(IMM_CELL_BORDER_WIDTH);
	int shared_leading_border = border / 2;
	int shared_trailing_border = border - shared_leading_border;
	SDL_Rect inner = box;
	int left_border = imm->borders.left ? shared_leading_border : border;
	int right_border = imm->borders.right ? shared_trailing_border : border;
	int top_border = imm->borders.top ? shared_leading_border : border;
	int bottom_border = imm->borders.bottom ? shared_trailing_border : border;

	dw_draw_filled_rectangle(box, C_GREY, C_GREY);
	inner.x += left_border;
	inner.y += top_border;
	inner.w -= left_border + right_border;
	inner.h -= top_border + bottom_border;
	dw_draw_filled_rectangle(inner, C_BLACK, C_BLACK);
	}

static void draw_imm_value(const vb_value_box_t *value, float pulse,
						   bool highlighted)
{
	if (value->t == NULL) {
		return;
	}
	int text_h = vb_get_text_size().h;
	int text_w = ax_get_texture_w_fit_h(text_h, value->t);
	SDL_Rect text = {
		.x = value->box.x + (value->box.w - text_w) / 2,
		.y = value->box.y + (value->box.h - text_h) / 2,
		.h = text_h
	};
	if (!highlighted) {
		dw_draw_texture_fit_h(text, value->t);
		return;
	}

	SDL_FRect animated_text = {
		.x = text.x,
		.y = text.y,
		.w = text_w,
		.h = text_h
	};
	animated_text = dw_grow_rect_height(
	    animated_text, pulse * DW_TEXT_HIGHLIGHT_GROWTH_FACTOR);
	dw_set_texture_color_mod(value->t, C_LIGHTGREY);
	dw_draw_texture_fit_h_f(animated_text, value->t);
	dw_set_texture_color_mod(value->t, C_WHITE);
}

static imm_cell_borders_t get_imm_cell_borders(int index)
{
	SDL_Rect cell = g_up_imm[index].val.box;
	imm_cell_borders_t borders = {0};
	for (int other_index = 0; other_index < TOTAL_IMM; other_index++) {
		if (other_index == index) {
			continue;
		}
		SDL_Rect other = g_up_imm[other_index].val.box;
		if (other.x + other.w == cell.x &&
		    rects_overlap_vertically(other, cell)) {
			borders.left = true;
		}
		if (cell.x + cell.w == other.x &&
		    rects_overlap_vertically(other, cell)) {
			borders.right = true;
		}
		if (other.y + other.h == cell.y &&
		    rects_overlap_horizontally(other, cell)) {
			borders.top = true;
		}
		if (cell.y + cell.h == other.y &&
		    rects_overlap_horizontally(other, cell)) {
			borders.bottom = true;
		}
	}
	return borders;
}

static bool rects_overlap_vertically(SDL_Rect first, SDL_Rect second)
{
	return first.y < second.y + second.h && second.y < first.y + first.h;
}

static bool rects_overlap_horizontally(SDL_Rect first, SDL_Rect second)
{
	return first.x < second.x + second.w && second.x < first.x + first.w;
}





























