#ifndef CODE_LINE_H
#define CODE_LINE_H

#include <stdbool.h>
#include "button_bt.h"


typedef struct operand_t{
	button_t *b;
	int id;
} operand_t;

typedef struct instruction_t{
	button_t *b;
	int id;
} instruction_t;

typedef struct code_line_t{
	instruction_t *ins;
	operand_t *op1;
	operand_t *op2;
	int state;
} code_line_t;

enum instructions{
	INVALID_INSTRUCTION,
	INSTRUCTION_MIN,
	MOV,
	ADD,
	LABEL,
	INSTRUCTION_MAX
};

enum instruction_operand_quantity{
	ZERO_OPERANDS,
	ONE_OPERAND,
	TWO_OPERANDS
};


enum operands{
	INVALID_OPERAND,	
	NO_OPERAND,
	REGISTERS_MIN,
	RAX,
	RBX,
	RCX,
	RDX,
	RDI,
	REGISTERS_MAX,
	MEMORY_MIN,
	MEMORY_MAX,
	BUFFERS_MIN,
	IB,
	OB,
	BUFFERS_MAX,
};

enum line_state{
	STATE_MIN,
	MISSING_BOTH,
	MISSING_OP1,
	MISSING_OP2,
	CHANGING_OP1,
	CHANGING_OP2,
	COMPLETE,
	IN_EXECUTION,
	EXECUTED,
	STATE_MAX
	};

extern char *add_text;
extern char *mov_text;
extern char *label_text;
extern char *rax_text;
extern char *rbx_text;
extern char *rcx_text;
extern char *rdx_text;
extern char *rdi_text;
extern char *ib_text;
extern char *ob_text;

code_line_t *cl_create_code_line(instruction_t *ins);
void cl_destroy_code_line(code_line_t *line);
code_line_t *cl_new_code_line(instruction_t *ins);
instruction_t *cl_create_instruction(int id, button_t *b);
operand_t *create_operand(int id, button_t *b);
void cl_assign_operand_to_line(operand_t *op, code_line_t *line);
//bool check_instruction_has_two_operands(code_line_t *line);
bool check_operand_compatilibity(operand_t *op, code_line_t *line);
void destroy_operand(operand_t *op);
int cl_get_instruction_operand_quantity(int instruction_id);
char *get_operand_text(int operand_id);
void print_code_line(code_line_t *line);
char *cl_create_code_line_text(int instruction_id, int op1_id, int op2_id);
char *cl_get_instruction_text(int instruction_id);
int cl_text_to_instruction_id(char *text);
int cl_text_to_operand_id(char *text);
texture_t *cl_create_operand_texture(int id);
texture_t *cl_create_instruction_texture(int id);


#endif
