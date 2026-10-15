#include <assert.h>

#define cw_get_code_list_size test_code_list_size
#define cw_get_instruction_y_coord test_instruction_y
#define cw_get_stage_code_box test_stage_code_box
#include "../source/arrow_ar.c"
#undef cw_get_code_list_size
#undef cw_get_instruction_y_coord
#undef cw_get_stage_code_box

int test_code_list_size(void)
{
	return 4;
}

int test_instruction_y(int position)
{
	assert(position >= 0 && position < test_code_list_size());
	return dm_scale_to_res(100) + position * cl_get_code_button_size().h;
}

SDL_Rect test_stage_code_box(void)
{
	return (SDL_Rect){.x = dm_scale_to_res(200)};
}

static void assert_centered(int position)
{
	SDL_Rect button = cl_get_code_button_size();
	SDL_Rect arrow = ar_get_arrow_wh();
	int instruction_y = test_instruction_y(position);
	int height_difference = button.h - arrow.h;
	int arrow_center = 2 * g_arrow_exec.box.y + arrow.h;
	int instruction_center = 2 * instruction_y + button.h;

	assert(g_arrow_exec.box.y == instruction_y + height_difference / 2);
	assert(g_arrow_exec.box.w == arrow.w);
	assert(g_arrow_exec.box.h == arrow.h);
	assert(arrow_center == instruction_center - height_difference % 2);
	if (height_difference % 2 == 0) {
		assert(arrow_center == instruction_center);
	}
}

static void move_to_instruction(int position, bool downward)
{
	int initial_y = g_arrow_exec.box.y;
	int target_y = test_instruction_y(position)
		+ (cl_get_code_button_size().h - ar_get_arrow_wh().h) / 2;
	int max_steps = abs(target_y - initial_y) + 1;
	bool arrived = false;

	for (int step = 0; step < max_steps && !arrived; step++) {
		int previous_y = g_arrow_exec.box.y;
		arrived = ar_move_execution_arrow(position);
		assert(arrived == (g_arrow_exec.box.y == target_y));
		if (downward) {
			assert(g_arrow_exec.box.y > previous_y);
			assert(g_arrow_exec.box.y <= target_y);
		} else {
			assert(g_arrow_exec.box.y < previous_y);
			assert(g_arrow_exec.box.y >= target_y);
		}
		if (step == 0) {
			assert(!arrived);
		}
	}
	assert(arrived);
	assert_centered(position);
	assert(ar_move_execution_arrow(position));
	assert_centered(position);
}

static void test_resolution(dm_resolution_t resolution, bool fast)
{
	dm_set_screen_resolution(resolution);
	ax_set_arrow_mdelta(fast);
	ar_reset_execution_arrow();
	assert_centered(0);
	assert(g_arrow_exec.box.x == test_stage_code_box().x - ar_get_arrow_wh().w);
	assert(g_arrow_exec.startx == g_arrow_exec.box.x);
	assert(g_arrow_exec.starty == g_arrow_exec.box.y);
	assert(g_arrow_exec.visible && g_arrow_exec.in_place);
	assert(g_arrow_exec.texture == g_exec_arrow);
	assert(g_arrow_exec.dir == AR_RIGHT && g_arrow_exec.travel == 0);
	assert(ar_move_execution_arrow(0));
	move_to_instruction(3, true);
	move_to_instruction(0, false);
	move_to_instruction(3, true);
	ar_hide_execution_arrow();
	ar_reset_execution_arrow();
	assert_centered(0);
	assert(g_arrow_exec.starty == g_arrow_exec.box.y);
	assert(g_arrow_exec.visible);
}

int main(void)
{
	test_resolution(DM_RESOLUTION_1600X900, false);
	test_resolution(DM_RESOLUTION_1600X900, true);
	test_resolution(DM_RESOLUTION_1920X1080, false);
	test_resolution(DM_RESOLUTION_1920X1080, true);
	return 0;
}