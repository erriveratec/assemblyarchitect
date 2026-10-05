#include <assert.h>
#include <stdio.h>

#include "code_line_cl.h"

static bool operand_is_compatible(int instruction_id, int state,
								  int op1_id, int op2_id, int candidate_id)
{
	instruction_t instruction = {.id = instruction_id};
	operand_t first_operand = {.id = op1_id};
	operand_t second_operand = {.id = op2_id};
	operand_t candidate = {.id = candidate_id};
	code_line_t line = {
		.ins = &instruction,
		.op1 = op1_id == NO_OPERAND ? NULL : &first_operand,
		.op2 = op2_id == NO_OPERAND ? NULL : &second_operand,
		.state = state
	};

	return cl_is_op_compatible(&candidate, &line);
}

static void test_instruction_operand_counts(void)
{
	assert(cl_get_instruction_operand_quantity(MOV) == TWO_OPERANDS);
	assert(cl_get_instruction_operand_quantity(ADD) == TWO_OPERANDS);
	assert(cl_get_instruction_operand_quantity(CMP) == TWO_OPERANDS);
	assert(cl_get_instruction_operand_quantity(LABEL) == ONE_OPERAND);
	assert(cl_get_instruction_operand_quantity(JMP) == ONE_OPERAND);
	assert(cl_get_instruction_operand_quantity(JE) == ONE_OPERAND);
	assert(cl_get_instruction_operand_quantity(JNE) == ONE_OPERAND);
}

static void test_first_operand_compatibility(void)
{
	assert(operand_is_compatible(MOV, MISSING_BOTH, NO_OPERAND, NO_OPERAND,
								 RAX));
	assert(operand_is_compatible(MOV, MISSING_BOTH, NO_OPERAND, NO_OPERAND,
								 OB));
	assert(!operand_is_compatible(MOV, MISSING_BOTH, NO_OPERAND, NO_OPERAND,
								 IB));
	assert(!operand_is_compatible(MOV, MISSING_BOTH, NO_OPERAND, NO_OPERAND,
								 IMMUP0));
	assert(!operand_is_compatible(ADD, MISSING_BOTH, NO_OPERAND, NO_OPERAND,
								 OB));
}

static void test_second_operand_compatibility(void)
{
	assert(operand_is_compatible(MOV, MISSING_OP2, RAX, NO_OPERAND, RAX));
	assert(operand_is_compatible(MOV, MISSING_OP2, RAX, NO_OPERAND, IB));
	assert(operand_is_compatible(MOV, MISSING_OP2, RAX, NO_OPERAND, IMMUP0));
	assert(!operand_is_compatible(MOV, MISSING_OP2, RAX, NO_OPERAND, OB));
	assert(!operand_is_compatible(MOV, MISSING_OP2, OB, NO_OPERAND, IB));
	assert(operand_is_compatible(MOV, MISSING_OP2, OB, NO_OPERAND, RAX));
}

static void test_operand_replacement_compatibility(void)
{
	assert(operand_is_compatible(MOV, MISSING_OP1, NO_OPERAND, RAX, RAX));
	assert(!operand_is_compatible(MOV, MISSING_OP1, NO_OPERAND, RAX, IB));
	assert(!operand_is_compatible(MOV, MISSING_OP1, NO_OPERAND, RAX, IMMUP0));
	assert(operand_is_compatible(MOV, CHANGING_OP1, NO_OPERAND, NO_OPERAND,
								 OB));
	assert(!operand_is_compatible(ADD, CHANGING_OP1, NO_OPERAND, NO_OPERAND,
								 OB));
	assert(!operand_is_compatible(JMP, MISSING_OP1, NO_OPERAND, NO_OPERAND,
								 RAX));
}

int main(void)
{
	test_instruction_operand_counts();
	test_first_operand_compatibility();
	test_second_operand_compatibility();
	test_operand_replacement_compatibility();
	puts("program characterization tests passed");
	return 0;
}