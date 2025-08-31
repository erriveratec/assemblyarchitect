#include<stdio.h>
#include<stdlib.h>
#include"dimensions_dm.h"
#include <SDL.h>

#define P_BUTTON_H 200
#define P_BUTTON_W 200
#define ESC_MENU_BOX_W 600
#define ESC_MENU_BOX_H 300
#define ESC_MENU_BUTTON_W 500
#define ESC_MENU_BUTTON_H 60
#define SEL_LEVEL_BUTTON_W 100
#define SEL_LEVEL_BUTTON_H 50
#define RET_BUTTON_W 90
#define RET_BUTTON_H 75
#define BORDERS_OFFSET 2

int g_res_id;
int g_screen_width;
int g_screen_height;

int scale_to_resolution(int dim);

/* Function: dm_get_arrow_ins_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object, x and y are initialize at 0
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_arrow_ins_box()
{
	SDL_Rect b;
	b.w = (g_screen_width*2/6)/3;
	b.h = g_screen_height/4;
	b.x = 0;
	b.y = g_screen_height/2 - b.h;
	return b;
}

/* Function: dm_get_instructions_stage_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object, x and y are initialize at 0
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_instructions_stage_box()
{
	SDL_Rect b;
	b.w = (g_screen_width*2/6)/3;
	b.h = g_screen_height/4;
	b.x = 0;
	b.y = g_screen_height/2 - b.h;
	return b;
}
/* Function: dm_get_code_stage_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object, x and y are initialize at 0
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_code_stage_box()
{
	SDL_Rect ib = dm_get_instructions_stage_box();
	SDL_Rect b;

	b.w = ((g_screen_width*2/6)*2/3);
	b.h = (g_screen_height*2/3);
	b.x = ib.x + ib.w;
	b.y = g_screen_height/18;
	return b;
}

/* Function: dm_get_registers_stage_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object, x and y are initialize at 0
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_registers_stage_box()
{
	SDL_Rect cb = dm_get_code_stage_box();
	SDL_Rect b;
	b.w = g_screen_width/6;
	b.h = g_screen_height/4;
	b.x = cb.x + cb.w;
	b.y = 0;
	return b;
}

/* Function: dm_get_return_button_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object, x and y are initialize at 0
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_return_button_box()
{
	SDL_Rect b;
	b.w = scale_to_resolution(RET_BUTTON_W);
	b.h = scale_to_resolution(RET_BUTTON_H);
	b.x = BORDERS_OFFSET;
	b.y = g_screen_height - b.h - BORDERS_OFFSET;
	return b;
}

/* Function: dm_get_sel_level_offset_y
 * -----------------------------------------------------------------------------
 * Returns the offset for the sel level buttons
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset for the sel level buttons
 */
int get_sel_level_offset_y()
{
	int y_offset =  scale_to_resolution(SEL_LEVEL_BUTTON_H + 
										0.5*SEL_LEVEL_BUTTON_H);
	return y_offset;
	
}

/* Function: dm_get_level_button_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object, x and y are initialize at 0
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_level_button_box()
{
	SDL_Rect b;
	b.w = scale_to_resolution(SEL_LEVEL_BUTTON_W);
	b.h = scale_to_resolution(SEL_LEVEL_BUTTON_H);
	b.x = (g_screen_width - (5*b.w))/6;
	b.y = g_screen_height/6;
	return b;
}
/* Function: dm_get_select_level_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object.
 */
SDL_Rect dm_get_select_level_box()
{
	const int texture_w = 700;
	SDL_Rect b;
	b.w = scale_to_resolution(texture_w);
	b.h = 0;
	b.x = (g_screen_width -  b.w)/2;
	b.y = g_screen_height/36;
	return b;
}

/* Function: dm_get_select_player_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object.
 */
SDL_Rect dm_get_select_player_box()
{
	const int texture_h = 90;
	SDL_Rect b;
	b.h = scale_to_resolution(texture_h);
	b.x = g_screen_width/32;
	b.y = g_screen_height/36;
	b.w = 0;
	return b;
}

/* Function: dm_get_code_button_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object, x and y are initialize at 0
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_code_button_box()
{
	SDL_Rect b;
	b.w = scale_to_resolution(CODE_BUTTON_W);
	b.h = scale_to_resolution(CODE_BUTTON_H);
	b.x = 0;
	b.y = 0;
	return b;
}


/* Function: dm_get_escape_menu_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_escape_menu_box()
{
	SDL_Rect b;
	b.w = scale_to_resolution(ESC_MENU_BOX_W);
	b.h = scale_to_resolution(ESC_MENU_BOX_H);
	b.x = (g_screen_width - b.w)/2;
	b.y = (g_screen_height - b.h)/2;
	return b;
}

/* Function: dm_get_escape_b1_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_escape_b1_box()
{
	SDL_Rect mb = dm_get_escape_menu_box();

	SDL_Rect b;
	b.w = scale_to_resolution(ESC_MENU_BUTTON_W);
	b.h = scale_to_resolution(ESC_MENU_BUTTON_H);
	b.x = mb.x + mb.w/2 - b.w/2;
	b.y = mb.y + (mb.h - 3*b.h)/4 ;
	return b;
}

/* Function: dm_get_escape_b1_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_escape_b2_box()
{
	SDL_Rect mb = dm_get_escape_menu_box();
	SDL_Rect b1 = dm_get_escape_b1_box();
	
	SDL_Rect b;
	b.w = scale_to_resolution(ESC_MENU_BUTTON_W);
	b.h = scale_to_resolution(ESC_MENU_BUTTON_H);
	b.x = b1.x;
	b.y = b1.y + (mb.h - 3*b.h)/4 + b.h;
	return b;
}

/* Function: dm_get_escape_b3_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_escape_b3_box()
{
	SDL_Rect mb = dm_get_escape_menu_box();
	SDL_Rect b2 = dm_get_escape_b2_box();
	
	SDL_Rect b;
	b.w = scale_to_resolution(ESC_MENU_BUTTON_W);
	b.h = scale_to_resolution(ESC_MENU_BUTTON_H);
	b.x = b2.x;
	b.y = b2.y + (mb.h - 3*b.h)/4 + b.h;
	return b;
}
/* Function: dm_get_p1_button_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_p1_button_box()
{
	SDL_Rect b;
	b.w = scale_to_resolution(P_BUTTON_W);
	b.h = scale_to_resolution(P_BUTTON_H);
	b.x = (g_screen_width - 3*b.w)/4;
	b.y = g_screen_height/3;
	return b;
}

/* Function: dm_get_p2_button_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_p2_button_box()
{
	SDL_Rect b;
	b.w = scale_to_resolution(P_BUTTON_W);
	b.h = scale_to_resolution(P_BUTTON_H);
	b.x = 2*(g_screen_width - 3*b.w)/4 + b.w;
	b.y = g_screen_height/3;
	return b;
}

/* Function: dm_get_p3_button_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
SDL_Rect dm_get_p3_button_box()
{
	SDL_Rect b;
	b.w = scale_to_resolution(P_BUTTON_W);
	b.h = scale_to_resolution(P_BUTTON_H);
	b.x = 3*(g_screen_width - 3*b.w)/4 + 2*b.w;
	b.y = g_screen_height/3;
	return b;
}
/* Function: dm_get_press_space_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object.
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

/* Function: dm_get_studio_name_msg_box
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object.
 */
SDL_Rect dm_get_studio_name_msg_box()
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
 * Returns the box dimensions for the object.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object.
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


















