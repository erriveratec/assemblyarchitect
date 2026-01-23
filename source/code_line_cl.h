#ifndef CODE_LINE_CL_H
#define CODE_LINE_CL_H

#include <stdbool.h>
#include "button_bt.h"

typedef struct code_line_t code_line_t;

typedef struct operand_t{
	btn_t *b;
	int id;
	code_line_t *jptr;
} operand_t;

typedef struct instruction_t{
	btn_t *b;
	int id;
} instruction_t;

struct code_line_t{
	instruction_t *ins;
	operand_t *op1;
	operand_t *op2;
	int state;
};

enum instructions{
	INVALID_INSTRUCTION,
	INSTRUCTION_MIN,
	MOV,
	ADD,
	LABEL,
	JMP,
	INSTRUCTION_MAX
};

enum instruction_operand_quantity{
	ZERO_OPERANDS,
	ONE_OPERAND,
	TWO_OPERANDS
};

enum operands{
	INVALID_OPERAND,
	OP1,
	OP2,
	BOTH_OPERANDS,
	NO_OPERAND,
	REG_MIN,
	RAX,
	RBX,
	RCX,
	RDX,
	RDI,
	REG_MAX,
	MEMORY_MIN,
	MEMORY_MAX,
	BUF_MIN,
	IB,
	OB,
	BUF_MAX,
	IMM_MIN,
	IMM0,
	IMM1,
	IMM2,
	IMM3,
	IMM4,
	IMM5, 
	IMM6, 
	IMM7,
	IMM8,
	IMM9,
	IMM_MAX
	RGBOX_MIN,
	IBOX,
	OBOX,
	RGBOX_MAX,
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
extern char *jmp_text;
extern char *rax_text;
extern char *rbx_text;
extern char *rcx_text;
extern char *rdx_text;
extern char *rdi_text;
extern char *ib_text;
extern char *ob_text;

void cl_destroy_code_line(code_line_t *line);
instruction_t *cl_create_instruction(int id, btn_t *b);
operand_t *cl_create_operand(int id, btn_t *b);
bool cl_is_op_compatible(operand_t *op, code_line_t *line);
int cl_get_instruction_operand_quantity(int instruction_id);
char *get_operand_text(int operand_id);
void print_code_line(code_line_t *line);
char *cl_create_code_line_text(int instruction_id, int op1_id, int op2_id);
char *cl_get_instruction_text(int instruction_id);
int cl_text_to_instruction_id(char *text);
int cl_text_to_operand_id(char *text);
texture_t *cl_create_operand_texture(int id);
texture_t *cl_create_instruction_texture(int id);
code_line_t *cl_new_code_line(instruction_t *ins);
code_line_t *cl_create_code_line(instruction_t *ins);
void cl_destroy_operand(operand_t *op);
bool cl_operands_are_registers(code_line_t *line);
bool cl_is_op_reg(int op_id);

#endif
