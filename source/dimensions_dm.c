#include "dimensions_dm.h"

static int g_res_id;
static int g_screen_width;
static int g_screen_height;

void dm_set_screen_resolution(int resolution_id)
{
	switch (resolution_id) {
	case R1920X1080:
		g_screen_width = 1920;
		g_screen_height = 1080;
		g_res_id = R1920X1080;
		break;
	case R1600X900:
	default:
		g_screen_width = 1600;
		g_screen_height = 900;
		g_res_id = R1600X900;
		break;
	}
}

void dm_set_screen_dimensions(int width, int height)
{
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
	switch (g_res_id) {
	case R1920X1080:
	case R1600X900:
	default:
		return dimension;
	}
}
