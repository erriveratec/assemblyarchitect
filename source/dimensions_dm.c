#include<stdio.h>
#include<stdlib.h>
#include"dimensions_dm.h"
#include <SDL.h>

int g_res_id;
int g_screen_width;
int g_screen_height;

int scale_to_resolution(int dim);

/* Function: dm_get_press_space_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the press space texture.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the studio name box
 */
SDL_Rect dm_get_press_space_box()
{
	const int texture_w = 400;
	
	SDL_Rect b;
	b.w = scale_to_resolution(texture_w);
	b.x = (g_screen_width - b.w)/2;
	b.y = g_screen_height/2;
	b.h = 0;
	return b;
}

/* Function: dm_get_studio_name_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the studio name texture.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the studio name box
 */
SDL_Rect dm_get_studio_name_box()
{
	const int texture_w = 600;
	
	SDL_Rect b;
	b.w = scale_to_resolution(texture_w);
	b.x = (g_screen_width - b.w)/2;
	b.y = g_screen_height/3;
	b.h = 0;
	return b;
}

/* Function: dm_get_game_title_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the game title texture.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the studio name box
 */
SDL_Rect dm_get_game_title_box()
{
	const int texture_w = 1100;
	const int texture_y = 25;
	
	SDL_Rect b;
	b.w = scale_to_resolution(texture_w);
	b.x = (g_screen_width - b.w)/2;
	b.y = scale_to_resolution(texture_y);
	b.h = 0;
	return b;
}

/* Function: set_screen_resolution
 * -----------------------------------------------------------------------------
 * Sets the screen resolution that will be used in the game
 *
 * Arguments:
 *	resolution_id: The id of the resolution that will be used
 *
 * Return:
 *	Void.
 */
void dm_set_screen_resolution(int resolution_id)
{
	switch(resolution_id){
		case R1600X900:
			g_screen_width = 1600;
			g_screen_height = 900;
			g_res_id = R1600X900;
			break;
		default:
			g_screen_width = 1600;
			g_screen_height = 900;
			g_res_id = R1600X900;
			break;
	}	
}

/* Function: scale_to_resoution
 * -----------------------------------------------------------------------------
 * Scales the given dimension according to the scree resolution. The baseline
 * resolution is 1600x900
 *
 * Arguments:
 *	dim: The dimension that will be scaled according to the resolution
 *
 * Return:
 *	Scale variable.
 */
int scale_to_resolution(int dim)
{
	int scaled_dim;

	switch(g_res_id){
		case R1600X900:
			scaled_dim = dim*1;
			break;
		default:
			scaled_dim = dim*1;
			break;
	}
	return scaled_dim;
}


















