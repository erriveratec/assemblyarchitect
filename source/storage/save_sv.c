#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#include "storage/save_sv.h"
#include "aux.h"
#include "dbg.h"
#include "file_fl.h"
#include "levels_lv.h"


#define READ_ERROR -1
#define SAVE_FILE_LINE_LENGTH 15

#define SAVE_FILE_PATH "data/save.dat"
#define STR_PLAYER_ENDS "PLAYER ENDS"


bool check_if_level_is_active(FILE *fp);
bool check_if_save_file_exists();
void fl_write_to_file(FILE *fp, char *string);





/* Function: check_if_level_is_active
 *------------------------------------------------------------------------------
 * Determines if the active flag for a given level is true or false
 *
 * Arguments:
 *	fp: the file pointer of the level data
 *
 * Return:
 *	boolean: true if the level was active, false if otherwise.
 *
 */
bool check_if_level_is_active(FILE *fp)
{
	char *line = NULL;	
	size_t len = 0;
	ssize_t read;

	char *saveptr1;
	char *text;
	bool level_is_active = false;

	while ((read = getline(&line, &len, fp)) != READ_ERROR){
		if (strstr(line, STR_LEVEL_ACTIVE_TRUE) != NULL){
			level_is_active = true;
			break;
		} else if (strstr(line, STR_LEVEL_ACTIVE_FALSE) != NULL){
			level_is_active = false;
			break;
		} 
	}
	return level_is_active;
}




/* Function: get_player_end_string
 *------------------------------------------------------------------------------
 * This function is called to generate a string that will be looked on to the 
 * file to know when a a player info is finished
 *
 * Arguments:
 *	player: The player id number.
 *
 * Return:
 *	char * to the created string.
 *
 */
static char *get_player_end_string(int player_id)
{
	char *number = ax_number_to_string_prepend_zero(player_id);
	check_mem(number);
	char *id = malloc(sizeof(char)*(strlen(STR_PLAYER_ENDS) + 
					  strlen(ax_char_space) + strlen(number)));
	check_mem(id);

	strcpy(id, STR_PLAYER_ENDS);
	strcat(id, ax_char_space);
	strcat(id, number);
	free(number);

error:
	return id;
}


/* Function: sv_load_architect
 *------------------------------------------------------------------------------
 * Fills an array passed as argument with the list of levels available for the
 * player
 *
 * Arguments:
 *	levels_array: an boolean array with the state of the available levels
 *
 * Return:
 *	void.
 *
 */
void sv_load_architect(int player_id, bool *levels_array)
{
	assert(player_id >= SV_ARCHITECT_EXECUTOR_X && 
		   player_id <= SV_ARCHITECT_HANDLER_Z &&
		   "Invalid player id");
	assert(levels_array != NULL && "levels array pointer is NULL");

	char *line = NULL;	
	size_t len = 0;
	ssize_t read;
	char path[512];

	ax_get_resource_path(path, sizeof(path), SAVE_FILE_PATH);
	FILE *fp = fopen(path, "r");
	check_mem(fp);
	char *saveptr1;
	char *text;

	char *player = fl_get_player_id_string(player_id);
	char *player_end = get_player_end_string(player_id);
	bool player_found = false;
	bool level_found = false;
	
	char *level = NULL;
	int level_num = 0;
	while (READ_ERROR != (read = getline(&line, &len, fp))){
		level = fl_get_level_id_string(level_num);
		if (strstr(line, player) != NULL){
			player_found = true;
		} else if (strstr(line, player_end) != NULL){
			player_found = false;
			break;
		}
		else if (strstr(line, level) != NULL && player_found == true){
			bool is_level_active = check_if_level_is_active(fp);
			levels_array[level_num] = is_level_active;
			level_num++;
		} 
	}

error:
	free(level);	
	free(player);	
	fclose(fp);	
	return;

}

/* Function: check_if_save_file_exists
 *------------------------------------------------------------------------------
 * This function verifies if the save file for the game exists. 
 *
 * Arguments:
 *	None.
 *
 * Return:
 *	bool true if save file exists, false if otherwise.
 *
 */
bool check_if_save_file_exists()
{
	char path[512];
	ax_get_resource_path(path, sizeof(path), SAVE_FILE_PATH);
	bool file_exists = (access(path, F_OK) == 0);
	return file_exists;
}

/* Function: fl_save_init_default
 *------------------------------------------------------------------------------
 * This function verifies if the save file for the game exists, if it doesn't 
 * exists it creates it and initializes it
 *
 * Arguments:
 *  None.
 *
 * Return:
 *	void.
 *
 */
void sv_save_init_default()
{
	bool file_exists = check_if_save_file_exists();

	if (file_exists == false){
		char path[512];	
		ax_get_resource_path(path, sizeof(path), SAVE_FILE_PATH);
		FILE *fp = fopen(path, "w");
		char *level = malloc(sizeof(char)*SAVE_FILE_LINE_LENGTH);
		check_mem(level);

		for (int j = 1; j <= SV_ARCHITECT_HANDLER_Z; j++){
			char *player_number = ax_number_to_string_prepend_zero(j);

			strcpy(level, STR_PLAYER); 
			strcat(level, " ");
			strcat(level, player_number);
			strcat(level, ax_char_newline);
			fl_write_to_file(fp, level);
			strcpy(level, ax_char_newline);
			fl_write_to_file(fp, level);
			for (int i = 0; i < LV_LEVEL_QUANTITY; i++){
				char *number = NULL;
				if (i<10){
					number = ax_number_to_string_prepend_zero(i);
				} else {
					number = ax_number_to_string(i);
				}

				check_mem(number);
				
				strcpy(level, STR_LEVEL_STARTS); 
				strcat(level, " ");
				strcat(level, number);
				strcat(level, ax_char_newline);
				fl_write_to_file(fp, level);
				strcpy(level, ax_char_newline);
				fl_write_to_file(fp, level);

				if (i == 0){
					strcpy(level, STR_LEVEL_ACTIVE_TRUE);
				}else {
					strcpy(level, STR_LEVEL_ACTIVE_FALSE);
				}

				fl_write_to_file(fp, level);
				
				strcpy(level, ax_char_newline);
				fl_write_to_file(fp, level);
				
				strcpy(level, STR_CODE_STARTS);
				fl_write_to_file(fp, level);
				
				if (i == 1){
					strcpy(level, FL_L1_CODE_1);
					fl_write_to_file(fp, level);
					strcpy(level, FL_L1_CODE_2);
					fl_write_to_file(fp, level);
					strcpy(level, FL_L1_CODE_3);
					fl_write_to_file(fp, level);

				} else if (i == 8){
					strcpy(level, FL_L8_CODE_1);
					fl_write_to_file(fp, level);
					strcpy(level, FL_L8_CODE_2);
					fl_write_to_file(fp, level);

				} else {
					strcpy(level, ax_char_newline);
					fl_write_to_file(fp, level);
				}
				
				strcpy(level, STR_CODE_ENDS);
				fl_write_to_file(fp, level);
				
				strcpy(level, ax_char_newline);
				fl_write_to_file(fp, level);

				strcpy(level, STR_LEVEL_ENDS);
				strcat(level, " ");
				strcat(level, number);
				strcat(level, ax_char_newline);
				fl_write_to_file(fp, level);
				strcpy(level, ax_char_newline);
				fl_write_to_file(fp, level);
				free(number);
			}
			strcpy(level, STR_PLAYER_ENDS); 
			strcat(level, " ");
			strcat(level, player_number);
			strcat(level, ax_char_newline);
			fl_write_to_file(fp, level);
			strcpy(level, ax_char_newline);
			fl_write_to_file(fp, level);

			free(player_number);
		}
		free(level);
		fclose(fp);
	}
	error:
	return;
}

