#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL.h>
#include <SDL_mixer.h>
#include "sdl_config.h"
#include "ui/escape_menu_em.h"


bool g_escape_menu = false;


/* Function: player_pressed_escape_key
 * ----------------------------------------------------------------------------
 * This function inverts the state of the escape menu flag in case that the 
 * player has pressed the escape key.
 *
 * Arguments:
 * 	None.
 *
 * Return:
 *	void.	
 */
void em_toggle_escape_menu()
{
	g_escape_menu = !g_escape_menu;

	if (g_escape_menu == true){			
		if (g_sfx_menu) Mix_PlayChannel(-1, g_sfx_menu, 0);
	} else if (g_escape_menu == false){			
		if (g_sfx_iface_back_cancel) Mix_PlayChannel(-1, 
			g_sfx_iface_back_cancel, 0);
	} 
}

/* Function: em_get_escape_state
 * ----------------------------------------------------------------------------
 * This function return the boolean state of the state.
 *
 * Arguments:
 * 	None.
 *
 * Return:
 *	Boolean with the state fo the escape_menu variable
 */
bool em_get_escape_state()
{
	return g_escape_menu;
}
