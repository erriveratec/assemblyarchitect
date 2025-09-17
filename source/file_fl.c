#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <assert.h>
#include "aux.h"
#include "dbg.h"
#include "instruction_window_iw.h"
#include "code_window_cw.h"
#include "buffers_bf.h"
#include "registers_rg.h"
#include "dimensions_dm.h"
#include "file_fl.h"
#include "levels_lv.h"
#include "text_tx.h"

#define SAVE_FILE_LINE_LENGTH 15
#define MSG_LENGTH 256

#define READ_ERROR -1
#define LEVELS_FILE_PATH "data/levels.dat"
#define SAVE_FILE_PATH "data/save.dat"
#define SAVE_FILE_PATH_TEMP "data/save.dattemp"
#define MSGS_FILE_PATH "data/level_msgs.dat"

// Text for the save file creation
#define STR_LEVEL_STARTS "+++LEVEL STARTS"
#define STR_LEVEL_ENDS "+++LEVEL ENDS"
#define STR_CODE_STARTS "CODE STARTS\n"
#define STR_CODE_ENDS "CODE ENDS\n"
#define STR_LEVEL_ACTIVE_TRUE "LEVEL ACTIVE TRUE\n"
#define STR_LEVEL_ACTIVE_FALSE "LEVEL ACTIVE FALSE\n"
#define STR_LEVEL_ACTIVE "LEVEL ACTIVE"
#define STR_LEVEL "Level"
#define STR_PLAYER "PLAYER"
#define STR_PLAYER_ENDS "PLAYER ENDS"

//Load level strings
#define STR_CHALLENGE_TEXT_BEGIN "ChallengeTextBegin"
#define STR_CHALLENGE_TEXT_END "ChallengeTextEnd"
#define STR_INPUT_SIZE "InputSize"
#define STR_INSTRUCTION_LIMIT "InstructionLimit"
#define STR_INPUT_TYPE "InputType"
#define STR_NATURAL "Natural"
#define STR_WHOLE "Whole"
#define STR_CHAR "Char"
#define STR_MIXED "Mixed"
#define STR_INSTRUCTIONS_BEGIN "InstructionsBegin"
#define STR_INSTRUCTIONS_END "InstructionsEnd"
#define STR_REGISTERS_BEGIN "RegistersBegin"
#define STR_REGISTERS_END "RegistersEnd"
#define STR_WIN_CONDITION_BEGIN "WinConditionBegin"
#define STR_WIN_CONDITION_END "WinConditionEnd"

//LEVEL MESSAGES STRINGS
#define STR_MSG_QTY "QUANTITY"
#define STR_MSG "MSG"
#define STR_MSG_END "MSG_END"


// Level specific instructions
// ----- LEVEL 2 ----
#define STR_L2_CODE_1 "mov [ob], rax\n"
#define STR_L2_CODE_2 "mov rax, rax\n"
#define STR_L2_CODE_3 "mov rax, rax\n"

// ----- LEVEL 9 ----
#define STR_L9_CODE_1 "mov rax, [ib]\n"
#define STR_L9_CODE_2 "mov [ob], rax\n"


static char *get_level_id_string(int level_id);
static char *get_player_id_string(int player_id);
static char *get_player_end_string(int player_id);
static void parse_challenge_text(FILE *fp);
static int get_level_id_number(int level_id);
bool check_if_save_file_exists();
static char *get_delimiter_level_string(char *text, int level_id);
static void parse_instructions(FILE *fp);
static void parse_registers(FILE *fp);
bool check_if_save_file_exists();
void write_to_file(FILE *fp, char *string);
void copy_file(char *src, char *dst);
void delete_file(char *path);
void write_player_code_to_file(FILE *fp);
static void parse_saved_code(FILE *fp);
static char *create_string_with_number(char *s,  int n);
bool check_if_level_is_active(FILE *fp);
static void parse_win_condition(FILE *fp);
static void parse_message(FILE *fp, int msg_pos);

/* Function: create_string_with_number
 *------------------------------------------------------------------------------
 * Creates a string with a number attached to the end
 *
 * Arguments:
 * s: string to be used in the first part
 * n: number that will be used.
 *
 * Return:
 *	char * to the created string.
 *
 */
static char *create_string_with_number(char *s,  int n)
{
	assert(s != NULL && "The string pointer is NULL");


	char *number = ax_number_to_string_prepend_zero(n);
	check_mem(number);
	char *string = malloc(sizeof(char)*(strlen(s) + strlen(number)));
	check_mem(string);

	strcpy(string, s);
	strcat(string, char_space);
	strcat(string, number);
	free(number);

error:
	return string;
}

/* Function: get_level_id_string
 *------------------------------------------------------------------------------
 * This function is called to generate a string that will be looked on to the 
 * file to search a level
 *
 * Arguments:
 *	level: The level number that will be search in the file
 *
 * Return:
 *	char * to the created string.
 *
 */
static char *get_level_id_string(int level_id)
{
	char *number = NULL;

	if (level_id < 10){
		number = ax_number_to_string_prepend_zero(level_id);
	} else {
		number = ax_number_to_string(level_id);
	}
	
	check_mem(number);
	char *id = malloc(sizeof(char)*(strlen(STR_LEVEL_STARTS) + 
					  strlen(char_space) + strlen(number)));
	check_mem(id);

	strcpy(id, STR_LEVEL_STARTS);
	strcat(id, char_space);
	strcat(id, number);
	free(number);

error:
	return id;
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
					  strlen(char_space) + strlen(number)));
	check_mem(id);

	strcpy(id, STR_PLAYER_ENDS);
	strcat(id, char_space);
	strcat(id, number);
	free(number);

error:
	return id;
}

/* Function: get_player_id_string
 *------------------------------------------------------------------------------
 * This function is called to generate a string that will be looked on to the 
 * file to search a level.
 *
 * Arguments:
 *	player: The player id number.
 *
 * Return:
 *	char * to the created string.
 *
 */
static char *get_player_id_string(int player_id)
{
	char *number = ax_number_to_string_prepend_zero(player_id);
	check_mem(number);
	char *id = malloc(sizeof(char)*(strlen(STR_PLAYER) + strlen(char_space) + 
					  strlen(number)));
	check_mem(id);

	strcpy(id, STR_PLAYER);
	strcat(id, char_space);
	strcat(id, number);
	free(number);

error:
	return id;
}
/* Function: get_delimeter_level_string
 *------------------------------------------------------------------------------
 * This function is called to generate a string that will be looked on to the 
 * save file to search a level
 *
 * Arguments:
 *	level: The level number that will be search in the file
 *
 * Return:
 *	char * to the created string.
 *
 */
static char *get_delimeter_level_string(char *text, int level_id)
{
	char *number = ax_number_to_string_two_digits(level_id - 
								                     LV_LEVEL_MIN);
	check_mem(number);
	char *string = malloc(sizeof(char)*(strlen(text) + 2*CHAR_SIZE + 
						  strlen(number)+1));
	check_mem(string);

	strcpy(string, text);
	strcat(string, " \0");
	strcat(string, number);
	strcat(string, "\n\0");
	free(number);

error:
	return string;
}

/* Function: parse_challenge_text
 *------------------------------------------------------------------------------
 * Reads the text that will be assigned to particular level.
 *
 * Arguments:
 *	fp: the file pointer of the level data
 *
 * Return:
 *	void.
 *
 */
static void parse_challenge_text(FILE *fp)
{
	char *line = NULL;	
	size_t len = 0;
	ssize_t read;

	char *saveptr1;
	char *text;
	char *delim = "\"";

	while ((read = getline(&line, &len, fp)) != READ_ERROR){
		
		text =  strtok_r(line, delim, &saveptr1);
		if (strstr(line, STR_CHALLENGE_TEXT_END) != NULL){
			break;
		} else {
			cw_set_challenge_text(text);
		}
	}
	return;
}
/* Function: parse_win_condition
 *------------------------------------------------------------------------------
 * Processes the win condition specified in the text file
 *
 * Arguments:
 *	fp: the file pointer of the level data
 *
 * Return:
 *	Void.
 *
 */
static void parse_win_condition(FILE *fp)
{
	char *line = NULL;	
	size_t len = 0;
	ssize_t read;

	char *saveptr1;
	char *text;

	while ((read = getline(&line, &len, fp)) != READ_ERROR){
		text =  strtok_r(line, char_newline, &saveptr1);
		if (strstr(STR_WIN_CONDITION_END, line) != NULL){
			break;
		} else if (strcmp(line, char_newline) != STRING_EQUAL){
			lv_set_level_win_condition(text);			
		} 	
	}
	return;
}

/* Function: parse_saved_code
 *------------------------------------------------------------------------------
 * This function processes the saved code for a level in the save file.
 *
 * Arguments:
 *	fp: the file pointer of the level data
 *
 * Return:
 *	void.
 *
 */
static void parse_saved_code(FILE *fp)
{
	char *line = NULL;	
	size_t len = 0;
	ssize_t read;

	char *saveptr1;
	char *text;

	while ((read = getline(&line, &len, fp)) != READ_ERROR){
		text =  strtok_r(line, char_newline, &saveptr1);
		if (strstr(STR_CODE_ENDS, line) != NULL){
			cw_update_saved_jump_instructions();
			break;
		} else if (strcmp(line, char_newline) != STRING_EQUAL){
			cw_add_saved_line(text);
		} 	
	}
	return;
}

/* Function: parse_instructions
 *------------------------------------------------------------------------------
 * Arguments:
 *	fp: the file pointer of the level data
 *
 * Return:
 *	void.
 */
static void parse_instructions(FILE *fp)
{
	char *line = NULL;	
	size_t len = 0;
	ssize_t read;

	while ((read = getline(&line, &len, fp)) != READ_ERROR){
		if (strstr(line, add_text) != NULL){
			iw_add_instruction_to_list(ADD);
		} else if (strstr(line, mov_text) != NULL){
			iw_add_instruction_to_list(MOV);
		} else if (strstr(line, label_text) != NULL){
			iw_add_instruction_to_list(LABEL);
		} else if (strstr(line, jmp_text) != NULL){
			iw_add_instruction_to_list(JMP);
		} else if (strstr(line, STR_INSTRUCTIONS_END)){
			break;
		} else {
			printf("Error: a non valid instruction was read -> %s\n", line);
		}
	}
}

/* Function: parse_registers
 *------------------------------------------------------------------------------
 * Arguments:
 *	fp: the file pointer of the level data
 *
 * Return:
 *	void.
 */
static void parse_registers(FILE *fp)
{
	char *line = NULL;	
	size_t len = 0;
	ssize_t read;

	while ((read = getline(&line, &len, fp)) != READ_ERROR){
		if (strstr(line, "rax") != NULL){
			rg_add_register_to_list(RAX);
		} else if (strstr(line, "rbx") != NULL){
			rg_add_register_to_list(RBX);
		} else if (strstr(line, "rcx") != NULL){
			rg_add_register_to_list(RCX);
		} else if (strstr(line, "rdx") != NULL){
			rg_add_register_to_list(RDX);
		} else if (strstr(line, "rdi") != NULL){
			rg_add_register_to_list(RDI);
		} else if (strstr(line, STR_REGISTERS_END)){
			break;
		} else {
			printf("Error: a non valid register was read -> %s\n", line);
		}
	}
}


/* Function: fl_file_initialize_level
 *------------------------------------------------------------------------------
 * Initializes level Lists (registers, instructions) and text based on what
 * is contained in the text file.
 *
 * Arguments:
 *	level: The level number that will be looked in the text file.
 *
 * Return:
 *	void.
 *
 */
void fl_file_initialize_level(int level_id)
{
	char *line = NULL;	
	size_t len = 0;
	ssize_t read;

	FILE *fp = fopen(LEVELS_FILE_PATH, "r");
	check_mem(fp);
	char *saveptr1;
	char *text;

	char *level = get_level_id_string(level_id);
	bool level_found = false;
	while (READ_ERROR != (read = getline(&line, &len, fp))){
		if (strstr(line, level) != NULL){
			level_found = true;
			char *name = create_string_with_number(STR_LEVEL, level_id);
			cw_set_stage_name(name);
			free(name);
		} else if (strstr(line, STR_CHALLENGE_TEXT_BEGIN) != NULL &&
														   level_found == true){
			parse_challenge_text(fp);
		} else if (strstr(line, STR_INPUT_SIZE) != NULL && level_found == true){
			char *size = strchr(line, CHAR_SPACE);
			bf_set_input_buffer_size(atoi(size));
		} else if (strstr(line, STR_INSTRUCTION_LIMIT) != NULL && 
				   										   level_found == true){
			char *size = strchr(line, CHAR_SPACE);
			lv_set_level_instructions_limit(atoi(size));
		} else if (strstr(line, STR_INPUT_TYPE) != NULL && level_found == true){
			if (strstr(line, STR_NATURAL)){
				bf_create_natural_numbers_input_list(g_input_buffer_size);
				g_input_list_type = NATURAL;
			} else if (strstr(line, STR_WHOLE)){

			} else if (strstr(line, STR_CHAR)){

			} else if (strstr(line, STR_MIXED)){

			}

		} else if (strstr(line, STR_INSTRUCTIONS_BEGIN) != NULL &&
				   										   level_found == true){
			iw_create_instruction_list();
			parse_instructions(fp);
		} else if (strstr(line, STR_REGISTERS_BEGIN) != NULL &&
				   										   level_found == true){
			create_register_list();
			parse_registers(fp);
		} else if (strstr(line, STR_WIN_CONDITION_BEGIN) != NULL &&
				   										   level_found == true){
			parse_win_condition(fp);
		} else if (strstr(line, STR_LEVEL_ENDS) != NULL && level_found == true){
			level_found= false;
			break;
		}

	}
	rg_update_register_box_position();
error:
	free(level);
	fclose(fp);
	return;
}

/* Function: parse_message
 *------------------------------------------------------------------------------
 * This function processes a message of a level
 *
 * Arguments:
 *	fp: the file pointer of the level data
 *
 * Return:
 *	void.
 *
 */
static void parse_message(FILE *fp, int msg_pos)
{
	char *line = NULL;	
	size_t len = 0;
	ssize_t read;

	char *saveptr1;
	char *text;
	char msg[MSG_LENGTH] = "";

	int i = 0;
	while ((read = getline(&line, &len, fp)) != READ_ERROR){
		text =  strtok_r(line, char_newline, &saveptr1);
		
		if (strstr(STR_MSG_END, line) != NULL){
			tx_set_message_in_array(msg_pos, msg);
			break;
		} else if (i != 0){
			strcat(msg, char_newline);
		}  
		strcat(msg, text);
		i++;
	}
	return;
}

/* Function: fl_load_level_msgs
 *------------------------------------------------------------------------------
 * Reads the level messages from the file
 *
 * Arguments:
 *	file: string containing the name of the save file
 *
 * Return:
 *	void.
 *
 */
void fl_load_level_msgs(int level_id)
{
	assert(level_id > LV_LEVEL_MIN && level_id < LV_LEVEL_MAX &&
		   "Invalid level id");

	char *line = NULL;	
	size_t len = 0;
	ssize_t read;

	FILE *fp = fopen(MSGS_FILE_PATH, "r");
	check_mem(fp);
	char *saveptr1;
	char *text;

	char *level = get_level_id_string(level_id);
	bool level_found = false;

	while (READ_ERROR != (read = getline(&line, &len, fp))){
		if (strstr(line, level) != NULL){
			level_found = true;
		} else if (strstr(line, STR_MSG_QTY) != NULL && level_found == true){
			char *qty = strchr(line, CHAR_SPACE);
			int size = atoi(qty);
			tx_set_and_allocate_msgs_array(size);
		} else if (strstr(line, STR_MSG) != NULL && level_found == true){
			char *pos = strchr(line, CHAR_SPACE);
			parse_message(fp, atoi(pos));
		} else if (strstr(line, STR_LEVEL_ENDS) != NULL && level_found == true){
			level_found= false;
			break;
		}
	}
error:
	free(level);	
	fclose(fp);	
	return;
}

/* Function: fl_load_save_file
 *------------------------------------------------------------------------------
 * Reads the save file to load dthe developed code for a level.
 *
 * Arguments:
 *	player_id: id of the player that the save will be loaded
 *	level_id: id of the level that will be loaded
 *
 * Return:
 *	void.
 *
 */
void fl_load_save_file(int player_id, int level_id)
{
	assert(player_id >= FL_PLAYER_1 && player_id <= FL_PLAYER_3 &&
		   "Invalid player id");
	assert(level_id > LV_LEVEL_MIN && level_id < LV_LEVEL_MAX &&
		   "Invalid level id");

	char *line = NULL;	
	size_t len = 0;
	ssize_t read;

	FILE *fp = fopen(SAVE_FILE_PATH, "r");
	check_mem(fp);
	char *saveptr1;
	char *text;

	char *player = get_player_id_string(player_id);
	char *level = get_level_id_string(level_id);
	bool player_found = false;
	bool level_found = false;

	while (READ_ERROR != (read = getline(&line, &len, fp))){
		if (strstr(line, player) != NULL){
			player_found = true;
		} else if (strstr(line, level) != NULL && player_found == true){
			level_found = true;
		} else if (strstr(line, STR_CODE_STARTS) != NULL && 
			level_found == true){
			parse_saved_code(fp);
			break;
		} 
	}

error:
	free(level);	
	free(player);	
	fclose(fp);	
	return;
}


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
/* Function: fl_load_player_levels
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
void fl_load_player_levels(int player_id, bool *levels_array)
{
	assert(player_id >= FL_PLAYER_1 && player_id <= FL_PLAYER_3 &&
		   "Invalid player id");
	assert(levels_array != NULL && "levels array pointer is NULL");

	char *line = NULL;	
	size_t len = 0;
	ssize_t read;

	FILE *fp = fopen(SAVE_FILE_PATH, "r");
	check_mem(fp);
	char *saveptr1;
	char *text;

	char *player = get_player_id_string(player_id);
	char *player_end = get_player_end_string(player_id);
	bool player_found = false;
	bool level_found = false;
	
	char *level = NULL;
	int level_num = 1;
	while (READ_ERROR != (read = getline(&line, &len, fp))){
		level = get_level_id_string(level_num);
		if (strstr(line, player) != NULL){
			player_found = true;
		} else if (strstr(line, player_end) != NULL){
			player_found = false;
			break;
		}
		else if (strstr(line, level) != NULL && player_found == true){
			bool is_level_active = check_if_level_is_active(fp);
			levels_array[level_num - 1] = is_level_active;
			level_num++;
		} 
	}

error:
	free(level);	
	free(player);	
	fclose(fp);	
	return;

}


/* Function: copy_file
 *------------------------------------------------------------------------------
 * This funcion receives a source and a destination file and copies one into 
 * the another.
 *
 * Arguments:
 *	src: string containing the source file path.
 *	dst: string containing the destination file path.
 *
 * Return:
 *	void.
 *
 */
void copy_file(char *dst, char *src)
{
	assert(src != NULL && dst != NULL && "The file pointer are invalid");
	FILE *src_fp = fopen(src, "r");
	FILE *dst_fp = fopen(dst, "w");
	check_mem(src_fp);
	check_mem(dst_fp);

	char *line = NULL;	
	size_t len = 0;
	ssize_t read;

	while (READ_ERROR != (read = getline(&line, &len, src_fp))){
		write_to_file(dst_fp, line);
	}
	fclose(src_fp);
	fclose(dst_fp);

	error:
	return;
}

/* Function: delete_file
 *------------------------------------------------------------------------------
 * This function deletes the file specified by the path contained in the string
 *
 * Arguments:
 *	file: path to the file that will be deleted
 *
 * Return:
 *	void.
 *
 */
void delete_file(char *file)
{
	assert(file != NULL && "The file pointer is invalid");

	if (remove(file) != 0){
		printf("Error: The file could not ve deleted\n");
	}

}



/* Function: write_player_code_to_file
 *------------------------------------------------------------------------------
 * This function writes to file the code developed by the player.
 *
 * Arguments:
 *	fp: The file pointer where the code is going to be written.
 *
 * Return:
 *	void.
 *
 */
void write_player_code_to_file(FILE *fp)
{
	assert(fp != NULL && "The file pointer is NULL");

	int list_size = cw_get_code_list_size();
	for (int i = 0; i < list_size; i++){
		int instruction = cw_get_instruction_at_code_pos(i);
		int op1 = NO_OPERAND;
		int op2 = NO_OPERAND;
		
		if (cl_get_instruction_operand_quantity(instruction) > ZERO_OPERANDS){
			op1 = cw_get_instruction_operand(i, FIRST_OP);
		}
		if (cl_get_instruction_operand_quantity(instruction) == TWO_OPERANDS){
			op2 = cw_get_instruction_operand(i, SECOND_OP);
		}

		char *line = cl_create_code_line_text(instruction, op1, op2);
		write_to_file(fp, line);
		free(line);
	}
}

/* Function: fl_save_level
 *------------------------------------------------------------------------------
 * This functions receives the state of the list developed by the player for a 
 * level and saves it to a text file for later storage.
 *
 * Arguments:
 *	level: The level number that will be looked in the text file.
 *
 * Return:
 *	void.
 *
 */
void fl_save_level(int player_id, int level_id)
{
	assert(level_id > LV_LEVEL_MIN && level_id < LV_LEVEL_MAX && 
		   "Invalid level");
	assert(player_id >= FL_PLAYER_1 && player_id <= FL_PLAYER_3 && 
		   "player");
	
	FILE *fp = fopen(SAVE_FILE_PATH, "r");
	FILE *fptemp = fopen(SAVE_FILE_PATH_TEMP, "w");
	check_mem(fp);
	check_mem(fptemp);

	char *line = NULL;	
	size_t len = 0;
	ssize_t read;

	char *player = get_player_id_string(player_id);
	char *level_start = get_delimeter_level_string(STR_LEVEL_STARTS, level_id);
	char *level_end = get_delimeter_level_string(STR_LEVEL_ENDS, level_id);
	bool level_found = false;
	bool player_found = false;

	while (READ_ERROR != (read = getline(&line, &len, fp))){
		write_to_file(fptemp, line);
	
		if (strcmp(line, level_start) == STRING_EQUAL){
			level_found = true;
			write_to_file(fptemp, char_newline);
			write_to_file(fptemp, STR_LEVEL_ACTIVE_TRUE);
			write_to_file(fptemp, char_newline);
			while (READ_ERROR != (read = getline(&line, &len, fp))){
				if (strcmp(line, STR_CODE_STARTS) == STRING_EQUAL){
					write_to_file(fptemp, STR_CODE_STARTS);
					write_player_code_to_file(fptemp);
				}
				if (strcmp(line, STR_CODE_ENDS) == STRING_EQUAL){
					write_to_file(fptemp, STR_CODE_ENDS);
					break;
					
				}
			}
		} 
	}
	free(level_start);
	free(level_end);
	fclose(fp);
	fclose(fptemp);

	copy_file(SAVE_FILE_PATH,SAVE_FILE_PATH_TEMP);
	delete_file(SAVE_FILE_PATH_TEMP);
	error:
	return;
}

/* Function: fl_enable_next_level
 *------------------------------------------------------------------------------
 * When a player finishes a level, this function puts the level flag as 
 * active fot eh next player. 
 *
 * Arguments:
 *  player_id: The id of the player that is going to be changed.
 *	level: The level number that will be looked in the text file.
 *
 * Return:
 *	void.
 *
 */
void fl_enable_next_level(int player_id, int level_id)
{
	assert(level_id > LV_LEVEL_MIN && level_id < LV_LEVEL_MAX && 
		   "Invalid level");
	assert(player_id >= FL_PLAYER_1 && player_id <= FL_PLAYER_3 && 
		   "player");
	
	FILE *fp = fopen(SAVE_FILE_PATH, "r");
	FILE *fptemp = fopen(SAVE_FILE_PATH_TEMP, "w");
	check_mem(fp);
	check_mem(fptemp);

	char *line = NULL;	
	size_t len = 0;
	ssize_t read;

	char *player = get_player_id_string(player_id);
	char *level_start = get_delimeter_level_string(STR_LEVEL_STARTS, level_id);
	char *level_end = get_delimeter_level_string(STR_LEVEL_ENDS, level_id);
	bool level_found = false;
	bool player_found = false;

	while (READ_ERROR != (read = getline(&line, &len, fp))){
		write_to_file(fptemp, line);
	
		if (strcmp(line, level_start) == STRING_EQUAL){
			level_found = true;
			write_to_file(fptemp, char_newline);
			write_to_file(fptemp, STR_LEVEL_ACTIVE_TRUE);
			write_to_file(fptemp, char_newline);
			while (READ_ERROR != (read = getline(&line, &len, fp))){
				if (strcmp(line, STR_CODE_STARTS) == STRING_EQUAL){
					write_to_file(fptemp, STR_CODE_STARTS);
					break;
				}
			}
		}
	} 
	free(level_start);
	free(level_end);
	fclose(fp);
	fclose(fptemp);

	copy_file(SAVE_FILE_PATH,SAVE_FILE_PATH_TEMP);
	delete_file(SAVE_FILE_PATH_TEMP);
	error:
	return;
}

/* Function: write_to_file
 *------------------------------------------------------------------------------
 * This functions compreses the write to file in order to present it on a
 * more readable way
 *
 * Arguments:
 *  string: the string pointer that is going to be write it to the file pointer
 *	fp: file pointer that is going to be written.
 *
 * Return:
 *	void.
 *
 */
void write_to_file(FILE *fp, char *string)
{
	if (fputs(string, fp) == EOF){
		perror("Error writin to the file");
		fclose(fp);
	}
}

/* Function: fl_save_file_init
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
void fl_save_file_init()
{
	bool file_exists = check_if_save_file_exists();

	if (file_exists == false){
		FILE *fp = fopen(SAVE_FILE_PATH, "w");
		char *level = malloc(sizeof(char)*SAVE_FILE_LINE_LENGTH);
		check_mem(level);

		for (int j = 1; j <= PLAYER_QUANTITY; j++){
			char *player_number = ax_number_to_string_prepend_zero(j);

			strcpy(level, STR_PLAYER); 
			strcat(level, " ");
			strcat(level, player_number);
			strcat(level, char_newline);
			write_to_file(fp, level);
			strcpy(level, char_newline);
			write_to_file(fp, level);
			for (int i = 1; i <= LV_LEVEL_QUANTITY; i++){
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
				strcat(level, char_newline);
				write_to_file(fp, level);
				strcpy(level, char_newline);
				write_to_file(fp, level);

				if (i == 1){
					strcpy(level, STR_LEVEL_ACTIVE_TRUE);
				}else {
					strcpy(level, STR_LEVEL_ACTIVE_FALSE);
				}

				write_to_file(fp, level);
				
				strcpy(level, char_newline);
				write_to_file(fp, level);
				
				strcpy(level, STR_CODE_STARTS);
				write_to_file(fp, level);
				
				if (i == 2){
					strcpy(level, STR_L2_CODE_1);
					write_to_file(fp, level);
					strcpy(level, STR_L2_CODE_2);
					write_to_file(fp, level);
					strcpy(level, STR_L2_CODE_3);
					write_to_file(fp, level);

				} else if (i == 9){
					strcpy(level, STR_L9_CODE_1);
					write_to_file(fp, level);
					strcpy(level, STR_L9_CODE_2);
					write_to_file(fp, level);

				} else {
					strcpy(level, char_newline);
					write_to_file(fp, level);
				}
				
				strcpy(level, STR_CODE_ENDS);
				write_to_file(fp, level);
				
				strcpy(level, char_newline);
				write_to_file(fp, level);

				strcpy(level, STR_LEVEL_ENDS);
				strcat(level, " ");
				strcat(level, number);
				strcat(level, char_newline);
				write_to_file(fp, level);
				strcpy(level, char_newline);
				write_to_file(fp, level);
				free(number);
			}
			strcpy(level, STR_PLAYER_ENDS); 
			strcat(level, " ");
			strcat(level, player_number);
			strcat(level, char_newline);
			write_to_file(fp, level);
			strcpy(level, char_newline);
			write_to_file(fp, level);

			free(player_number);
		}
		free(level);
		fclose(fp);
	}
	error:
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
	bool file_exists = (access(SAVE_FILE_PATH, F_OK) == 0);
	return file_exists;
}
