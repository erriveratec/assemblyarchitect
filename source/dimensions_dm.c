#include "dimensions_dm.h"

static dm_resolution_t g_resolution = DM_RESOLUTION_1600X900;
static int g_screen_width = 1600;
static int g_screen_height = 900;

void dm_set_screen_resolution(dm_resolution_t resolution)
{
	switch (resolution) {
	case DM_RESOLUTION_1920X1080:
		g_screen_width = 1920;
		g_screen_height = 1080;
		g_resolution = DM_RESOLUTION_1920X1080;
		break;
	case DM_RESOLUTION_1600X900:
	default:
		g_screen_width = 1600;
		g_screen_height = 900;
		g_resolution = DM_RESOLUTION_1600X900;
		break;
	}
}

void dm_set_screen_dimensions(int width, int height)
{
	if (width <= 0 || height <= 0) {
		return;
	}
	g_screen_width = width;
	g_screen_height = height;
}

int dm_get_screen_width(void)
{
	return g_screen_width;
}

int dm_get_screen_height(void)
{
	return g_screen_height;
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
