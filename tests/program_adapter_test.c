#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "domain/program.h"
#include "legacy_code_ids.h"
#include "migration/legacy_program_adapter.h"

#define FIXTURE_CAPACITY 64

typedef struct legacy_fixture {
	aa_legacy_line_snapshot_t lines[FIXTURE_CAPACITY];
	unsigned char identities[FIXTURE_CAPACITY];
	unsigned char outside_identity;
	size_t count;
	size_t failed_read_position;
} legacy_fixture_t;

static size_t fixture_count(const void *context)
{
	const legacy_fixture_t *fixture = context;
	return fixture->count;
}

static bool fixture_read(const void *context, size_t position,
						 aa_legacy_line_snapshot_t *line)
{
	const legacy_fixture_t *fixture = context;
	if (line == NULL || position >= fixture->count ||
		position == fixture->failed_read_position) {
		return false;
	}
	*line = fixture->lines[position];
	return true;
}

static aa_legacy_program_reader_t fixture_reader(const legacy_fixture_t *fixture)
{
	aa_legacy_program_reader_t reader = {
		.count = fixture_count,
		.read_line = fixture_read,
		.context = fixture
	};
	return reader;
}

static aa_legacy_line_snapshot_t *fixture_add(legacy_fixture_t *fixture,
											 int opcode)
{
	assert(fixture->count < FIXTURE_CAPACITY);
	size_t position = fixture->count++;
	aa_legacy_line_snapshot_t *line = &fixture->lines[position];
	memset(line, 0, sizeof(*line));
	line->identity = &fixture->identities[position];
	line->opcode = opcode;
	line->line_state = COMPLETE;
	return line;
}

static void import_ok(aa_program_t *program, const legacy_fixture_t *fixture)
{
	aa_legacy_line_snapshot_t before[FIXTURE_CAPACITY];
	memcpy(before, fixture->lines, sizeof(before));
	aa_legacy_program_reader_t reader = fixture_reader(fixture);
	aa_legacy_import_report_t report;
	assert(aa_legacy_program_import(program, &reader, &report) == AA_RESULT_OK);
	assert(report.result == AA_RESULT_OK);
	assert(report.issue == AA_LEGACY_IMPORT_ISSUE_NONE);
	assert(memcmp(before, fixture->lines, sizeof(before)) == 0);
}

static size_t position_for_id(const aa_program_t *program,
							  aa_instruction_id_t id)
{
	for (size_t position = 0; position < aa_program_count(program); position++) {
		const aa_instruction_t *instruction =
			aa_program_instruction_at(program, position);
		if (instruction->id == id) {
			return position;
		}
	}
	return SIZE_MAX;
}

static bool programs_semantically_equal(const aa_program_t *left,
										const aa_program_t *right)
{
	if (aa_program_count(left) != aa_program_count(right)) {
		return false;
	}
	for (size_t position = 0; position < aa_program_count(left); position++) {
		const aa_instruction_t *left_instruction =
			aa_program_instruction_at(left, position);
		const aa_instruction_t *right_instruction =
			aa_program_instruction_at(right, position);
		if (left_instruction->opcode != right_instruction->opcode ||
			left_instruction->operand_count != right_instruction->operand_count) {
			return false;
		}
		for (size_t operand_position = 0;
			 operand_position < left_instruction->operand_count;
			 operand_position++) {
			const aa_operand_t *left_operand =
				&left_instruction->operands[operand_position];
			const aa_operand_t *right_operand =
				&right_instruction->operands[operand_position];
			if (left_operand->kind != right_operand->kind) {
				return false;
			}
			switch (left_operand->kind) {
			case AA_OPERAND_NONE:
				break;
			case AA_OPERAND_REGISTER:
				if (left_operand->value.reg != right_operand->value.reg) {
					return false;
				}
				break;
			case AA_OPERAND_BUFFER:
				if (left_operand->value.buffer != right_operand->value.buffer) {
					return false;
				}
				break;
			case AA_OPERAND_IMMEDIATE:
				if (left_operand->value.immediate !=
					right_operand->value.immediate) {
					return false;
				}
				break;
			case AA_OPERAND_LABEL_REFERENCE:
				if (position_for_id(left,
									left_operand->value.label_instruction_id) !=
					position_for_id(right,
									 right_operand->value.label_instruction_id)) {
					return false;
				}
				break;
			default:
				return false;
			}
		}
	}
	return true;
}

static void test_empty_and_label_import(void)
{
	legacy_fixture_t fixture = {0};
	fixture.failed_read_position = SIZE_MAX;
	aa_program_t *program = aa_program_create();
	assert(program != NULL);
	aa_instruction_t prior = aa_instruction_create(AA_OPCODE_LABEL);
	assert(aa_program_append(program, &prior, NULL) == AA_RESULT_OK);
	uint64_t revision = aa_program_revision(program);
	import_ok(program, &fixture);
	assert(aa_program_count(program) == 0);
	assert(aa_program_revision(program) == revision + 1);

	aa_legacy_line_snapshot_t *label = fixture_add(&fixture, LABEL);
	label->has_operand_1 = true;
	label->operand_1 = 77;
	import_ok(program, &fixture);
	assert(aa_program_count(program) == 1);
	assert(aa_program_instruction_at(program, 0)->opcode == AA_OPCODE_LABEL);
	assert(aa_program_instruction_at(program, 0)->operand_count == 0);
	aa_instruction_id_t label_id = aa_program_instruction_at(program, 0)->id;
	revision = aa_program_revision(program);
	import_ok(program, &fixture);
	assert(aa_program_revision(program) == revision);
	assert(aa_program_instruction_at(program, 0)->id == label_id);
	aa_program_destroy(program);
}

static void test_opcode_and_operand_mapping(void)
{
	legacy_fixture_t fixture = {0};
	fixture.failed_read_position = SIZE_MAX;
	const int registers[] = {RAX, RBX, RCX, RDX, RDI};
	for (size_t index = 0; index < sizeof(registers) / sizeof(registers[0]);
		 index++) {
		aa_legacy_line_snapshot_t *line = fixture_add(&fixture, MOV);
		line->has_operand_1 = true;
		line->operand_1 = registers[index];
		line->has_operand_2 = true;
		line->operand_2 = registers[(index + 1) % 5];
	}
	aa_legacy_line_snapshot_t *input_move = fixture_add(&fixture, MOV);
	input_move->has_operand_1 = true;
	input_move->operand_1 = RAX;
	input_move->has_operand_2 = true;
	input_move->operand_2 = IB;
	aa_legacy_line_snapshot_t *output_move = fixture_add(&fixture, MOV);
	output_move->has_operand_1 = true;
	output_move->operand_1 = OB;
	output_move->has_operand_2 = true;
	output_move->operand_2 = RDI;
	aa_legacy_line_snapshot_t *add = fixture_add(&fixture, ADD);
	add->has_operand_1 = true;
	add->operand_1 = RBX;
	add->has_operand_2 = true;
	add->operand_2 = IMMUP10;
	aa_legacy_line_snapshot_t *compare = fixture_add(&fixture, CMP);
	compare->has_operand_1 = true;
	compare->operand_1 = RCX;
	compare->has_operand_2 = true;
	compare->operand_2 = RDX;

	aa_program_t *program = aa_program_create();
	assert(program != NULL);
	import_ok(program, &fixture);
	assert(aa_program_count(program) == fixture.count);
	for (size_t index = 0; index < 5; index++) {
		const aa_instruction_t *instruction =
			aa_program_instruction_at(program, index);
		assert(instruction->opcode == AA_OPCODE_MOV);
		assert(instruction->operands[0].value.reg ==
			   (aa_register_t)(AA_REGISTER_RAX + index));
	}
	const aa_instruction_t *input_instruction =
		aa_program_instruction_at(program, 5);
	assert(input_instruction->operands[1].kind == AA_OPERAND_BUFFER);
	assert(input_instruction->operands[1].value.buffer == AA_BUFFER_INPUT);
	const aa_instruction_t *output_instruction =
		aa_program_instruction_at(program, 6);
	assert(output_instruction->operands[0].value.buffer == AA_BUFFER_OUTPUT);
	assert(aa_program_instruction_at(program, 7)->opcode == AA_OPCODE_ADD);
	assert(aa_program_instruction_at(program, 8)->opcode == AA_OPCODE_CMP);
	aa_program_destroy(program);
}

static void test_all_legacy_immediates(void)
{
	legacy_fixture_t fixture = {0};
	fixture.failed_read_position = SIZE_MAX;
	for (int id = IMMUP0; id <= IMMUP10; id++) {
		aa_legacy_line_snapshot_t *line = fixture_add(&fixture, MOV);
		line->has_operand_1 = true;
		line->operand_1 = RAX;
		line->has_operand_2 = true;
		line->operand_2 = id;
	}
	for (int id = IMMUP_1; id <= IMMUP_9; id++) {
		aa_legacy_line_snapshot_t *line = fixture_add(&fixture, MOV);
		line->has_operand_1 = true;
		line->operand_1 = RAX;
		line->has_operand_2 = true;
		line->operand_2 = id;
	}
	for (int id = IMMDO0; id <= IMMDO9; id++) {
		aa_legacy_line_snapshot_t *line = fixture_add(&fixture, MOV);
		line->has_operand_1 = true;
		line->operand_1 = RAX;
		line->has_operand_2 = true;
		line->operand_2 = id;
	}
	aa_program_t *program = aa_program_create();
	assert(program != NULL);
	import_ok(program, &fixture);
	assert(aa_program_count(program) == 30);
	for (size_t position = 0; position < 30; position++) {
		const aa_instruction_t *instruction =
			aa_program_instruction_at(program, position);
		assert(instruction->operands[1].kind == AA_OPERAND_IMMEDIATE);
		int expected = position <= 10 ? (int)position : -(int)(position - 10);
		assert(instruction->operands[1].value.immediate == expected);
	}
	aa_program_destroy(program);
}

static void test_incomplete_instructions_and_line_states(void)
{
	legacy_fixture_t first = {0};
	legacy_fixture_t second = {0};
	first.failed_read_position = SIZE_MAX;
	second.failed_read_position = SIZE_MAX;
	aa_legacy_line_snapshot_t *jump = fixture_add(&first, JMP);
	jump->line_state = MISSING_OP1;
	aa_legacy_line_snapshot_t *move = fixture_add(&first, MOV);
	move->line_state = MISSING_BOTH;
	aa_legacy_line_snapshot_t *jump_again = fixture_add(&second, JMP);
	jump_again->line_state = IN_EXECUTION;
	aa_legacy_line_snapshot_t *move_again = fixture_add(&second, MOV);
	move_again->line_state = EXECUTED;
	aa_legacy_line_snapshot_t *changing_move = fixture_add(&first, MOV);
	changing_move->has_operand_1 = true;
	changing_move->operand_1 = RAX;
	changing_move->has_operand_2 = true;
	changing_move->operand_2 = IB;
	changing_move->line_state = CHANGING_OP1;
	aa_legacy_line_snapshot_t *complete_move = fixture_add(&second, MOV);
	complete_move->has_operand_1 = true;
	complete_move->operand_1 = RAX;
	complete_move->has_operand_2 = true;
	complete_move->operand_2 = IB;
	complete_move->line_state = COMPLETE;
	aa_program_t *left = aa_program_create();
	aa_program_t *right = aa_program_create();
	assert(left != NULL && right != NULL);
	import_ok(left, &first);
	import_ok(right, &second);
	assert(programs_semantically_equal(left, right));
	assert(aa_program_validate_partial(left, NULL).result == AA_RESULT_OK);
	assert(aa_program_validate(left, NULL).result == AA_RESULT_INVALID_OPERAND);
	aa_program_destroy(left);
	aa_program_destroy(right);
}

static void test_two_pass_jump_resolution_and_ids(void)
{
	legacy_fixture_t fixture = {0};
	fixture.failed_read_position = SIZE_MAX;
	aa_legacy_line_snapshot_t *backward_label = fixture_add(&fixture, LABEL);
	aa_legacy_line_snapshot_t *forward_jump = fixture_add(&fixture, JMP);
	aa_legacy_line_snapshot_t *conditional_equal = fixture_add(&fixture, JE);
	aa_legacy_line_snapshot_t *conditional_not_equal = fixture_add(&fixture, JNE);
	aa_legacy_line_snapshot_t *forward_label = fixture_add(&fixture, LABEL);
	aa_legacy_line_snapshot_t *backward_jump = fixture_add(&fixture, JMP);
	forward_jump->has_operand_1 = true;
	forward_jump->jump_target_identity = forward_label->identity;
	conditional_equal->has_operand_1 = true;
	conditional_equal->jump_target_identity = forward_label->identity;
	conditional_not_equal->has_operand_1 = true;
	conditional_not_equal->jump_target_identity = backward_label->identity;
	backward_jump->has_operand_1 = true;
	backward_jump->jump_target_identity = backward_label->identity;

	aa_program_t *left = aa_program_create();
	aa_program_t *right = aa_program_create();
	assert(left != NULL && right != NULL);
	for (int index = 0; index < 3; index++) {
		aa_instruction_t old = aa_instruction_create(AA_OPCODE_LABEL);
		assert(aa_program_append(left, &old, NULL) == AA_RESULT_OK);
	}
	aa_instruction_t old = aa_instruction_create(AA_OPCODE_LABEL);
	assert(aa_program_append(right, &old, NULL) == AA_RESULT_OK);
	import_ok(left, &fixture);
	import_ok(right, &fixture);
	assert(programs_semantically_equal(left, right));
	assert(aa_program_instruction_at(left, 0)->id !=
		   aa_program_instruction_at(right, 0)->id);
	for (size_t position = 0; position < fixture.count; position++) {
		const aa_instruction_t *instruction =
			aa_program_instruction_at(left, position);
		if (instruction->opcode == AA_OPCODE_JMP ||
			instruction->opcode == AA_OPCODE_JE ||
			instruction->opcode == AA_OPCODE_JNE) {
			assert(instruction->operands[0].kind ==
				   AA_OPERAND_LABEL_REFERENCE);
			const aa_instruction_t *target = aa_program_find_by_id(
				left, instruction->operands[0].value.label_instruction_id);
			assert(target != NULL && target->opcode == AA_OPCODE_LABEL);
		}
	}
	aa_program_destroy(left);
	aa_program_destroy(right);
}

static void assert_program_unchanged(const aa_program_t *program,
									 const aa_instruction_t *snapshot,
									 uint64_t revision)
{
	assert(aa_program_count(program) == 1);
	assert(aa_program_revision(program) == revision);
	const aa_instruction_t *current = aa_program_instruction_at(program, 0);
	assert(current->id == snapshot->id);
	assert(current->opcode == snapshot->opcode);
	assert(current->operand_count == snapshot->operand_count);
	assert(current->operands[0].kind == snapshot->operands[0].kind);
	assert(current->operands[0].value.reg == snapshot->operands[0].value.reg);
	assert(current->operands[1].kind == snapshot->operands[1].kind);
	assert(current->operands[1].value.buffer == snapshot->operands[1].value.buffer);
}

static void test_import_failures_are_atomic(void)
{
	aa_program_t *destination = aa_program_create();
	assert(destination != NULL);
	aa_instruction_t move = aa_instruction_create(AA_OPCODE_MOV);
	move.operands[0] = aa_operand_register(AA_REGISTER_RAX);
	move.operands[1] = aa_operand_buffer(AA_BUFFER_INPUT);
	assert(aa_program_append(destination, &move, NULL) == AA_RESULT_OK);
	aa_instruction_t original = *aa_program_instruction_at(destination, 0);
	uint64_t original_revision = aa_program_revision(destination);

	legacy_fixture_t fixture = {0};
	fixture.failed_read_position = SIZE_MAX;
	aa_legacy_line_snapshot_t *bad = fixture_add(&fixture, 9999);
	aa_legacy_line_snapshot_t saved[FIXTURE_CAPACITY];
	memcpy(saved, fixture.lines, sizeof(saved));
	aa_legacy_program_reader_t reader = fixture_reader(&fixture);
	aa_legacy_import_report_t report;
	assert(aa_legacy_program_import(destination, &reader, &report) ==
		   AA_RESULT_INVALID_OPCODE);
	assert(report.issue == AA_LEGACY_IMPORT_ISSUE_UNKNOWN_OPCODE);
	assert_program_unchanged(destination, &original, original_revision);
	assert(memcmp(saved, fixture.lines, sizeof(saved)) == 0);

	bad->opcode = MOV;
	bad->has_operand_1 = true;
	bad->operand_1 = RAX;
	bad->has_operand_2 = true;
	bad->operand_2 = 9999;
	assert(aa_legacy_program_import(destination, &reader, &report) ==
		   AA_RESULT_INVALID_OPERAND);
	assert(report.issue == AA_LEGACY_IMPORT_ISSUE_UNKNOWN_OPERAND);
	assert_program_unchanged(destination, &original, original_revision);

	bad->opcode = JMP;
	bad->has_operand_1 = true;
	bad->has_operand_2 = false;
	bad->jump_target_identity = &fixture.outside_identity;
	assert(aa_legacy_program_import(destination, &reader, &report) ==
		   AA_RESULT_UNRESOLVED_SYMBOL);
	assert(report.issue == AA_LEGACY_IMPORT_ISSUE_JUMP_TARGET_MISSING);
	assert_program_unchanged(destination, &original, original_revision);

	bad->opcode = MOV;
	aa_legacy_line_snapshot_t *not_label = fixture_add(&fixture, MOV);
	bad->opcode = JMP;
	bad->jump_target_identity = not_label->identity;
	assert(aa_legacy_program_import(destination, &reader, &report) ==
		   AA_RESULT_UNRESOLVED_SYMBOL);
	assert(report.issue == AA_LEGACY_IMPORT_ISSUE_JUMP_TARGET_NOT_LABEL);
	assert_program_unchanged(destination, &original, original_revision);
	aa_program_destroy(destination);
}

static void assert_reconcile_rejected_unchanged(
	aa_program_t *program,
	legacy_fixture_t *fixture,
	aa_result_t expected_result,
	aa_legacy_import_issue_t expected_issue)
{
	aa_instruction_t before[FIXTURE_CAPACITY];
	size_t count = aa_program_count(program);
	assert(count <= FIXTURE_CAPACITY);
	for (size_t position = 0; position < count; position++) {
		before[position] = *aa_program_instruction_at(program, position);
	}
	uint64_t revision = aa_program_revision(program);
	aa_legacy_program_reader_t reader = fixture_reader(fixture);
	aa_legacy_import_report_t report;
	assert(aa_legacy_program_reconcile(program, &reader, &report) ==
		expected_result);
	assert(report.issue == expected_issue);
	assert(aa_program_count(program) == count);
	assert(aa_program_revision(program) == revision);
	for (size_t position = 0; position < count; position++) {
		assert(memcmp(aa_program_instruction_at(program, position),
					  &before[position], sizeof(before[position])) == 0);
	}
}

static void test_identity_bound_reconciliation(void)
{
	legacy_fixture_t fixture = {0};
	fixture.failed_read_position = SIZE_MAX;
	aa_legacy_line_snapshot_t *label = fixture_add(&fixture, LABEL);
	aa_legacy_line_snapshot_t *jump = fixture_add(&fixture, JMP);
	aa_legacy_line_snapshot_t *second_jump = fixture_add(&fixture, JE);
	aa_legacy_line_snapshot_t *first_mov = fixture_add(&fixture, MOV);
	aa_legacy_line_snapshot_t *duplicate_mov = fixture_add(&fixture, MOV);
	aa_legacy_line_snapshot_t *incomplete_jump = fixture_add(&fixture, JNE);

	aa_program_t *program = aa_program_create();
	assert(program != NULL);
	import_ok(program, &fixture);
	aa_instruction_id_t ids[6];
	for (size_t position = 0; position < fixture.count; position++) {
		ids[position] = aa_program_instruction_at(program, position)->id;
		fixture.lines[position].bound_instruction_id = ids[position];
	}
	uint64_t revision = aa_program_revision(program);
	aa_legacy_program_reader_t reader = fixture_reader(&fixture);
	aa_legacy_import_report_t report;
	assert(aa_legacy_program_reconcile(program, &reader, &report) == AA_RESULT_OK);
	assert(aa_program_revision(program) == revision);

	jump->has_operand_1 = true;
	jump->jump_target_identity = label->identity;
	second_jump->has_operand_1 = true;
	second_jump->jump_target_identity = label->identity;
	first_mov->has_operand_1 = true;
	first_mov->operand_1 = RAX;
	first_mov->has_operand_2 = true;
	first_mov->operand_2 = IB;
	revision = aa_program_revision(program);
	assert(aa_legacy_program_reconcile(program, &reader, &report) == AA_RESULT_OK);
	assert(aa_program_revision(program) == revision + 1);
	for (size_t position = 0; position < fixture.count; position++) {
		assert(aa_program_instruction_at(program, position)->id == ids[position]);
	}
	assert(aa_program_find_by_id(program, ids[1])->operands[0].value.
		label_instruction_id == ids[0]);
	assert(aa_program_find_by_id(program, ids[2])->operands[0].value.
		label_instruction_id == ids[0]);
	assert(aa_program_find_by_id(program, ids[3])->operands[0].value.reg ==
		AA_REGISTER_RAX);
	assert(aa_program_find_by_id(program, ids[4])->operands[0].kind ==
		AA_OPERAND_NONE);
	assert(aa_program_find_by_id(program, ids[5])->operands[0].kind ==
		AA_OPERAND_NONE);
	revision = aa_program_revision(program);
	assert(aa_legacy_program_reconcile(program, &reader, &report) == AA_RESULT_OK);
	assert(aa_program_revision(program) == revision);

	jump->jump_target_identity = &fixture.outside_identity;
	assert_reconcile_rejected_unchanged(program, &fixture,
		AA_RESULT_UNRESOLVED_SYMBOL,
		AA_LEGACY_IMPORT_ISSUE_JUMP_TARGET_MISSING);
	jump->jump_target_identity = first_mov->identity;
	assert_reconcile_rejected_unchanged(program, &fixture,
		AA_RESULT_UNRESOLVED_SYMBOL,
		AA_LEGACY_IMPORT_ISSUE_JUMP_TARGET_NOT_LABEL);
	jump->jump_target_identity = label->identity;
	fixture.lines[1].bound_instruction_id = ids[0];
	assert_reconcile_rejected_unchanged(program, &fixture,
		AA_RESULT_INVALID_ARGUMENT,
		AA_LEGACY_IMPORT_ISSUE_IDENTITY_MISMATCH);
	fixture.lines[1].bound_instruction_id = AA_INSTRUCTION_ID_INVALID;
	assert_reconcile_rejected_unchanged(program, &fixture,
		AA_RESULT_INVALID_ARGUMENT,
		AA_LEGACY_IMPORT_ISSUE_IDENTITY_MISMATCH);
	fixture.lines[1].bound_instruction_id = ids[1];
	fixture.count--;
	assert_reconcile_rejected_unchanged(program, &fixture,
		AA_RESULT_INVALID_ARGUMENT,
		AA_LEGACY_IMPORT_ISSUE_IDENTITY_MISMATCH);
	fixture.count++;
	fixture.lines[0].bound_instruction_id = ids[1];
	assert_reconcile_rejected_unchanged(program, &fixture,
		AA_RESULT_INVALID_ARGUMENT,
		AA_LEGACY_IMPORT_ISSUE_IDENTITY_MISMATCH);
	fixture.lines[0].bound_instruction_id = ids[0];
	aa_legacy_line_snapshot_t swapped = fixture.lines[0];
	fixture.lines[0] = fixture.lines[1];
	fixture.lines[1] = swapped;
	assert_reconcile_rejected_unchanged(program, &fixture,
		AA_RESULT_INVALID_ARGUMENT,
		AA_LEGACY_IMPORT_ISSUE_IDENTITY_MISMATCH);
	fixture.lines[1] = fixture.lines[0];
	fixture.lines[0] = swapped;
	assert(duplicate_mov != first_mov && incomplete_jump->identity != NULL);
	aa_program_destroy(program);
}

static void test_reader_failure_and_snapshot_immutability(void)
{
	legacy_fixture_t fixture = {0};
	fixture.failed_read_position = 0;
	aa_legacy_line_snapshot_t *line = fixture_add(&fixture, LABEL);
	line->line_state = CHANGING_OP1;
	aa_legacy_line_snapshot_t before = *line;
	aa_program_t *program = aa_program_create();
	assert(program != NULL);
	aa_legacy_program_reader_t reader = fixture_reader(&fixture);
	aa_legacy_import_report_t report;
	assert(aa_legacy_program_import(program, &reader, &report) ==
		   AA_RESULT_INVALID_ARGUMENT);
	assert(report.issue == AA_LEGACY_IMPORT_ISSUE_READER_FAILED);
	assert(aa_program_count(program) == 0);
	assert(memcmp(&before, line, sizeof(before)) == 0);
	aa_program_destroy(program);
}

int main(void)
{
	test_empty_and_label_import();
	test_opcode_and_operand_mapping();
	test_all_legacy_immediates();
	test_incomplete_instructions_and_line_states();
	test_two_pass_jump_resolution_and_ids();
	test_import_failures_are_atomic();
	test_reader_failure_and_snapshot_immutability();
	test_identity_bound_reconciliation();
	puts("program adapter tests passed");
	return 0;
}
