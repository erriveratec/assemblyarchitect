#include "dimensions_dm.h"

int dm_get_screen_width(void)
{
	return DM_LOGICAL_CANVAS_WIDTH;
}

int dm_get_screen_height(void)
{
	return DM_LOGICAL_CANVAS_HEIGHT;
}

int dm_scale_to_res(int dimension)
{
	return dimension;
}
