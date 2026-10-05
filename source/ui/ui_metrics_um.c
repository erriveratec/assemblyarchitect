#include "dimensions_dm.h"
#include "ui/ui_metrics_um.h"

static const int BORDER_WIDTH = 5;
static const int HORIZONTAL_PADDING = 12;
static const int VERTICAL_PADDING = 10;
static const int STAGE_LABEL_HEIGHT = 25;
static const int MESSAGE_TEXT_HEIGHT = 35;
static const int EMPHASIS_TEXT_HEIGHT = 50;
static const int CODE_TEXT_HEIGHT = 40;
static const int BUTTON_ANIMATION_MAX = 15;
static const int BUTTON_ANIMATION_DELTA = 1;

int um_border_width(void)
{
	return dm_scale_to_res(BORDER_WIDTH);
}

int um_padding_horizontal(void)
{
	return dm_scale_to_res(HORIZONTAL_PADDING);
}

int um_padding_vertical(void)
{
	return dm_scale_to_res(VERTICAL_PADDING);
}

int um_padding_horizontal_with_border(void)
{
	return um_padding_horizontal() + um_border_width();
}

int um_padding_vertical_with_border(void)
{
	return um_padding_vertical() + um_border_width();
}

int um_stage_label_height(void)
{
	return dm_scale_to_res(STAGE_LABEL_HEIGHT);
}

int um_message_text_height(void)
{
	return dm_scale_to_res(MESSAGE_TEXT_HEIGHT);
}

int um_emphasis_text_height(void)
{
	return dm_scale_to_res(EMPHASIS_TEXT_HEIGHT);
}

int um_code_text_height(void)
{
	return dm_scale_to_res(CODE_TEXT_HEIGHT);
}

int um_button_animation_max(void)
{
	return dm_scale_to_res(BUTTON_ANIMATION_MAX);
}

int um_button_animation_delta(void)
{
	return dm_scale_to_res(BUTTON_ANIMATION_DELTA);
}
