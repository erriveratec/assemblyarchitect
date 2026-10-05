#ifndef DIMENSIONS_DM_H
#define DIMENSIONS_DM_H

typedef enum dm_resolution_t {
	DM_RESOLUTION_1920X1080,
	DM_RESOLUTION_1600X900
} dm_resolution_t;

/* Selects the preset and its logical screen-coordinate dimensions. */
void dm_set_screen_resolution(dm_resolution_t resolution);
/* Overrides logical screen dimensions without changing the selected preset. */
void dm_set_screen_dimensions(int width, int height);
int dm_get_screen_width(void);
int dm_get_screen_height(void);
int dm_scale_to_res(int dimension);
#endif
