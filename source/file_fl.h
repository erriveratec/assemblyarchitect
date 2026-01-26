#ifndef FILE_FL_H
#define FILE_FL_H

#define PLAYER_QUANTITY 3

// Level specific instructions
// ----- LEVEL 2 ----
#define FL_L2_CODE_1 "MOV [ob], rax\n"
#define FL_L2_CODE_2 "MOV rax, rax\n"
#define FL_L2_CODE_3 "MOV rax, rax\n"

// ----- LEVEL 9 ----
#define FL_L9_CODE_1 "MOV rax, [ib]\n"
#define FL_L9_CODE_2 "MOV [ob], rax\n"


enum fl_player{FL_NO_PLAYER, FL_PLAYER_1, FL_PLAYER_2, FL_PLAYER_3};

void fl_file_initialize_level(int level);
void fl_save_file_init();
void fl_save_level(int player_id, int level_id);
void fl_load_save_file(int player_id, int level_id);
void fl_enable_next_level(int player_id, int level_id);
void fl_load_player_levels(int player_id, bool *levels_array);
void fl_load_level_msgs(int level_id);
#endif
