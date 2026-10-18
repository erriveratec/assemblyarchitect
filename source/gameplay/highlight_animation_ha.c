#include "gameplay/highlight_animation_ha.h"

#include <math.h>

#include "draw_dw.h"

static float g_progress;
static float g_frame_progress;
static bool g_descending;
static bool g_frame_descending;

void ha_reset(void)
{
	g_progress = 0.0f;
	g_frame_progress = 0.0f;
	g_descending = false;
	g_frame_descending = false;
}

void ha_begin_frame(void)
{
	g_frame_progress = g_progress;
	g_frame_descending = g_descending;

	int animation_limit = dw_get_animation_limit();
	int animation_step = dw_get_animation_step();
	if (animation_limit <= 0 || animation_step <= 0) {
		ha_reset();
		return;
	}

	float delta = (float)animation_step / (float)animation_limit;
	if (!g_descending && g_progress >= 1.0f) {
		g_descending = true;
	} else if (g_descending && g_progress <= 0.0f) {
		g_descending = false;
	}
	g_progress += g_descending ? -delta : delta;
	if (g_progress > 1.0f) {
		g_progress = 1.0f;
	} else if (g_progress < 0.0f) {
		g_progress = 0.0f;
	}
}

float ha_get_progress(void)
{
	return g_frame_progress;
}

float ha_get_value(float animation_limit)
{
	if (!isfinite(animation_limit) || animation_limit <= 0.0f) {
		return 0.0f;
	}
	return g_frame_progress * animation_limit;
}

bool ha_is_descending(void)
{
	return g_frame_descending;
}