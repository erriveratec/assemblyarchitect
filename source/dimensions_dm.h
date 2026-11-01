#ifndef DIMENSIONS_DM_H
#define DIMENSIONS_DM_H

enum {
	DM_LOGICAL_CANVAS_WIDTH = 1920,
	DM_LOGICAL_CANVAS_HEIGHT = 1080
};

/*
 * Return canonical logical-canvas dimensions.
 * These are never SDL window, display, viewport, or renderer-output sizes.
 */
int dm_get_logical_width(void);
int dm_get_logical_height(void);
#endif
