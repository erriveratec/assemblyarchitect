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

static void test_identity_preserving_reconciliation(void)
{
	aa_program_t *program = aa_program_create();
	assert(program != NULL);
	aa_instruction_id_t label_id = append_instruction(program, AA_OPCODE_LABEL);
	aa_instruction_id_t jump_id = append_instruction(program, AA_OPCODE_JMP);
	aa_instruction_id_t first_mov_id = append_instruction(program, AA_OPCODE_MOV);
	aa_instruction_id_t duplicate_mov_id = append_instruction(program,
		AA_OPCODE_MOV);
	assert(aa_program_set_operand(program, jump_id, 0,
								  aa_operand_label_reference(label_id)) == AA_RESULT_OK);
	aa_instruction_t candidate[4] = {
		{.id = label_id, .opcode = AA_OPCODE_LABEL, .operand_count = 0},
		{.id = jump_id, .opcode = AA_OPCODE_JMP, .operand_count = 1,
		 .operands = {{.kind = AA_OPERAND_LABEL_REFERENCE,
					   .value.label_instruction_id = label_id}}},
		{.id = first_mov_id, .opcode = AA_OPCODE_MOV, .operand_count = 2},
		{.id = duplicate_mov_id, .opcode = AA_OPCODE_MOV, .operand_count = 2}
	};
	uint64_t revision = aa_program_revision(program);
	assert(aa_program_reconcile(program, candidate, 4) == AA_RESULT_OK);
	assert(aa_program_revision(program) == revision);
	assert(aa_program_instruction_at(program, 0)->id == label_id);
	assert(aa_program_instruction_at(program, 1)->id == jump_id);
	assert(aa_program_instruction_at(program, 2)->id == first_mov_id);
	assert(aa_program_instruction_at(program, 3)->id == duplicate_mov_id);
	assert(aa_program_instruction_at(program, 1)->operands[0].value.
		label_instruction_id == label_id);
	assert(aa_program_instruction_at(program, 2)->operands[0].kind ==
		AA_OPERAND_NONE);
	const aa_instruction_t *borrowed_program =
		aa_program_instruction_at(program, 0);
	assert(aa_program_reconcile(program, borrowed_program,
							   aa_program_count(program)) == AA_RESULT_OK);
	assert(aa_program_revision(program) == revision);

	candidate[2].operands[0] = aa_operand_register(AA_REGISTER_RAX);
	candidate[2].operands[1] = aa_operand_buffer(AA_BUFFER_INPUT);
	revision = aa_program_revision(program);
	assert(aa_program_reconcile(program, candidate, 4) == AA_RESULT_OK);
	assert(aa_program_revision(program) == revision + 1);
	assert(aa_program_instruction_at(program, 2)->id == first_mov_id);
	assert(aa_program_instruction_at(program, 3)->id == duplicate_mov_id);
	assert(aa_program_instruction_at(program, 2)->operands[0].value.reg ==
		AA_REGISTER_RAX);
	assert(aa_program_instruction_at(program, 3)->operands[0].kind ==
		AA_OPERAND_NONE);

	aa_instruction_t appended = aa_instruction_create(AA_OPCODE_CMP);
	aa_instruction_id_t appended_id = AA_INSTRUCTION_ID_INVALID;
	assert(aa_program_append(program, &appended, &appended_id) == AA_RESULT_OK);
	assert(appended_id == duplicate_mov_id + 1);

	aa_instruction_t high_ids[3] = {candidate[0], candidate[1], candidate[2]};
	high_ids[2].id = 20;
	revision = aa_program_revision(program);
	assert(aa_program_reconcile(program, high_ids, 3) == AA_RESULT_OK);
	assert(aa_program_revision(program) == revision + 1);
	aa_instruction_t lower_ids[3] = {candidate[0], candidate[1], candidate[2]};
	lower_ids[2].id = 3;
	assert(aa_program_reconcile(program, lower_ids, 3) == AA_RESULT_OK);
	aa_instruction_id_t after_lower_ids = AA_INSTRUCTION_ID_INVALID;
	assert(aa_program_append(program, &appended, &after_lower_ids) == AA_RESULT_OK);
	assert(after_lower_ids == 21);

	aa_instruction_t before[4];
	for (size_t position = 0; position < aa_program_count(program); position++) {
		before[position] = *aa_program_instruction_at(program, position);
	}
	revision = aa_program_revision(program);
	aa_instruction_t invalid[3] = {lower_ids[0], lower_ids[1], lower_ids[2]};
	invalid[0].id = AA_INSTRUCTION_ID_INVALID;
	assert(aa_program_reconcile(program, invalid, 3) == AA_RESULT_INVALID_ARGUMENT);
	invalid[0] = lower_ids[0];
	invalid[2].id = invalid[1].id;
	assert(aa_program_reconcile(program, invalid, 3) == AA_RESULT_INVALID_ARGUMENT);
	invalid[2] = lower_ids[2];
	invalid[1].operands[0] = aa_operand_label_reference(999);
	assert(aa_program_reconcile(program, invalid, 3) ==
		AA_RESULT_UNRESOLVED_SYMBOL);
	invalid[1].operands[0] = aa_operand_label_reference(lower_ids[2].id);
	assert(aa_program_reconcile(program, invalid, 3) ==
		AA_RESULT_UNRESOLVED_SYMBOL);
	assert(aa_program_revision(program) == revision);
	assert(aa_program_count(program) == 4);
	for (size_t position = 0; position < aa_program_count(program); position++) {
		const aa_instruction_t *current = aa_program_instruction_at(program,
																position);
		assert(current->id == before[position].id);
		assert(current->opcode == before[position].opcode);
		assert(current->operands[0].kind == before[position].operands[0].kind);
		assert(current->operands[1].kind == before[position].operands[1].kind);
	}

	invalid[0] = lower_ids[0];
	invalid[0].id = UINT32_MAX;
	assert(aa_program_reconcile(program, invalid, 1) == AA_RESULT_ID_EXHAUSTED);
	assert(aa_program_revision(program) == revision);
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
	test_identity_preserving_reconciliation();
	puts("program domain tests passed");
	return 0;
}