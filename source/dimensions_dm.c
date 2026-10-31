#include "dimensions_dm.h"

static dm_resolution_t g_resolution = DM_RESOLUTION_1600X900;
static int g_logical_width = 1600;
static int g_logical_height = 900;

void dm_set_screen_resolution(dm_resolution_t resolution)
{
	switch (resolution) {
	case DM_RESOLUTION_1920X1080:
		g_logical_width = 1920;
		g_logical_height = 1080;
		g_resolution = DM_RESOLUTION_1920X1080;
		break;
	case DM_RESOLUTION_1600X900:
	default:
		g_logical_width = 1600;
		g_logical_height = 900;
		g_resolution = DM_RESOLUTION_1600X900;
		break;
	}
}

void dm_set_logical_dimensions(int width, int height)
{
	if (width <= 0 || height <= 0) {
		return;
	}
	g_logical_width = width;
	g_logical_height = height;
}

void dm_set_screen_dimensions(int width, int height)
{
	dm_set_logical_dimensions(width, height);
}

int dm_get_screen_width(void)
{
	return g_logical_width;
}

int dm_get_screen_height(void)
{
	return g_logical_height;
}

int dm_scale_to_res(int dimension)
{
	/* Both supported resolution IDs currently use an identity scale. */
	switch (g_resolution) {
	case DM_RESOLUTION_1920X1080:
	case DM_RESOLUTION_1600X900:
	default:
		return dimension;
	}
}
