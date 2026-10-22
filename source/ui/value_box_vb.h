#ifndef VALUE_BOX_VB_H
#define VALUE_BOX_VB_H

#include <stdbool.h>
#include "draw_dw.h"

typedef struct vb_value_box_t {
	int value;
	int type;
	bool visible_box;
	SDL_Rect box;
	texture_t *t;
} vb_value_box_t;

SDL_Rect vb_get_size(void);
SDL_Rect vb_get_text_size(void);
int vb_get_vertical_offset(void);
void vb_copy(vb_value_box_t *dst, vb_value_box_t src, bool copy_position);
/* Display bounds may be larger than the canonical bounds used to transport values. */
void vb_copy_for_transport(vb_value_box_t *dst, const vb_value_box_t *src);
void vb_draw(vb_value_box_t *box, SDL_Color color);

#endif
