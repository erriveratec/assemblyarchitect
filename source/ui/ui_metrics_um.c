#include "ui/ui_metrics_um.h"

#include "dimensions_dm.h"

static const int BORDER_WIDTH = 5;
static const int HORIZONTAL_PADDING = 12;
static const int VERTICAL_PADDING = 10;

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
