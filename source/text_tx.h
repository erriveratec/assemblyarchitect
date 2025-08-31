#ifndef TEXT_TX_H
#define TEXT_TX_H
#include<stdio.h>
#include<stdlib.h>
#include"dimensions_dm.h"
#include"draw_dw.h"


void tx_text_box(int pos, char *msg);
void tx_update_assets();
int tx_get_text_box_member(int text_box_id, int member);
void tx_create_level_text_texture(int level_id);


enum text_box_positions{
	TX_BOX_MIN,
	TX_INS_BOX,
	TX_UPPER_BOX,
	TX_LOWER_BOX,
	TX_CODE_BOX,
	TX_SB_BOX,
	TX_BIG_BOX,
	TX_ERROR_BOX,
	TX_BOX_MAX
};

#define TUT_TEXT_X INS_BOX_X
#define TUT_TEXT_Y SCREEN_HEIGHT/2 - MSG_BOX_H

#define MSG_TEXT_H 30

#define MSG_BOX_W (INS_BOX_W + 100)
#define MSG_BOX_H 210
#define MSG_INS_BOX_X INS_BOX_X
#define MSG_INS_BOX_Y (SCREEN_HEIGHT/2 - MSG_BOX_H/3)

#define BIG_MSG_BOX_W (SCREEN_WIDTH/3)
#define BIG_MSG_BOX_H (SCREEN_HEIGHT/2)
#define BIG_MSG_BOX_X (SCREEN_WIDTH/2 - BIG_MSG_BOX_H/2)
#define BIG_MSG_BOX_Y (SCREEN_HEIGHT/4)

#define BIG_MSG_TEXT_H 50
#define ERROR_BOX_Y_OFFSET 60
#define ERROR_MSG_BOX_X RES_BOX_X + MSG_BOX_W/2 - ARROW_W/2
#define ERROR_MSG_BOX_Y RES_BOX_Y + RES_BOX_H + ERROR_BOX_Y_OFFSET

#define CLICK_MESSAGE_H 30

#define MSG_CLICK_ANYWHERE "Click anywhere to continue"

#define L1_MSG_FIRST_CHALLENGE "Welcome to Level 01"

#define L2_MSG_SECOND_CHALLENGE "Welcome to Level 02. In this level we will"\
" learn how to delete instructions, change operands and rearrange, their"\
" position. (Click anywhere to continue)."

#define L3_MSG_THIRD_CHALLENGE "Welcome to Level 03. In this level we will learn"\
" the behaviour of values when retrieved from the Input Buffer [IB] and"\
" registers. (Click anywhere to continue)."

#define L4_MSG_FOURTH_CHALLENGE "Welcome to Level 04. In this level you "\
" solve the challenge by yourself applying what has been learned."\
" (Click anywhere to continue)."

#define L5_MSG_FIFTH_CHALLENGE "Welcome to Level 05. Notice that now are more"\
" registers available. You will need them to solve the challenge."\
" (Click anywhere to continue)."

#define L6_MSG_SIXTH_CHALLENGE "Welcome to Level 06. Notice that there is"\
" a new instruction available \"add\". "\
" (Click anywhere to continue)."

#define L7_MSG_SEVENTH_CHALLENGE "Welcome to Level 07. Did you know that"\
" instructions \"mov\" \"add\" can be used with both operands being registers?"\
" (Click anywhere to continue)."

#define L8_MSG_EIGHTH_CHALLENGE "You are now in Level 08. I have new info for"\
" you: Both operands of instructions \"mov\" \"add\" can be the same register."\
" (Click anywhere to continue)."

#define L9_MSG_NINTH_CHALLENGE "You have arrive to level 09. There is a new"\
" instruction: \"jmp\", which jumps to a place pointed by a LABEL"\
" (Click anywhere to continue)."

#define L1_MSG_CHALLENGE_DESCRIPTION "First, read the challenge description."\
" (Click anywhere to continue)."
#define L1_MSG_SEL_INS2 "Select and drag another \"mov\" instruction from the"\
" instruction box."

#define L1_MSG_DROP_INS "Drop the instruction in the code box"\
" below the previous instruction."

#define L1_MSG_SEL_OP1 "The destiny operand is where the"\
" recovered value will be stored."\
" Select \"rax\" as the destiny operand."

#define L1_MSG_SEL_OP2_1 "The second operand is from where the value will be"\
" recovered. Select the Input Buffer [IB] as the source"\
" operand."

#define L1_MSG_SEL_OP1_2 "Now we need to move the value from the \"rax\""\
" register to the Output Buffer [OB], select the the Output Buffer"\
" as the destiny operand."

#define L1_MSG_SEL_OP2_2 "Select \"rax\" as the source operand, we want to"\
" move the value from \"rax\" to the Output Buffer [OB]."

#define L1_MSG_PLAY_TUT "Let's see what that instruction does. Press the play"\
" button."

#define L1_MSG_CONGRATS "Congratulations, you have beaten the first level."\
" Press the continue button."

#define L1_MSG_ERROR "The value was recovered from the Input Buffer [IB]."\
" Now, we will move it to the Output Buffer [OB]"\
" Press the back button to continue."

#define L2_MSG_SEL_LAST_INS "Select and drag the last instruction"\
" by clicking in the instruction name."	

#define L2_MSG_DEL_INS "Drag the instruction out of the code"\
" to delete it."

#define L2_MSG_CHANGE_OP "Change any operand by clicking on it. Click"\
" the source operand of the instruction."

#define L2_MSG_SEL_IB "Select the Input Buffer [IB] as the source operand."

#define L2_MSG_MOV_INS "Select and drag the second instruction and move in to"\
" the first position."

#define L3_MSG_AVAIL_OPS_1 "When selecting an instruction operand the"\
" available operands will be pointed with"\
" an arrow. Select \"rax\"."

#define L3_MSG_AVAIL_OPS_2 "In this case there are two available operands for"\
" selection, to complete the challenge, select Input Buffer [IB]."

#define L3_MSG_RECOVERED "Values stored on a register (like \"rax\")"\
" can be read several times and the value will countinue"\
" in the register."

#define L3_MSG_READ_RAX "Try using the instruction:      mov [OB], rax"\
" several times in a row to complete the challenge." 

#define L5_MSG_NEW_REGS "Notice that now are more registers available"\
" try using them to solve the challenge."

#define L6_MSG_ADD "For Example:        \"add OP1, OP2\" adds OP1 + OP2"\
" and stores the result in OP1. (Click anywhere to continue)."

#define L6_MSG_SOLVE "Solve the challenge using a combination of the"\
" \"mov\" and \"add\" instructions."\

#define L7_MSG_ADD "For Example:        \"add rax, rbx\" adds rax + rbx"\
" and stores the result in rax. (Click anywhere to continue)."

#define L8_MSG_ADD "For Example:        \"add rax, rax\" adds rax + rax"\
" and stores the result in rax. (Click anywhere to continue)."

#define L9_MSG_NEW_INS_JMP "The operand of the \"jmp\" instruction is a "\
" LABEL that must be defined in the code."\
" (Click anywhere to continue)."

#define L9_MSG_RESTRICTION "Restriction: the maximum"\
" number of instructions that can be used to solve de challenge is 4."\
" (Click anywhere to continue)."

#define L9_MSG_SELECT_LABEL "Select an drag a LABEL from the instruction box."

#define MSG_OBJECTIVE1 "The objective of the challenge is to move items from"\
" the Input Buffer [IB]. (Click anywhere to continue)."

#define MSG_OBJECTIVE2 "To the Output Buffer [OB]. (Click anywhere to"\
" continue)."

#define MSG_OBJECTIVE3 "All values recovered from the Input Buffer [IB], must"\
" go to a register first, in this case \"rax\"."\
" (Click anywhere to continue)."

#define MSG_COMPAT_OPS "Notice: Not all operands are compatible with"\
" each other, for instance the"\
" instruction:  mov [ob], [ib]"\
" is not possible."

#define MSG_PRESS_PLAY	"Press the play button."

#define MSG_SEL_INS1 "Let's move the value from the Input Buffer [IB] to"\
" the register. Select and drag the \"mov\" instruction from"\
" the instruction box."

#define MSG_DROP_INS1 "Drop the instruction in the code box."



#endif
