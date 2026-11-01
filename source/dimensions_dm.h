#ifndef DIMENSIONS_DM_H
#define DIMENSIONS_DM_H

enum {
	DM_LOGICAL_CANVAS_WIDTH = 1920,
	DM_LOGICAL_CANVAS_HEIGHT = 1080
};

/* Return canonical logical canvas dimensions, never SDL window/output sizes. */
int dm_get_screen_width(void);
int dm_get_screen_height(void);
#endif
