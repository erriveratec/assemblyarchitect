#include "instruction_layout_il.h"

#include "dimensions_dm.h"

static const int INSTRUCTION_BOX_WIDTH = 170;
static const int INSTRUCTION_BOX_HEIGHT = 225;
static const int INSTRUCTION_BOX_Y = 225;

SDL_Rect il_get_initial_instruction_bounds(void)
{
	SDL_Rect bounds = {
		.x = 0,
		.y = dm_scale_to_res(INSTRUCTION_BOX_Y),
		.w = dm_scale_to_res(INSTRUCTION_BOX_WIDTH),
		.h = dm_scale_to_res(INSTRUCTION_BOX_HEIGHT)
	};
	return bounds;
}
