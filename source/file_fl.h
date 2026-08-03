#ifndef FILE_FL_H
#define FILE_FL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define PLAYER_QUANTITY 3

// Level specific instructions
// ----- LEVEL 1 ----
#define FL_L1_CODE_1 "MOV [ob], rax\n"
#define FL_L1_CODE_2 "MOV rax, rax\n"
#define FL_L1_CODE_3 "MOV rax, rax\n"

// ----- LEVEL 9 ----
#define FL_L8_CODE_1 "MOV rax, [ib]\n"
#define FL_L8_CODE_2 "MOV [ob], rax\n"


#define STR_CODE_STARTS  "CODE STARTS\n"
#define STR_CODE_ENDS  "CODE ENDS\n"
#define STR_LEVEL_STARTS  "+++LEVEL STARTS"
#define STR_LEVEL_ENDS  "+++LEVEL ENDS"
#define STR_PLAYER  "PLAYER"
#define STR_LEVEL_ACTIVE_TRUE  "LEVEL ACTIVE TRUE\n"
#define STR_LEVEL_ACTIVE_FALSE  "LEVEL ACTIVE FALSE\n"



enum fl_player{FL_NO_PLAYER, FL_PLAYER_1, FL_PLAYER_2, FL_PLAYER_3};

void fl_file_initialize_level(int level);
void fl_save_level(int player_id, int level_id);
void fl_load_save_file(int player_id, int level_id);
void fl_enable_next_level(int player_id, int level_id);
void fl_load_level_msgs(int level_id);
void fl_load_hover_level_msgs();


void fl_write_to_file(FILE *fp, char *string);
char *fl_get_level_id_string(int level_id);
char *fl_get_player_id_string(int player_id);


#endif
