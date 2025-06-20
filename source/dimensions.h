#ifndef DIMENSIONS_H
#define DIMENSIONS_H

/*******************************RESOLUTION*************************************/
#define SCREEN_WIDTH 1366
#define SCREEN_HEIGHT 768

/*******************************STUDIO NAME SCREEN*****************************/
#define STUDIO_NAME_TEXT "One Man Studio"
#define STUDIO_NAME_X (SCREEN_WIDTH - STUDIO_NAME_W)/2
#define STUDIO_NAME_Y 300
#define STUDIO_NAME_W 600

/********************************GAME TITLE SCREEN*****************************/
#define GAME_TITLE_X (SCREEN_WIDTH - GAME_TITLE_W)/2
#define GAME_TITLE_Y 25
#define GAME_TITLE_W 1100
#define GAME_TITLE_TEXT "Computer Architecture Game"
#define PRESS_SPACE_TEXT "Press Space"

#define SPACE_TEXT_X (SCREEN_WIDTH - SPACE_TEXT_W)/2
#define SPACE_TEXT_Y 500
#define SPACE_TEXT_W 400
#define PRESS_SPACE_TEXT "Press Space"

/******************************SELECT PLAYER SCREEN****************************/
#define SELECT_PLAYER_TEXT_X 50
#define SELECT_PLAYER_TEXT_Y 25
#define SELECT_PLAYER_TEXT_H 90
#define SELECT_PLAYER_TEXT "Which hacker are you?"

#define PLAYER_1_TEXT "HACKER W"
#define PLAYER_2_TEXT "HACKER X"
#define PLAYER_3_TEXT "HACKER Y"

#define P_BUTTON_H 200
#define P_BUTTON_W 200

#define P1_BUTTON_X (SCREEN_WIDTH-3*P_BUTTON_W)/4 
#define P1_BUTTON_Y 300

#define P2_BUTTON_X 2*(SCREEN_WIDTH-3*P_BUTTON_W)/4 + P_BUTTON_W
#define P2_BUTTON_Y 300

#define P3_BUTTON_X 3*(SCREEN_WIDTH-3*P_BUTTON_W)/4 + 2*P_BUTTON_W
#define P3_BUTTON_Y 300

/*******************************INSTRUCTION BOX********************************/
// The offset of the borders of the instruction box
#define INS_BOX_OFFSET 25

// The instruction box where the player can select instructions
#define INS_BOX_X 0
#define INS_BOX_Y 75
#define INS_BOX_W 125
#define INS_BOX_H 250

/*******************************RESULT BOX*************************************/

#define RES_BOX_X 500
#define RES_BOX_Y 500
#define RES_BOX_W 200
#define RES_BOX_H 30

/***********************************CODE***************************************/

// The code box that contains level, description and developed code
#define CODE_BOX_X (INS_BOX_X + INS_BOX_W)
#define CODE_BOX_Y 50
#define CODE_BOX_W 350
#define CODE_BOX_H 625

#define CODE_BOX_NUMBER_WIDTH 35

#define CODE_LINES_SIZE 5
#define LAST_INSTRUCTION_DISTANCE 75

#define LINE_NUMBER_OFFSET 15
#define CODE_BOX_OFFSET 25
#define ADJUSTING_OFFSET 6

#define MAXIMUM_CODE_LINES 99

//#define TEXT_BOX_SCALE 0.20		//Should be deprecated in favor of pixels
#define TEXT_BOX_WIDTH 300
#define TEXT_BOX_HEIGHT 30

#define MOVEMENT_DELTA 30

#define OP1_X_OFFSET (CODE_BUTTON_W + LINE_NUMBER_OFFSET)
#define OP2_X_OFFSET (2*CODE_BUTTON_W + 2*LINE_NUMBER_OFFSET)

/*************************INPUT/OUTPUT BUFFERS*********************************/
// The height of the text for the input/output buffers
#define BUFFER_TEXT_H 42

// The height and width of the contents of the buffers

// The position of the text of the input/output buffers
#define INPUT_BUFFER_TEXT_X 900
#define INPUT_BUFFER_TEXT_Y 0
#define OUTPUT_BUFFER_TEXT_X 900
#define OUTPUT_BUFFER_TEXT_Y 720

// The dimensions of the input/output boxes
#define BUFFER_BOX_X 900
#define INPUT_BUFFER_BOX_Y 40
#define OUTPUT_BUFFER_BOX_Y 640
#define	BUFFER_BOX_W 950
#define BUFFER_BOX_H 75

// x and y offsets for the numbers and the box
#define BUFFER_VALUE_OFFSET_X 20
#define BUFFER_VALUE_OFFSET_Y 20
#define BETWEEN_NUMBERS_OFFSET 30

#define BUFFER_MOVEMENT_DELTA 5

/***************************REGISTER BOX***************************************/
// The offset of the borders of the register box
#define REG_BOX_OFFSET 25

// The height of the register text
#define REG_TEXT_H 40

// The position of the register text
#define REG_TEXT_X (CODE_BOX_X + CODE_BOX_W + REG_BOX_OFFSET)
#define REG_TEXT_Y 5

// The position of the registers box
#define REG_BOX_X (CODE_BOX_X + CODE_BOX_W)
#define REG_BOX_Y 0
#define REG_BOX_H 200

// The between registers offset
#define BETWEEN_REG_OFFSET 10

/*****************************CODE BUTTONS*************************************/
// Dimensions of the buttons that will be used in code
#define CODE_BUTTON_W 75
#define CODE_BUTTON_H 40

/*****************************SCALING FACTORS**********************************/
// Should be deprecater in favor of pixels
#define M_SCALE_FACTOR 0.3 
#define STAGE_NAME_SCALE 0.45

/******************************STAGE BUTTONS***********************************/

#define STAGE_BUTTON_X 550
#define STAGE_BUTTON_Y 700
#define STAGE_BUTTON_HIDDEN_Y 800
#define STAGE_BUTTON_W 40
#define STAGE_BUTTON_H 40
#define BUTTONS_SPACE 20

/******************************AVATAR******************************************/
#define AVATAR_START_X 500
#define AVATAR_START_Y 500
#define AVATAR_W 50
#define AVATAR_H 50
#define AVATAR_X_POS_OFFSET (AVATAR_W + 20)

/******************************VALUE BOXES*************************************/

//#define VALUE_BOX_H 56
//#define VALUE_BOX_W 64
#define VALUE_BOX_H 40
#define VALUE_BOX_W 48
#define VALUE_H 35
#define VALUE_W 50




#endif
