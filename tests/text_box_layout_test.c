#include <assert.h>

#include "../source/text_tx.c"
#include "../source/ui/run_result_rr.c"

static const int LARGE_FORMAT_MIN_WIDTH = 1920;

static void test_upper_right_layout(bool large_box)
{
	tx_text_box_options_t options = {
		.position = TX_UPPER_RIGHT_BOX,
		.large_box = large_box,
		.large_text = false
	};
	SDL_Rect box;
	assert(tx_get_text_box_rects(&options, &box, NULL, NULL));
	assert(box.w > 0 && box.h > 0);
	assert(box.x >= 0 && box.y >= 0);
	assert(box.x + box.w <= dm_get_screen_width());
	assert(box.y + box.h <= dm_get_screen_height());
	assert(dm_get_screen_width() - box.x - box.w == get_border_ofs());
	assert(box.y == get_border_ofs());
	SDL_Rect expected_size = tx_get_text_box_wh();
	if (large_box) {
		expected_size.w = dm_scale_to_res(LARGE_TEXT_BOX_W);
		expected_size.h = dm_scale_to_res(LARGE_TEXT_BOX_H);
	}
	assert(box.w == expected_size.w && box.h == expected_size.h);

	assert(g_run_result_layout_config.size == RUN_RESULT_BOX_LARGE);
	assert(g_run_result_layout_config.position == TX_CENTER_BOX);
	for (int success = 0; success <= 1; success++) {
		run_result_layout_t result;
		assert(rr_get_layout_for_size(RUN_RESULT_BOX_LARGE, success,
		                              &result, NULL));
		/* Two large boxes cannot fit side by side with a centered result
		 * at 1600x900 under the current sizing policy. */
		if (!large_box || dm_get_screen_width() >= LARGE_FORMAT_MIN_WIDTH) {
			assert(!rr_rects_overlap(box, result.box));
		}
	}
}

static void test_screen_layout(void)
{
	test_upper_right_layout(false);
	test_upper_right_layout(true);
}

int main(void)
{
	dm_set_screen_resolution(DM_RESOLUTION_1600X900);
	test_screen_layout();
	dm_set_screen_resolution(DM_RESOLUTION_1920X1080);
	test_screen_layout();
	dm_set_screen_dimensions(1920, 1200);
	test_screen_layout();
	return 0;
}