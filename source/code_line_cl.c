#include <assert.h>
#include <stdbool.h>
#include "code_line_cl.h"
#include "dbg.h"
#include "dimensions_dm.h"
#include "aux.h"

#define INSTRUCTION_STRING_LENGTH 20

static operand_t *copy_operand(operand_t *op);
bool check_valid_operand(int id);

char *add_text = "ADD";
char *mov_text = "MOV";
char *label_text = "line";
char *jmp_text = "JMP";
char *cmp_text = "CMP";
char *rax_text = "rax";
char *rbx_text = "rbx";
char *rcx_text = "rcx";
char *rdx_text = "rdx";
char *rdi_text = "rdi";
char *ib_text = "[ib]";
char *ob_text = "[ob]";
char *immup0_text = "IMMUP0";
char *immup1_text = "IMMUP1";
char *immup2_text = "IMMUP2";
char *immup3_text = "IMMUP3";
char *immup4_text = "IMMUP4";
char *immup5_text = "IMMUP5";
char *immup6_text = "IMMUP6";
char *immup7_text = "IMMUP7";
char *immup8_text = "IMMUP8";
char *immup9_text = "IMMUP9";
char *immup10_text = "IMMUP10";
char *immup_1_text = "IMMUP_1";
char *immup_2_text = "IMMUP_2";
char *immup_3_text = "IMMUP_3";
char *immup_4_text = "IMMUP_4";
char *immup_5_text = "IMMUP_5";
char *immup_6_text = "IMMUP_6";
char *immup_7_text = "IMMUP_7";
char *immup_8_text = "IMMUP_8";
char *immup_9_text = "IMMUP_9";
char *immdo0_text = "IMMDO0";
char *immdo1_text = "IMMDO1";
char *immdo2_text = "IMMDO2";
char *immdo3_text = "IMMDO3";
char *immdo4_text = "IMMDO4";
char *immdo5_text = "IMMDO5";
char *immdo6_text = "IMMDO6";
char *immdo7_text = "IMMDO7";
char *immdo8_text = "IMMDO8";
char *immdo9_text = "IMMDO9";
char *invalid_op_text = "INVALID_OP";

/* Function: cl_is_op_imm
 * -----------------------------------------------------------------------------
 * Verifies using the id of the operand if corresponds to a register
 *
 * Arguments:
 * 	op_id: Te id of the operand that will be verified
 *	
 * Return:
 *	true if the id of the operand corresponds to a register
 *
 */
bool cl_is_op_imm(int op_id)
{
	assert(((op_id > REG_MIN && op_id < REG_MAX) 
			|| (op_id > MEMORY_MIN && op_id < MEMORY_MAX) 
			|| (op_id > BUF_MIN && op_id < BUF_MAX) 
			|| (op_id > RGBOX_MIN && op_id < RGBOX_MAX) 
			|| (op_id > IMM_MIN && op_id < IMM_MAX))
			&& "Invalid operand id");
	
	bool is_imm = false;	
	
	if (op_id > IMM_MIN && op_id < IMM_MAX){
		is_imm = true;		
	}
	return is_imm;
}

/* Function: cl_is_op_reg
 * -----------------------------------------------------------------------------
 * Verifies using the id of the operand if corresponds to a register
 *
 * Arguments:
 * 	op_id: Te id of the operand that will be verified
 *	
 * Return:
 *	true if the id of the operand corresponds to a register
 *
 */
bool cl_is_op_reg(int op_id)
{
	assert(((op_id > REG_MIN && op_id < REG_MAX) 
			|| (op_id > MEMORY_MIN && op_id < MEMORY_MAX) 
			|| (op_id > BUF_MIN && op_id < BUF_MAX) 
			|| (op_id > RGBOX_MIN && op_id < RGBOX_MAX) 
			|| (op_id > IMM_MIN && op_id < IMM_MAX))
			&& "Invalid operand id");
	
	bool is_register = false;	
	
	if (op_id > REG_MIN && op_id < REG_MAX){
		is_register = true;		
	}
	return is_register;
}

/* Function: cl_check_both_operands_are_registers
 * -----------------------------------------------------------------------------
 * Determines if the operands of a given line, both are registers.
 *
 * Arguments:
 * 	line: the line object that will be analized
 *	
 * Return:
 *	true if both operands are registers
 *
 */
bool cl_operands_are_registers(code_line_t *line)
{
	bool op1 = false;
	bool op2 = false;

	if (line->op1->id > REG_MIN && line->op1->id < REG_MAX){
		op1 = true;
	}
	if (line->op2->id > REG_MIN && line->op2->id < REG_MAX){
		op2 = true;
	} 
	return op1 & op2;
}



/* Function: cl_new_code_line
 * -----------------------------------------------------------------------------
 * This function generates a new code_line object of a pointer it creates a 
 * different pointers that can be used on a list. It is used in the instruction
 * window. Used when selecting an instruction from the IW and generating a new
 * line based in which instruction the player selected
 *
 * Arguments:
 * 	instruction: the instruction of the new line that will be created
 *	
 * Return:
 *	The new code_line_object
 *
 */
code_line_t *cl_new_code_line(instruction_t *ins)
{
	texture_t *t = cl_create_instruction_texture(ins->id);	
	btn_t *b = bt_create_btn(ins->b->r, t);

	instruction_t *i = cl_create_instruction(ins->id, b);
	
	code_line_t *new = cl_create_code_line(i);

	return new;
}

/* Function: cl_create_instruction_texture
 * -----------------------------------------------------------------------------
 * Arguments:
 * 	id: The id of the texture that must be returned
 *	
 * Return:
 *	void
 *
 */
texture_t *cl_create_instruction_texture(int id)
{
	assert(id > INSTRUCTION_MIN && id < INSTRUCTION_MAX &&  "Incorrect id");
	texture_t *texture = NULL;
	
	switch(id){
		case MOV:
			texture = dw_create_text_texture(mov_text, C_WHITE);
			break;
		case ADD:
			texture = dw_create_text_texture(add_text, C_WHITE);
			break;
		case LABEL:
			texture = dw_create_text_texture(label_text, C_WHITE);
			break;
		case JMP:
			texture = dw_create_text_texture(jmp_text, C_WHITE);
			break;
		case CMP:
			texture = dw_create_text_texture(cmp_text, C_WHITE);
			break;
		default:
			printf("Error: the id of the register is invalid");
				
	}
	return texture;
}

/* Function: cl_create_operand_texture
 * -----------------------------------------------------------------------------
 * Arguments:
 * 	id: The id of the texture that must be returned
 *	
 * Return:
 *	void
 *
 */
texture_t *cl_create_operand_texture(int id)
{
	assert(((id > REG_MIN && id < REG_MAX) ||
			(id > BUF_MIN && id < BUF_MAX)) &&  "Incorrect id");
	texture_t *texture = NULL;
	
	switch(id){
		case RAX:
			texture = dw_create_text_texture(rax_text, C_WHITE);
			break;
		case RBX:
			texture = dw_create_text_texture(rbx_text, C_WHITE);
			break;
		case RCX:
			texture = dw_create_text_texture(rcx_text, C_WHITE);
			break;
		case RDX:
			texture = dw_create_text_texture(rdx_text, C_WHITE);
			break;
		case RDI:
			texture = dw_create_text_texture(rdi_text, C_WHITE);
			break;
		case IB:
			texture = dw_create_text_texture(ib_text, C_WHITE);
			break;
		case OB:
			texture = dw_create_text_texture(ob_text, C_WHITE);
			break;
		default:
			printf("Error: the id of the register is invalid");
				
	}
	return texture;
}

/* Function: cl_text_to_operand_id
 *------------------------------------------------------------------------------
 * If operand can be first, should us str str
 * Arguments:
 * 	text: the text of the instruction that will be converted
 *	
 * Return:
 *	instruction_id: The numerical identifier of the instruction.
 */
int cl_text_to_operand_id(char *text)
{
	int operand_id = INVALID_OPERAND;
	if (strstr(text, rax_text) != NULL){
		operand_id = RAX;
	} else if (strstr(text, rbx_text)!= NULL){
		operand_id = RBX;
	} else if (strstr(text, rcx_text)!= NULL){
		operand_id = RCX;
	} else if (strstr(text, rdx_text)!= NULL){
		operand_id = RDX;
	} else if (strstr(text, rdi_text)!= NULL){
		operand_id = RDI;
	} else if (strstr(text, ib_text)!= NULL){
		operand_id = IB;
	} else if (strstr(text, ob_text)!= NULL){
		operand_id = OB;
	} else if (strcmp(text, immup0_text) == STRING_EQUAL){
		operand_id = IMMUP0;
	} else if (strcmp(text, immup1_text) == STRING_EQUAL){
		operand_id = IMMUP1;
	} else if (strcmp(text, immup2_text) == STRING_EQUAL){
		operand_id = IMMUP2;
	} else if (strcmp(text, immup3_text) == STRING_EQUAL){
		operand_id = IMMUP3;
	} else if (strcmp(text, immup4_text) == STRING_EQUAL){
		operand_id = IMMUP4;
	} else if (strcmp(text, immup5_text) == STRING_EQUAL){
		operand_id = IMMUP5;
	} else if (strcmp(text, immup6_text) == STRING_EQUAL){
		operand_id = IMMUP6;
	} else if (strcmp(text, immup7_text) == STRING_EQUAL){
		operand_id = IMMUP7;
	} else if (strcmp(text, immup8_text) == STRING_EQUAL){
		operand_id = IMMUP8;
	} else if (strcmp(text, immup9_text) == STRING_EQUAL){
		operand_id = IMMUP9;
	} else if (strcmp(text, immup10_text) == STRING_EQUAL){
		operand_id = IMMUP10;
	} else if (strcmp(text, immup_1_text) == STRING_EQUAL){
		operand_id = IMMUP_1;
	} else if (strcmp(text, immup_2_text) == STRING_EQUAL){
		operand_id = IMMUP_2;
	} else if (strcmp(text, immup_3_text) == STRING_EQUAL){
		operand_id = IMMUP_3;
	} else if (strcmp(text, immup_4_text) == STRING_EQUAL){
		operand_id = IMMUP_4;
	} else if (strcmp(text, immup_5_text) == STRING_EQUAL){
		operand_id = IMMUP_5;
	} else if (strcmp(text, immup_6_text) == STRING_EQUAL){
		operand_id = IMMUP_6;
	} else if (strcmp(text, immup_7_text) == STRING_EQUAL){
		operand_id = IMMUP_7;
	} else if (strcmp(text, immup_8_text) == STRING_EQUAL){
		operand_id = IMMUP_8;
	} else if (strcmp(text, immup_9_text) == STRING_EQUAL){
		operand_id = IMMUP_9;
	} else if (strcmp(text, immdo0_text) == STRING_EQUAL){
		operand_id = IMMDO0;
	} else if (strcmp(text, immdo1_text) == STRING_EQUAL){
		operand_id = IMMDO1;
	} else if (strcmp(text, immdo2_text) == STRING_EQUAL){
		operand_id = IMMDO2;
	} else if (strcmp(text, immdo3_text) == STRING_EQUAL){
		operand_id = IMMDO3;
	} else if (strcmp(text, immdo4_text) == STRING_EQUAL){
		operand_id = IMMDO4;
	} else if (strcmp(text, immdo5_text) == STRING_EQUAL){
		operand_id = IMMDO5;
	} else if (strcmp(text, immdo6_text) == STRING_EQUAL){
		operand_id = IMMDO6;
	} else if (strcmp(text, immdo7_text) == STRING_EQUAL){
		operand_id = IMMDO7;
	} else if (strcmp(text, immdo8_text) == STRING_EQUAL){
		operand_id = IMMDO8;
	} else if (strcmp(text, immdo9_text) == STRING_EQUAL){
		operand_id = IMMDO9;
	}

	assert(operand_id != INVALID_OPERAND && "Operand id is invalid");
	return operand_id;
}

/* Function: cl_text_to_instruction_id
 *------------------------------------------------------------------------------
 * Arguments:
 * 	text: the text of the instruction that will be converted
 *	
 * Return:
 *	instruction_id: The numerical identifier of the instruction.
 */
int cl_text_to_instruction_id(char *text)
{
	int instruction_id = INVALID_OPERAND;

	if (strstr(text, mov_text) != NULL){
		instruction_id = MOV;
	} else if (strstr(text, add_text)!= NULL){
		instruction_id = ADD;
	} else if (strstr(text, label_text)!= NULL){
		instruction_id = LABEL;
	} else if (strstr(text, jmp_text)!= NULL){
		instruction_id = JMP;
	}

	assert(instruction_id != INVALID_OPERAND && "Instruction id is invalid");
	return instruction_id;
}

/* Function: print_code_line
 *------------------------------------------------------------------------------
 * Arguments:
 * 	line: the line that will be printed
 *	
 * Return:
 *	void.
 */
void print_code_line(code_line_t *line)
{
	char *ins_text = cl_get_instruction_text(line->ins->id);
	char *op1 = get_operand_text(line->op1->id);
	char *op2 = get_operand_text(line->op2->id);

	printf("%s %s, %s\n", ins_text, op1, op2);

}

/* Function: get_instruction_operand_quantity
 *------------------------------------------------------------------------------
 * Arguments:
 * 	instruction: the instruction that will be checked
 *	
 * Return:
 *	int with the number of operands of the instruction
 */
int cl_get_instruction_operand_quantity(int instruction_id)
{
	assert(instruction_id > INSTRUCTION_MIN && 
	       instruction_id < INSTRUCTION_MAX && "The instruction id is invalid");

	int number_of_operands = ZERO_OPERANDS;
	switch (instruction_id){
		case MOV:
			number_of_operands = TWO_OPERANDS;
			break;
		case ADD:
			number_of_operands = TWO_OPERANDS;
			break;
		case LABEL:
			number_of_operands = ONE_OPERAND;
			break;
		case JMP:
			number_of_operands = ONE_OPERAND;
			break;
		case CMP:
			number_of_operands = TWO_OPERANDS;
			break;
		default:
			number_of_operands = ZERO_OPERANDS;
			break;


	}
	return number_of_operands;
}

/* Function: cl_get_instruction_text
 *------------------------------------------------------------------------------
 * Arguments:
 * 	instruction_id: the id of the instruction
 *	
 * Return:
 *	char pointer with the name of the instruction
 */
char *cl_get_instruction_text(int instruction_id)
{
	assert(instruction_id > INSTRUCTION_MIN && 
	       instruction_id < INSTRUCTION_MAX && "The instruction id is invalid");

	char *text;

	switch (instruction_id){
		case MOV:
			text = mov_text;
			break;
		case ADD:
			text = add_text;
			break;
		case LABEL:
			text = label_text;
			break;
		case JMP:
			text = jmp_text;
			break;
		case CMP:
			text = cmp_text;
			break;
		default: 
			text = "INVALID";
	}
error:
	return text;
}

/* Function: get_operand_text
 *------------------------------------------------------------------------------
 * Arguments:
 * 	instruction_id: the id of the operand
 *	
 * Return:
 *	char pointer with the name of the instruction
 */
char *get_operand_text(int operand_id)
{
	assert(operand_id > REG_MIN && operand_id < IMM_MAX && 
		   "The operand id is invalid");

	char *text;

	switch (operand_id){
		case RAX:
			text = rax_text;
			break;
		case RBX:
			text = rbx_text;
			break;
		case RCX:
			text = rcx_text;
			break;
		case RDX:
			text = rdx_text;
			break;
		case RDI:
			text = rdi_text;
			break;
		case IB:
			text = ib_text;
			break;
		case OB:
			text = ob_text;
			break;
		case IMMUP0:
			text = immup0_text;
			break;
		case IMMUP1:
			text = immup1_text;
			break;
		case IMMUP2:
			text = immup2_text;
			break;
		case IMMUP3:
			text = immup3_text;
			break;
		case IMMUP4:
			text = immup4_text;
			break;
		case IMMUP5:
			text = immup5_text;
			break;
		case IMMUP6:
			text = immup6_text;
			break;
		case IMMUP7:
			text = immup7_text;
			break;
		case IMMUP8:
			text = immup8_text;
			break;
		case IMMUP9:
			text = immup9_text;
			break;
		case IMMUP10:
			text = immup10_text;
			break;
		case IMMUP_1:
			text = immup_1_text;
			break;
		case IMMUP_2:
			text = immup_2_text;
			break;
		case IMMUP_3:
			text = immup_3_text;
			break;
		case IMMUP_4:
			text = immup_4_text;
			break;
		case IMMUP_5:
			text = immup_5_text;
			break;
		case IMMUP_6:
			text = immup_6_text;
			break;
		case IMMUP_7:
			text = immup_7_text;
			break;
		case IMMUP_8:
			text = immup_8_text;
			break;
		case IMMUP_9:
			text = immup_9_text;
			break;
		case IMMDO0:
			text = immdo0_text;
			break;
		case IMMDO1:
			text = immdo1_text;
			break;
		case IMMDO2:
			text = immdo2_text;
			break;
		case IMMDO3:
			text = immdo3_text;
			break;
		case IMMDO4:
			text = immdo4_text;
			break;
		case IMMDO5:
			text = immdo5_text;
			break;
		case IMMDO6:
			text = immdo6_text;
			break;
		case IMMDO7:
			text = immdo7_text;
			break;
		case IMMDO8:
			text = immdo8_text;
			break;
		case IMMDO9:
			text = immdo9_text;
			break;
		default:
			text = invalid_op_text;
	}
error:
	return text;
}

/* Function: cl_create_code_line_text
 * -----------------------------------------------------------------------------
 * This function generates the string of a code line, the string will be single
 * and it will include the whole instruction in a single line
 *
 * Arguments:
 * 	instruction_id: The number id of the instruction
 * 	op1_id: The number id of the operand 1
 * 	op2_id: The number id of the operand 2
 *
 * Return:
 *	Pointer to the created string
 */
char *cl_create_code_line_text(int instruction_id, int op1_id, int op2_id)
{
	assert(instruction_id > INSTRUCTION_MIN 
		   && instruction_id < INSTRUCTION_MAX
		   && "The instruction id is invalid");
	
	if (instruction_id == JMP || instruction_id == LABEL){
		assert(op1_id >=0 && "Label destitny is negative");
	} else {
		assert(op1_id >= NO_OPERAND && op1_id < IMM_MAX && "Invalid OP1");
		assert(op2_id >= NO_OPERAND && op2_id < IMM_MAX && "Invalid OP2");
	}

	char *line_text = malloc(sizeof(char)*INSTRUCTION_STRING_LENGTH);
	char *instruction = cl_get_instruction_text(instruction_id);
	strcpy(line_text, instruction);
	
	if (instruction_id == LABEL || instruction_id == JMP) {
		char *op1 = NULL;
		op1 = ax_number_to_string_two_digits(op1_id);
		strcat(line_text, ax_char_space);
		strcat(line_text, op1);
	} else if (op1_id != NO_OPERAND){
		char *op1 = NULL;
		op1 = get_operand_text(op1_id);
		strcat(line_text, ax_char_space);
		strcat(line_text, op1);
	}
	if (op2_id != NO_OPERAND){
		char *op2 = get_operand_text(op2_id);
		strcat(line_text, ax_char_comma);
		strcat(line_text, ax_char_space);
		strcat(line_text, op2);
	}
	strcat(line_text, ax_char_newline);
	return line_text;
}


/* Function: cl_create_instruction
 * -----------------------------------------------------------------------------
 * This function creates a instruction object.
 *
 * Arguments:
 * 	id: the id of the instruction that will be created.
 * 	b: the button object of the instruction.
 *
 * Return:
 *	Pointer to the created code node
 */
instruction_t *cl_create_instruction(int id, btn_t *b)
{
	assert(NULL != b && "The button pointer is NULL");
	assert(id > INSTRUCTION_MIN && id < INSTRUCTION_MAX && 
		   "Invalid instruction id");
	
	instruction_t *ins = malloc(sizeof(instruction_t));	

	ins->b = b;
	ins->id = id;
	return ins;
}


/* Function: cl_create_code_line
 * -----------------------------------------------------------------------------
 * This function creates a code line with a given instruction object, the other
 * elements of the instruction are filled with defaults. The ins object should
 * be already created
 *
 * Arguments:
 * 	ins: the button object of the instruction to be added to the new code line.
 *
 * Return:
 *	Pointer to the created code line
 */
code_line_t *cl_create_code_line(instruction_t *ins)
{
	assert(NULL != ins && "The instruction pointer is NULL");
	
	code_line_t *new_line = malloc(sizeof(code_line_t));	
	check_mem(new_line);

	new_line->ins = ins;
	new_line->op1 = NULL;
	new_line->op2 = NULL;

	if (cl_get_instruction_operand_quantity(ins->id) == ZERO_OPERANDS){
		new_line->state = COMPLETE;
	} else if (cl_get_instruction_operand_quantity(ins->id) == ONE_OPERAND){
		new_line->state = MISSING_OP1;
	} else if (cl_get_instruction_operand_quantity(ins->id) == TWO_OPERANDS){
		new_line->state = MISSING_BOTH;
	} else{
		assert("Invalid instruction operand quantity");
	}

error:
	return new_line;
}

/* Function: cl_destroy_code_line
 * -----------------------------------------------------------------------------
 * This function frees and destroys a code line object 
 *
 * Arguments:
 * 	code_line: the the code line object to be destroyed
 *	
 * Return:
 *	Void
 *
 */
void cl_destroy_code_line(code_line_t *line)
{
	bt_destroy_button(line->ins->b);
	free(line->ins);

	if (line->op1 != NULL){
		bt_destroy_button(line->op1->b);
		free(line->op1);
	}
	if (line->op2 != NULL){
		bt_destroy_button(line->op2->b);
		free(line->op2);
	}
	free(line);
}



/* Function: copy_operand
 * -----------------------------------------------------------------------------
 * This function creates a new operand object by copying the button object and
 * the id. The pointer of the value is copied as well in order to maintain
 * consistency.
 *
 * Arguments:
 *	op: the operand that will be copied.
 *	
 * Return:
 *	object to the new operand that is a copy.
 *
 */
static operand_t *copy_operand(operand_t *op)
{
	assert(NULL != op && "The operand pointer cannot be NULL");
	btn_t *b = bt_copy_button(op->b);
	check_mem(b);

	operand_t *new_op = malloc(sizeof(operand_t));
	check_mem(new_op);
	new_op->b = b;
	new_op->id = op->id;
	
error:
	return new_op;
}

/* Function: check_valid_operand
 * -----------------------------------------------------------------------------
 * This function verifies is an operand id is valid
 *
 * Arguments:
 *  id: The operand id that will be verified.
 *	
 * Return:
 *	void.
 *
 */
bool check_valid_operand(int id)
{
	bool valid = false;

	if (id > REG_MIN && id < REG_MAX 
		|| id > MEMORY_MIN && id < MEMORY_MAX 
		|| id > BUF_MIN && id < BUF_MAX
		|| id > IMM_MIN && id < IMM_MAX){
		valid = true;
	}
	return valid;
}


/* Function: check_operand_compatibility
 * -----------------------------------------------------------------------------
 * This function verifies if when changing or assigning an operand, it e
 *
 * Arguments:
 *	operand: the operand that will be assigned.
 * 	line: the line where the operand will be assigned.
 *	
 * Return:
 *	void.
 *
 */
bool cl_is_op_compatible(operand_t *op, code_line_t *line)
{
	assert(op != NULL && "The operand cannot be NULL");
	assert(line != NULL && "The line cannot be NULL");
	assert(check_valid_operand(op->id) == true && 
		   "The operand value is not valid");

	if (cl_is_op_reg(op->id) == true){
		return true;
	}

	bool is_op_imm = cl_is_op_imm(op->id);
	bool compatible = false;

	switch(line->state){
		case MISSING_BOTH:
			if (op->id != IB){
				if (op->id == OB && line->ins->id == MOV){//Evaluar cambiar
					compatible = true;
				}
			}
			break;
		case MISSING_OP1:
			if (cl_is_op_reg(line->op2->id) == true 
				&& op->id != IB
				&& is_op_imm == false){
				compatible = true;
			}
			break;
		case MISSING_OP2:
			if (cl_is_op_reg(line->op1->id) == true && op->id != OB){
				compatible = true;
			}			
			break;
		case CHANGING_OP1:
			if (cl_is_op_reg(line->op2->id) == true && op->id != IB){
				if (op->id == OB && is_op_imm == false && line->ins->id == MOV){
					compatible = true;
				}
			}
			break;		
		case CHANGING_OP2:
			if (cl_is_op_reg(line->op1->id) == true && op->id != OB){
				compatible = true;
			}			
			break;		
		case COMPLETE:
			printf("Error, there should not be an operand assigment to a "
					"complete instruction. \n");
			break;
		default:
			printf("Error, the state of the code line is invalid\n");
			break;
	}
	return compatible;
}



/* Function: cl_create_operand
 * -----------------------------------------------------------------------------
 * This function creates a operand object.
 *
 * Arguments:
 * 	id: the id of the instruction that will be created.
 * 	b: the button object of the instruction.
 *
 * Return:
 *	Pointer to the created code node
 */
operand_t *cl_create_operand(int id, btn_t *b)
{
	assert(NULL != b && "The button pointer is NULL");
	assert((id > REG_MIN && id < REG_MAX ||
			id > MEMORY_MIN && id < MEMORY_MAX ||
			id > BUF_MIN && id < BUF_MAX) && 
	        "Invalid operand id");
	
	operand_t *op = malloc(sizeof(operand_t));	
	check_mem(op);

	op->b = b;
	op->id = id;

error:
	return op;
}

/* function: cl_destroy_operand
 * -----------------------------------------------------------------------------
 * this function destroys an operand object.
 *
 * arguments:
 * 	op: the operand that will be destroyed
 *
 * return:
 *	void.
 */
void cl_destroy_operand(operand_t *op)
{
	assert(NULL != op && "The operand pointer is NULL");
	
	bt_destroy_button(op->b);
	free(op);

	return;
}



