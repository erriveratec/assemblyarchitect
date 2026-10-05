#include <assert.h>
#include <stdio.h>

#include "domain/program.h"

static aa_instruction_id_t append_instruction(aa_program_t *program,
											  aa_opcode_t opcode)
{
	aa_instruction_t instruction = aa_instruction_create(opcode);
	aa_instruction_id_t id = AA_INSTRUCTION_ID_INVALID;
	assert(aa_program_append(program, &instruction, &id) == AA_RESULT_OK);
	assert(id != AA_INSTRUCTION_ID_INVALID);
	return id;
}

static void test_program_editing_and_identity(void)
{
	aa_program_t *program = aa_program_create();
	assert(program != NULL);
	assert(aa_program_count(program) == 0);
	assert(aa_program_instruction_at(program, 0) == NULL);

	aa_instruction_id_t mov_id = append_instruction(program, AA_OPCODE_MOV);
	aa_instruction_id_t add_id = append_instruction(program, AA_OPCODE_ADD);
	aa_instruction_t compare = aa_instruction_create(AA_OPCODE_CMP);
	aa_instruction_id_t cmp_id = AA_INSTRUCTION_ID_INVALID;
	assert(aa_program_insert(program, 1, &compare, &cmp_id) == AA_RESULT_OK);
	assert(aa_program_count(program) == 3);
	assert(aa_program_instruction_at(program, 0)->id == mov_id);
	assert(aa_program_instruction_at(program, 1)->id == cmp_id);
	assert(aa_program_instruction_at(program, 2)->id == add_id);

	uint64_t revision = aa_program_revision(program);
	assert(aa_program_move(program, 2, 0) == AA_RESULT_OK);
	assert(aa_program_instruction_at(program, 0)->id == add_id);
	assert(aa_program_find_by_id(program, add_id) != NULL);
	assert(aa_program_revision(program) == revision + 1);

	aa_instruction_id_t clone_id = AA_INSTRUCTION_ID_INVALID;
	assert(aa_program_clone_instruction(program, mov_id, 1, &clone_id) ==
		   AA_RESULT_OK);
	assert(clone_id != mov_id);
	assert(aa_program_instruction_at(program, 1)->id == clone_id);
	assert(aa_program_remove_by_id(program, clone_id) == AA_RESULT_OK);
	assert(aa_program_remove(program, 0) == AA_RESULT_OK);
	assert(aa_program_remove(program, aa_program_count(program) - 1) ==
		   AA_RESULT_OK);
	aa_program_clear(program);
	assert(aa_program_count(program) == 0);
	aa_program_destroy(program);
}

static void test_operand_editing_and_validation(void)
{
	aa_program_t *program = aa_program_create();
	aa_instruction_id_t mov_id = append_instruction(program, AA_OPCODE_MOV);
	aa_validation_report_t report = aa_program_validate(program, NULL);
	assert(report.result == AA_RESULT_INVALID_OPERAND);
	assert(report.instruction_id == mov_id);
	assert(report.operand_position == 0);

	assert(aa_program_set_operand(program, mov_id, 0,
								  aa_operand_register(AA_REGISTER_RAX)) == AA_RESULT_OK);
	assert(aa_program_set_operand(program, mov_id, 1,
								  aa_operand_buffer(AA_BUFFER_INPUT)) == AA_RESULT_OK);
	assert(aa_instruction_validate(aa_program_find_by_id(program, mov_id)) ==
		   AA_RESULT_OK);
	assert(aa_program_set_operand(program, mov_id, 0,
								  aa_operand_buffer(AA_BUFFER_OUTPUT)) ==
		   AA_RESULT_INCOMPATIBLE_OPERAND);
	assert(aa_program_clear_operand(program, mov_id, 1) == AA_RESULT_OK);
	assert(aa_program_validate(program, NULL).result == AA_RESULT_INVALID_OPERAND);
	aa_program_destroy(program);
}

static void test_stable_label_references(void)
{
	aa_program_t *program = aa_program_create();
	aa_instruction_id_t label_id = append_instruction(program, AA_OPCODE_LABEL);
	aa_instruction_id_t jump_id = append_instruction(program, AA_OPCODE_JMP);
	assert(aa_program_set_operand(program, jump_id, 0,
								  aa_operand_label_reference(label_id)) == AA_RESULT_OK);
	assert(aa_program_validate(program, NULL).result == AA_RESULT_OK);
	assert(aa_program_move(program, 0, 1) == AA_RESULT_OK);
	assert(aa_program_validate(program, NULL).result == AA_RESULT_OK);
	assert(aa_program_remove_by_id(program, label_id) == AA_RESULT_OK);
	aa_validation_report_t report = aa_program_validate(program, NULL);
	assert(report.result == AA_RESULT_UNRESOLVED_SYMBOL);
	assert(report.instruction_id == jump_id);
	assert(report.operand_position == 0);
	aa_program_destroy(program);
}

static void test_insert_from_borrowed_instruction(void)
{
	aa_program_t *program = aa_program_create();
	aa_instruction_id_t label_id = append_instruction(program, AA_OPCODE_LABEL);
	aa_instruction_id_t jump_id = append_instruction(program, AA_OPCODE_JMP);
	assert(aa_program_set_operand(program, jump_id, 0,
								  aa_operand_label_reference(label_id)) == AA_RESULT_OK);
	for (int index = 0; index < 6; index++) {
		append_instruction(program, AA_OPCODE_LABEL);
	}
	assert(aa_program_count(program) == 8);

	const aa_instruction_t *borrowed = aa_program_find_by_id(program, jump_id);
	aa_instruction_id_t inserted_id = AA_INSTRUCTION_ID_INVALID;
	assert(aa_program_insert(program, 0, borrowed, &inserted_id) == AA_RESULT_OK);
	const aa_instruction_t *inserted = aa_program_instruction_at(program, 0);
	assert(inserted->id == inserted_id);
	assert(inserted->id != jump_id);
	assert(inserted->opcode == AA_OPCODE_JMP);
	assert(inserted->operands[0].kind == AA_OPERAND_LABEL_REFERENCE);
	assert(inserted->operands[0].value.label_instruction_id == label_id);
	assert(aa_program_find_by_id(program, jump_id) != NULL);
	aa_program_destroy(program);
}

static void test_instruction_limit(void)
{
	aa_program_t *program = aa_program_create();
	aa_instruction_id_t first_id = append_instruction(program, AA_OPCODE_LABEL);
	aa_instruction_id_t over_limit_id =
		append_instruction(program, AA_OPCODE_LABEL);
	aa_program_rules_t rules = {.instruction_limit = 1};
	aa_validation_report_t report = aa_program_validate(program, &rules);
	assert(report.result == AA_RESULT_INSTRUCTION_LIMIT_REACHED);
	assert(report.position == 1);
	assert(report.instruction_id == over_limit_id);
	assert(report.instruction_id != first_id);
	aa_program_destroy(program);
}

static void test_failed_edits_are_atomic(void)
{
	aa_program_t *program = aa_program_create();
	aa_instruction_id_t mov_id = append_instruction(program, AA_OPCODE_MOV);
	assert(aa_program_set_operand(program, mov_id, 0,
								  aa_operand_register(AA_REGISTER_RAX)) == AA_RESULT_OK);
	assert(aa_program_set_operand(program, mov_id, 1,
								  aa_operand_buffer(AA_BUFFER_INPUT)) == AA_RESULT_OK);
	uint64_t revision = aa_program_revision(program);
	const aa_instruction_t *before = aa_program_find_by_id(program, mov_id);
	aa_instruction_t snapshot = *before;

	assert(aa_program_insert(program, 2, &snapshot, NULL) == AA_RESULT_OUT_OF_RANGE);
	assert(aa_program_remove(program, 1) == AA_RESULT_OUT_OF_RANGE);
	assert(aa_program_move(program, 0, 1) == AA_RESULT_OUT_OF_RANGE);
	assert(aa_program_set_operand(program, mov_id, 2,
								  aa_operand_register(AA_REGISTER_RBX)) ==
		   AA_RESULT_OUT_OF_RANGE);
	assert(aa_program_set_operand(program, mov_id, 0,
								  aa_operand_buffer(AA_BUFFER_OUTPUT)) ==
		   AA_RESULT_INCOMPATIBLE_OPERAND);

	assert(aa_program_count(program) == 1);
	assert(aa_program_revision(program) == revision);
	const aa_instruction_t *after = aa_program_find_by_id(program, mov_id);
	assert(after != NULL);
	assert(after->opcode == snapshot.opcode);
	assert(after->operands[0].kind == snapshot.operands[0].kind);
	assert(after->operands[0].value.reg == snapshot.operands[0].value.reg);
	assert(after->operands[1].kind == snapshot.operands[1].kind);
	assert(after->operands[1].value.buffer == snapshot.operands[1].value.buffer);
	aa_program_destroy(program);
}

int main(void)
{
	test_program_editing_and_identity();
	test_operand_editing_and_validation();
	test_stable_label_references();
	test_insert_from_borrowed_instruction();
	test_instruction_limit();
	test_failed_edits_are_atomic();
	puts("program domain tests passed");
	return 0;
}