#include "ui/typography_ty.h"

#include "dimensions_dm.h"

static const int STAGE_LABEL_HEIGHT = 25;
static const int MESSAGE_HEIGHT = 35;
static const int EMPHASIS_HEIGHT = 50;
static const int CODE_HEIGHT = 40;

/* These are target box heights for fit-height text rendering, not font metrics. */

int ty_stage_label_height(void)
{
	return dm_scale_to_res(STAGE_LABEL_HEIGHT);
}

int ty_message_height(void)
{
	return dm_scale_to_res(MESSAGE_HEIGHT);
}

int ty_emphasis_height(void)
{
	return dm_scale_to_res(EMPHASIS_HEIGHT);
}

int ty_code_height(void)
{
	return dm_scale_to_res(CODE_HEIGHT);
}
