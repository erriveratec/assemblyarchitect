#ifndef DIMENSIONS_DM_H
#define DIMENSIONS_DM_H

enum resolutions {
	R1920X1080,
	R1600X900
};

void dm_set_screen_resolution(int resolution_id);
void dm_set_screen_dimensions(int width, int height);
int dm_get_screen_width(void);
int dm_get_screen_height(void);
int dm_scale_to_res(int dimension);
#endif
