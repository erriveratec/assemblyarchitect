#include <assert.h>
#include <math.h>

#define vb_draw test_vb_draw
#define vb_draw_style test_vb_draw_style
#define dw_draw_texture_fit_h test_draw_texture_fit_h
#define dw_draw_texture_fit_h_style test_draw_texture_fit_h_style
#define dw_draw_texture_fit_h_f_style test_draw_texture_fit_h_f_style
#define dw_draw_filled_rectangle_f test_draw_filled_rectangle_f
/* This narrow private-helper test follows the existing geometry test pattern. */
#include "../source/registers_rg.c"
#undef dw_draw_filled_rectangle_f
#undef dw_draw_texture_fit_h_f_style
#undef dw_draw_texture_fit_h_style
#undef dw_draw_texture_fit_h
#undef vb_draw_style
#undef vb_draw

static int g_value_box_draw_count;
static int g_static_label_draw_count;
static int g_animated_label_draw_count;
static int g_highlight_rectangle_draw_count;
static int g_white_style_draw_count;
static SDL_Rect g_static_label_rect;
static SDL_FRect g_animated_label_rect;
static dw_render_style_t g_value_box_style;

void test_vb_draw(vb_value_box_t *box, SDL_Color color)
{
	assert(box == &g_zf);
	assert(color.r == C_GREY.r && color.g == C_GREY.g &&
	       color.b == C_GREY.b && color.a == C_GREY.a);
	g_value_box_draw_count++;
}

void test_vb_draw_style(vb_value_box_t *box, SDL_Color color,
						dw_render_style_t style)
{
	assert(box == &g_zf);
	assert(color.r == C_GREY.r || color.r == C_WHITE.r);
	g_value_box_style = style;
	g_value_box_draw_count++;
}

int test_draw_texture_fit_h_style(SDL_Rect rect, texture_t *texture,
								dw_render_style_t style)
{
	assert(texture == g_zf_text);
	assert(style == DW_RENDER_STYLE_NORMAL);
	g_static_label_rect = rect;
	g_static_label_draw_count++;
	return SUCCESS;
}

int test_draw_texture_fit_h(SDL_Rect rect, texture_t *texture)
{
	assert(texture == g_zf_text);
	g_static_label_rect = rect;
	g_static_label_draw_count++;
	return SUCCESS;
}

int test_draw_texture_fit_h_f_style(SDL_FRect rect, texture_t *texture,
								  dw_render_style_t style)
{
	assert(texture == g_zf_text);
	assert(style == DW_RENDER_STYLE_HIGHLIGHT_WHITE);
	g_animated_label_rect = rect;
	g_animated_label_draw_count++;
	g_white_style_draw_count++;
	return SUCCESS;
}

void test_draw_filled_rectangle_f(SDL_FRect rect, SDL_Color in,
								 SDL_Color out)
{
	(void)rect;
	(void)in;
	(void)out;
	g_highlight_rectangle_draw_count++;
}

static void assert_close(float actual, float expected)
{
	assert(fabsf(actual - expected) < 0.0001f);
}

static void assert_same_size_and_center(SDL_FRect actual, SDL_Rect original,
										float growth)
{
	assert_close(actual.w, original.w + growth * original.w / original.h);
	assert_close(actual.h, original.h + growth);
	assert_close(actual.x + actual.w / 2.0f,
	             original.x + original.w / 2.0f);
	assert_close(actual.y + actual.h / 2.0f,
	             original.y + original.h / 2.0f);
}

int main(void)
{
	texture_t label_texture = {.w = 1, .h = 2};
	SDL_Rect zf_label_region = {.x = 420, .y = 320, .w = 50, .h = 40};
	SDL_Rect register_label_region = {.x = 130, .y = 560, .w = 50, .h = 40};
	static const float pulse_samples[] = {0.0f, 3.75f};
	float maximum_pulse = (float)dw_get_animation_limit();

	for (size_t index = 0;
	     index <= sizeof(pulse_samples) / sizeof(pulse_samples[0]); index++) {
		float shared_pulse = index < sizeof(pulse_samples) / sizeof(pulse_samples[0])
			? pulse_samples[index] : maximum_pulse;
		SDL_FRect zf_label = get_highlighted_label_rect(
			zf_label_region, &label_texture, shared_pulse);
		SDL_FRect register_label = get_highlighted_label_rect(
			register_label_region, &label_texture, shared_pulse);
		float label_growth = shared_pulse * DW_TEXT_HIGHLIGHT_GROWTH_FACTOR;
		assert_same_size_and_center(zf_label,
		                            (SDL_Rect){.x = 435, .y = 320, .w = 20, .h = 40},
		                            label_growth);
		assert_same_size_and_center(register_label,
		                            (SDL_Rect){.x = 145, .y = 560, .w = 20, .h = 40},
		                            label_growth);
		assert_close(zf_label.w, register_label.w);
		assert_close(zf_label.h, register_label.h);
	}

	assert(zf_label_region.x == 420 && zf_label_region.y == 320);
	assert(zf_label_region.w == 50 && zf_label_region.h == 40);
	assert(register_label_region.x == 130 && register_label_region.y == 560);
	assert(register_label_region.w == 50 && register_label_region.h == 40);

	texture_t zf_label_texture = {.w = 1, .h = 2};
	g_zf = (vb_value_box_t){
		.box = {.x = 420, .y = 270, .w = 50, .h = 40},
		.value = 1
	};
	g_zf_text = &zf_label_texture;
	SDL_Rect original_box = g_zf.box;
	SDL_Rect base_label_region = {
		.x = original_box.x,
		.y = original_box.y + cl_get_code_button_size().h,
		.w = vb_get_size().w,
		.h = cl_get_code_button_size().h
	};
	int base_label_width = ax_get_texture_w_fit_h(base_label_region.h,
	                                              g_zf_text);
	rg_set_zero_flag_highlight_source(UI_HIGHLIGHT_SOURCE_TUTORIAL, false);
	rg_draw_flag_boxes(maximum_pulse);
	assert(g_value_box_draw_count == 1);
	assert(g_value_box_style == DW_RENDER_STYLE_NORMAL);
	assert(g_static_label_draw_count == 1);
	assert(g_animated_label_draw_count == 0);
	assert(g_static_label_rect.x ==
	       base_label_region.x + (base_label_region.w - base_label_width) / 2);
	assert(g_static_label_rect.y == base_label_region.y);
	assert(g_static_label_rect.w == base_label_width);
	assert(g_static_label_rect.h == base_label_region.h);
	assert(g_zf.box.x == original_box.x && g_zf.box.y == original_box.y &&
	       g_zf.box.w == original_box.w && g_zf.box.h == original_box.h);

	g_value_box_draw_count = 0;
	g_static_label_draw_count = 0;
	ha_reset();
	ha_begin_frame();
	ha_begin_frame();
	rg_set_zero_flag_highlight_source(UI_HIGHLIGHT_SOURCE_TUTORIAL, true);
	float active_pulse = ha_get_value(maximum_pulse);
	rg_draw_flag_boxes(maximum_pulse);
	assert(g_value_box_draw_count == 1);
	assert(g_static_label_draw_count == 0);
	assert(g_animated_label_draw_count == 1);
	assert(g_highlight_rectangle_draw_count == 0);
	assert(g_value_box_style == DW_RENDER_STYLE_HIGHLIGHT_WHITE);
	assert(g_white_style_draw_count == 1);
	SDL_FRect expected_label = get_highlighted_label_rect(
		base_label_region, g_zf_text, active_pulse);
	assert_close(g_animated_label_rect.x, expected_label.x);
	assert_close(g_animated_label_rect.y, expected_label.y);
	assert_close(g_animated_label_rect.h, expected_label.h);
	assert(g_zf.box.x == original_box.x && g_zf.box.y == original_box.y &&
	       g_zf.box.w == original_box.w && g_zf.box.h == original_box.h);
	assert(g_zf.value == 1);
	rg_set_zero_flag_highlight_source(UI_HIGHLIGHT_SOURCE_TUTORIAL, false);
	return 0;
}