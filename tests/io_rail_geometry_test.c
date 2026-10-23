#include <assert.h>
#include <SDL_ttf.h>
#include "sdl_config.h"
#include "immediates_im.h"

static vb_value_box_t shifted_immediate_value_box(int id);
static void capture_rectangle(SDL_Rect rect, SDL_Color inner, SDL_Color outer);
#define im_get_imm_value_box_by_id shifted_immediate_value_box
#define dw_draw_filled_rectangle capture_rectangle
#include "../source/game_mechanics_mc.c"
#undef dw_draw_filled_rectangle
#undef im_get_imm_value_box_by_id

static SDL_Rect rectangles[5];
static int rectangle_count;
static int immediate_shift_x;

static vb_value_box_t shifted_immediate_value_box(int id)
{
	vb_value_box_t value = im_get_imm_value_box_by_id(id);
	value.box.x += immediate_shift_x;
	return value;
}

static void reach_operand(avatar_t *avatar, int op_id)
{
	const SDL_Rect avatar_size = get_avatar_size();
	const SDL_Rect value_size = vb_get_size();
	int expected_x;
	int expected_value_y = get_operand_y_dest(op_id);
	if (op_id > IMM_MIN && op_id < IMM_MAX) {
		expected_x = im_get_imm_value_box_x_coord_by_id(op_id) + immediate_shift_x;
	} else if (op_id == IB || op_id == OB) {
		expected_x = bf_get_buffer_value_box_x_coord_by_id(op_id);
		if (op_id == IB) {
			expected_value_y += 3 * value_size.h / 2;
		} else {
			expected_value_y -= 2 * value_size.h + avatar_size.h;
		}
	} else {
		assert(op_id == IBOX || op_id == OBOX);
		expected_x = rg_get_ibox_x();
		if (op_id == IBOX) {
			expected_value_y -= 5 * value_size.h / 2 + vb_get_vertical_offset();
		}
	}
	assert(get_avatar_operand_x_dest(op_id) == expected_x);
	avatar->box = avatar_size;
	avatar->box.x = expected_x - avatar_size.w;
	avatar->box.y = expected_value_y + avatar_size.h;
	avatar->mainval.box.x = avatar->box.x;
	avatar->mainval.box.y = expected_value_y;
	bool moving = true;
	for (int step = 0; step < 1000 && moving; step++) {
		moving = move_avatar_to_operand(avatar, op_id);
	}
	assert(!moving);
	assert(avatar->box.x == expected_x);
	assert(avatar->mainval.box.x == expected_x);
	assert(avatar->box.y == expected_value_y + avatar_size.h);
	assert(avatar->mainval.box.y == expected_value_y);
}

static void capture_rectangle(SDL_Rect rect, SDL_Color inner, SDL_Color outer)
{
	(void)inner;
	(void)outer;
	assert(rectangle_count < 5);
	rectangles[rectangle_count++] = rect;
}

static void test_io_rail(bool input)
{
	avatar_t *avatar = input ? &g_iavatar : &g_oavatar;
	avatar->id = input ? IAVATAR : OAVATAR;
	const int operands[] = {IMMUP0, IMMUP10, IMMUP_1, IB, OB,
		input ? IBOX : OBOX};
	for (size_t index = 0; index < sizeof(operands) / sizeof(operands[0]); index++) {
		reach_operand(avatar, operands[index]);
	}
	reach_operand(avatar, IMMUP0);
	const int connector_center_x = avatar->box.x + avatar->box.w / 2;
	const int rail_width = dm_scale_to_res(STAGE_RAIL_WIDTH);
	const SDL_Rect buffer = input ? bf_get_input_buffer_bounds() :
		bf_get_output_buffer_bounds();
	const SDL_Rect value = input ? rg_get_input_value_box_bounds() :
		rg_get_output_value_box_bounds();
	const int right_center_x = buffer.x + bf_get_value_box_spacing() + value.w / 2;
	const horizontal_rail_geometry_t rail =
		get_io_rail_geometry(avatar, rail_width, right_center_x);
	assert(rail.x + rail_width / 2 == connector_center_x);
	assert(rail.width > 0);
	assert(rail.width == right_center_x - connector_center_x + rail_width);
	const int old_left_x = value.x + value.w / 2 - rail_width / 2;
	const int old_width = buffer.x + value.w / 2 - old_left_x +
		bf_get_value_box_spacing() + rail_width / 2;
	assert(rail.x + rail.width == old_left_x + old_width);

	for (int visible = 0; visible < 2; visible++) {
		im_set_imm_up_avail(visible != 0);
		rectangle_count = 0;
		if (input) {
			draw_iavatar();
		} else {
			draw_oavatar();
		}
		assert(rectangle_count == 5);
		assert(rectangles[1].x == rail.x);
		assert(rectangles[1].w == rail.width);
		assert(rectangles[1].h == rail_width);
		assert(rectangles[1].x + rail_width / 2 == connector_center_x);
		const int left_end_center_x = rectangles[2].x + rectangles[2].w / 2;
		assert(left_end_center_x == connector_center_x);
		assert(rectangles[4].x + rectangles[4].w / 2 == left_end_center_x);
		const SDL_Rect registers = rg_get_register_box();
		const int medium = input ? (registers.y - buffer.y - buffer.h) / 2 :
			(buffer.y - registers.y - registers.h) / 2;
		const int expected_y = (input ? registers.y : buffer.y) - medium - rail_width / 2;
		assert(rectangles[1].y == expected_y);
		const int end_width = dm_scale_to_res(STAGE_RAIL_END_WIDTH);
		assert(rectangles[2].x == rail.x - (end_width - rail_width) / 2);
		assert(rectangles[3].x == old_left_x + old_width -
			(end_width - rail_width) + rail_width / 2);
		assert(rectangles[2].w == end_width && rectangles[3].w == end_width);
	}
}

static void test_resolution(dm_resolution_t resolution)
{
	dm_set_screen_resolution(resolution);
	ax_set_fast_move_delta(false);
	im_layout_grid();
	assert(im_get_imm_value_box_by_id(IMMUP0).box.x == im_get_grid_bounds().x);
	test_io_rail(true);
	test_io_rail(false);
	const int original_left_end_center_x = rectangles[2].x + rectangles[2].w / 2;
	const int original_connector_center_x = g_oavatar.box.x + g_oavatar.box.w / 2;
	immediate_shift_x = im_get_cell_size().w;
	test_io_rail(true);
	test_io_rail(false);
	const int shifted_left_end_center_x = rectangles[2].x + rectangles[2].w / 2;
	const int shifted_connector_center_x = g_oavatar.box.x + g_oavatar.box.w / 2;
	assert(shifted_connector_center_x - original_connector_center_x == immediate_shift_x);
	assert(shifted_left_end_center_x - original_left_end_center_x ==
		shifted_connector_center_x - original_connector_center_x);
	immediate_shift_x = 0;
}

int main(void)
{
	assert(SDL_Init(0) == 0);
	assert(TTF_Init() == 0);
	g_screen = SDL_CreateRGBSurfaceWithFormat(0, 1920, 1080, 32,
		SDL_PIXELFORMAT_RGBA32);
	assert(g_screen != NULL);
	g_renderer = SDL_CreateSoftwareRenderer(g_screen);
	assert(g_renderer != NULL);
	g_font = TTF_OpenFont("fonts/DOSVGA437.ttf", 130);
	assert(g_font != NULL);
	im_init_imm_assets();
	assert(get_operand_x_dest(INVALID_OPERAND) == NO_VALUE);
	assert(get_operand_y_dest(INVALID_OPERAND) == NO_VALUE);
	avatar_t invalid_route = {.id = IAVATAR, .box = {10, 20, 30, 40}};
	SDL_Rect original = invalid_route.box;
	assert(!move_avatar_to_operand(&invalid_route, OBOX));
	assert(SDL_RectEquals(&invalid_route.box, &original));
	test_resolution(DM_RESOLUTION_1600X900);
	test_resolution(DM_RESOLUTION_1920X1080);
	test_resolution(DM_RESOLUTION_1600X900);
	im_destroy_imm_assets();
	TTF_CloseFont(g_font);
	SDL_DestroyRenderer(g_renderer);
	SDL_FreeSurface(g_screen);
	TTF_Quit();
	SDL_Quit();
	return 0;
}