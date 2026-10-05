#include "migration/legacy_program_adapter.h"

#include <stdint.h>
#include <stdlib.h>

#include "legacy_code_ids.h"

typedef struct aa_imported_line {
	aa_legacy_line_snapshot_t snapshot;
	aa_opcode_t opcode;
	aa_instruction_id_t instruction_id;
} aa_imported_line_t;

static void set_report(aa_legacy_import_report_t *report,
					   aa_result_t result,
					   aa_legacy_import_issue_t issue,
					   size_t position,
					   size_t operand_position);

static aa_result_t convert_legacy_opcode(int legacy_opcode,
										 aa_opcode_t *opcode)
{
	if (opcode == NULL) {
		return AA_RESULT_INVALID_ARGUMENT;
	}
	switch (legacy_opcode) {
	case MOV:
		*opcode = AA_OPCODE_MOV;
		break;
	case ADD:
		*opcode = AA_OPCODE_ADD;
		break;
	case LABEL:
		*opcode = AA_OPCODE_LABEL;
		break;
	case JMP:
		*opcode = AA_OPCODE_JMP;
		break;
	case CMP:
		*opcode = AA_OPCODE_CMP;
		break;
	case JE:
		*opcode = AA_OPCODE_JE;
		break;
	case JNE:
		*opcode = AA_OPCODE_JNE;
		break;
	default:
		return AA_RESULT_INVALID_OPCODE;
	}
	return AA_RESULT_OK;
}

static aa_result_t convert_legacy_operand(int legacy_operand,
										 aa_operand_t *operand)
{
	if (operand == NULL) {
		return AA_RESULT_INVALID_ARGUMENT;
	}
	switch (legacy_operand) {
	case RAX:
		*operand = aa_operand_register(AA_REGISTER_RAX);
		return AA_RESULT_OK;
	case RBX:
		*operand = aa_operand_register(AA_REGISTER_RBX);
		return AA_RESULT_OK;
	case RCX:
		*operand = aa_operand_register(AA_REGISTER_RCX);
		return AA_RESULT_OK;
	case RDX:
		*operand = aa_operand_register(AA_REGISTER_RDX);
		return AA_RESULT_OK;
	case RDI:
		*operand = aa_operand_register(AA_REGISTER_RDI);
		return AA_RESULT_OK;
	case IB:
		*operand = aa_operand_buffer(AA_BUFFER_INPUT);
		return AA_RESULT_OK;
	case OB:
		*operand = aa_operand_buffer(AA_BUFFER_OUTPUT);
		return AA_RESULT_OK;
	default:
		break;
	}
	if (legacy_operand >= IMMUP0 && legacy_operand <= IMMUP10) {
		*operand = aa_operand_immediate(legacy_operand - IMMUP0);
		return AA_RESULT_OK;
	}
	if (legacy_operand >= IMMUP_1 && legacy_operand <= IMMUP_9) {
		*operand = aa_operand_immediate(-(legacy_operand - IMMUP_1 + 1));
		return AA_RESULT_OK;
	}
	if (legacy_operand >= IMMDO0 && legacy_operand <= IMMDO9) {
		*operand = aa_operand_immediate(-(10 + legacy_operand - IMMDO0));
		return AA_RESULT_OK;
	}
	return AA_RESULT_INVALID_OPERAND;
}

aa_result_t aa_legacy_instruction_from_snapshot(
	const aa_legacy_line_snapshot_t *snapshot,
	aa_instruction_t *instruction,
	aa_legacy_import_report_t *report)
{
	set_report(report, AA_RESULT_OK, AA_LEGACY_IMPORT_ISSUE_NONE, 0, SIZE_MAX);
	if (snapshot == NULL || instruction == NULL) {
		set_report(report, AA_RESULT_INVALID_ARGUMENT,
				   AA_LEGACY_IMPORT_ISSUE_INVALID_LINE, 0, SIZE_MAX);
		return AA_RESULT_INVALID_ARGUMENT;
	}
	aa_opcode_t opcode;
	aa_result_t result = convert_legacy_opcode(snapshot->opcode, &opcode);
	if (result != AA_RESULT_OK) {
		set_report(report, result, AA_LEGACY_IMPORT_ISSUE_UNKNOWN_OPCODE,
				   0, SIZE_MAX);
		return result;
	}
	if (opcode == AA_OPCODE_LABEL || opcode == AA_OPCODE_JMP ||
		opcode == AA_OPCODE_JE || opcode == AA_OPCODE_JNE ||
		snapshot->jump_target_identity != NULL) {
		set_report(report, AA_RESULT_INVALID_ARGUMENT,
				   AA_LEGACY_IMPORT_ISSUE_INVALID_LINE, 0, SIZE_MAX);
		return AA_RESULT_INVALID_ARGUMENT;
	}
	aa_instruction_t converted = aa_instruction_create(opcode);
	if (snapshot->has_operand_1) {
		result = convert_legacy_operand(snapshot->operand_1,
										&converted.operands[0]);
		if (result != AA_RESULT_OK) {
			set_report(report, result,
					   AA_LEGACY_IMPORT_ISSUE_UNKNOWN_OPERAND, 0, 0);
			return result;
		}
	}
	if (opcode == AA_OPCODE_MOV || opcode == AA_OPCODE_ADD ||
		opcode == AA_OPCODE_CMP) {
		if (snapshot->has_operand_2) {
			result = convert_legacy_operand(snapshot->operand_2,
											&converted.operands[1]);
			if (result != AA_RESULT_OK) {
				set_report(report, result,
						   AA_LEGACY_IMPORT_ISSUE_UNKNOWN_OPERAND, 0, 1);
				return result;
			}
		}
	} else if (snapshot->has_operand_1) {
		set_report(report, AA_RESULT_INVALID_OPERAND,
				   AA_LEGACY_IMPORT_ISSUE_INVALID_LINE, 0, 0);
		return AA_RESULT_INVALID_OPERAND;
	}
	result = aa_instruction_validate(&converted);
	if (result != AA_RESULT_OK && result != AA_RESULT_INVALID_OPERAND) {
		set_report(report, result, AA_LEGACY_IMPORT_ISSUE_DOMAIN_REJECTED,
				   0, SIZE_MAX);
		return result;
	}
	*instruction = converted;
	return AA_RESULT_OK;
}

static void set_report(aa_legacy_import_report_t *report,
					   aa_result_t result,
					   aa_legacy_import_issue_t issue,
					   size_t position,
					   size_t operand_position)
{
	if (report == NULL) {
		return;
	}
	report->result = result;
	report->issue = issue;
	report->position = position;
	report->operand_position = operand_position;
}

static size_t find_line_by_identity(const aa_imported_line_t *lines,
								   size_t count,
								   const void *identity)
{
	for (size_t position = 0; position < count; position++) {
		if (lines[position].snapshot.identity == identity) {
			return position;
		}
	}
	return SIZE_MAX;
}

static size_t program_position_for_id(const aa_program_t *program,
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
				if (program_position_for_id(
						left, left_operand->value.label_instruction_id) !=
					program_position_for_id(
						right, right_operand->value.label_instruction_id)) {
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

aa_result_t aa_legacy_program_import(
	aa_program_t *destination,
	const aa_legacy_program_reader_t *reader,
	aa_legacy_import_report_t *report)
{
	set_report(report, AA_RESULT_OK, AA_LEGACY_IMPORT_ISSUE_NONE, 0, SIZE_MAX);
	if (destination == NULL || reader == NULL || reader->count == NULL ||
		reader->read_line == NULL) {
		set_report(report, AA_RESULT_INVALID_ARGUMENT,
				   AA_LEGACY_IMPORT_ISSUE_INVALID_READER, 0, SIZE_MAX);
		return AA_RESULT_INVALID_ARGUMENT;
	}
	size_t count = reader->count(reader->context);
	if (count > SIZE_MAX / sizeof(aa_imported_line_t)) {
		set_report(report, AA_RESULT_ALLOCATION_FAILED,
				   AA_LEGACY_IMPORT_ISSUE_DOMAIN_REJECTED, 0, SIZE_MAX);
		return AA_RESULT_ALLOCATION_FAILED;
	}
	aa_imported_line_t *lines = NULL;
	aa_program_t *temporary = NULL;
	aa_result_t result = AA_RESULT_OK;
	if (count != 0) {
		lines = calloc(count, sizeof(*lines));
		if (lines == NULL) {
			result = AA_RESULT_ALLOCATION_FAILED;
			set_report(report, result,
					   AA_LEGACY_IMPORT_ISSUE_DOMAIN_REJECTED, 0, SIZE_MAX);
			goto cleanup;
		}
	}
	for (size_t position = 0; position < count; position++) {
		if (!reader->read_line(reader->context, position,
							  &lines[position].snapshot)) {
			result = AA_RESULT_INVALID_ARGUMENT;
			set_report(report, result, AA_LEGACY_IMPORT_ISSUE_READER_FAILED,
					   position, SIZE_MAX);
			goto cleanup;
		}
		if (lines[position].snapshot.identity == NULL) {
			result = AA_RESULT_INVALID_ARGUMENT;
			set_report(report, result, AA_LEGACY_IMPORT_ISSUE_INVALID_LINE,
					   position, SIZE_MAX);
			goto cleanup;
		}
		for (size_t prior = 0; prior < position; prior++) {
			if (lines[prior].snapshot.identity ==
				lines[position].snapshot.identity) {
				result = AA_RESULT_INVALID_ARGUMENT;
				set_report(report, result,
						   AA_LEGACY_IMPORT_ISSUE_DUPLICATE_IDENTITY,
						   position, SIZE_MAX);
				goto cleanup;
			}
		}
		result = convert_legacy_opcode(lines[position].snapshot.opcode,
									   &lines[position].opcode);
		if (result != AA_RESULT_OK) {
			set_report(report, result, AA_LEGACY_IMPORT_ISSUE_UNKNOWN_OPCODE,
					   position, SIZE_MAX);
			goto cleanup;
		}
	}
	temporary = aa_program_create();
	if (temporary == NULL) {
		result = AA_RESULT_ALLOCATION_FAILED;
		set_report(report, result, AA_LEGACY_IMPORT_ISSUE_DOMAIN_REJECTED,
				   0, SIZE_MAX);
		goto cleanup;
	}
	for (size_t position = 0; position < count; position++) {
		const aa_legacy_line_snapshot_t *snapshot =
			&lines[position].snapshot;
		aa_instruction_t instruction =
			aa_instruction_create(lines[position].opcode);
		bool is_jump = lines[position].opcode == AA_OPCODE_JMP ||
			lines[position].opcode == AA_OPCODE_JE ||
			lines[position].opcode == AA_OPCODE_JNE;
		if (snapshot->has_operand_2 &&
			(lines[position].opcode == AA_OPCODE_LABEL || is_jump)) {
			result = AA_RESULT_INVALID_OPERAND;
			set_report(report, result, AA_LEGACY_IMPORT_ISSUE_INVALID_LINE,
					   position, 1);
			goto cleanup;
		}
		if (!is_jump && snapshot->jump_target_identity != NULL) {
			result = AA_RESULT_INVALID_OPERAND;
			set_report(report, result, AA_LEGACY_IMPORT_ISSUE_INVALID_LINE,
					   position, 0);
			goto cleanup;
		}
		if (is_jump && !snapshot->has_operand_1 &&
			snapshot->jump_target_identity != NULL) {
			result = AA_RESULT_INVALID_OPERAND;
			set_report(report, result, AA_LEGACY_IMPORT_ISSUE_INVALID_LINE,
					   position, 0);
			goto cleanup;
		}
		if (lines[position].opcode != AA_OPCODE_LABEL && !is_jump) {
			if (snapshot->has_operand_1) {
				result = convert_legacy_operand(snapshot->operand_1,
											   &instruction.operands[0]);
				if (result != AA_RESULT_OK) {
					set_report(report, result,
							   AA_LEGACY_IMPORT_ISSUE_UNKNOWN_OPERAND,
							   position, 0);
					goto cleanup;
				}
			}
			if (snapshot->has_operand_2) {
				result = convert_legacy_operand(snapshot->operand_2,
											   &instruction.operands[1]);
				if (result != AA_RESULT_OK) {
					set_report(report, result,
							   AA_LEGACY_IMPORT_ISSUE_UNKNOWN_OPERAND,
							   position, 1);
					goto cleanup;
				}
			}
		}
		result = aa_program_insert(temporary, position, &instruction,
									   &lines[position].instruction_id);
		if (result != AA_RESULT_OK) {
			set_report(report, result,
					   AA_LEGACY_IMPORT_ISSUE_DOMAIN_REJECTED,
					   position, SIZE_MAX);
			goto cleanup;
		}
	}
	for (size_t position = 0; position < count; position++) {
		if (lines[position].opcode != AA_OPCODE_JMP &&
			lines[position].opcode != AA_OPCODE_JE &&
			lines[position].opcode != AA_OPCODE_JNE) {
			continue;
		}
		const void *target_identity =
			lines[position].snapshot.jump_target_identity;
		if (!lines[position].snapshot.has_operand_1) {
			continue;
		}
		size_t target_position = find_line_by_identity(lines, count,
													  target_identity);
		if (target_identity == NULL || target_position == SIZE_MAX) {
			result = AA_RESULT_UNRESOLVED_SYMBOL;
			set_report(report, result,
					   AA_LEGACY_IMPORT_ISSUE_JUMP_TARGET_MISSING,
					   position, 0);
			goto cleanup;
		}
		if (lines[target_position].opcode != AA_OPCODE_LABEL) {
			result = AA_RESULT_UNRESOLVED_SYMBOL;
			set_report(report, result,
					   AA_LEGACY_IMPORT_ISSUE_JUMP_TARGET_NOT_LABEL,
					   position, 0);
			goto cleanup;
		}
		aa_operand_t target = aa_operand_label_reference(
			lines[target_position].instruction_id);
		result = aa_program_set_operand(temporary,
										lines[position].instruction_id, 0, target);
		if (result != AA_RESULT_OK) {
			set_report(report, result,
					   AA_LEGACY_IMPORT_ISSUE_DOMAIN_REJECTED,
					   position, 0);
		goto cleanup;
		}
	}
	aa_validation_report_t validation =
		aa_program_validate_partial(temporary, NULL);
	if (validation.result != AA_RESULT_OK) {
		result = validation.result;
		set_report(report, result,
				   AA_LEGACY_IMPORT_ISSUE_DOMAIN_REJECTED,
				   validation.position, validation.operand_position);
		goto cleanup;
	}
	if (!programs_semantically_equal(destination, temporary)) {
		result = aa_program_replace_from(destination, temporary);
	}
	if (result != AA_RESULT_OK) {
		set_report(report, result,
				   AA_LEGACY_IMPORT_ISSUE_DOMAIN_REJECTED, 0, SIZE_MAX);
	}

cleanup:
	aa_program_destroy(temporary);
	free(lines);
	return result;
}

aa_result_t aa_legacy_program_reconcile(
	aa_program_t *destination,
	const aa_legacy_program_reader_t *reader,
	aa_legacy_import_report_t *report)
{
	set_report(report, AA_RESULT_OK, AA_LEGACY_IMPORT_ISSUE_NONE, 0, SIZE_MAX);
	if (destination == NULL || reader == NULL || reader->count == NULL ||
		reader->read_line == NULL) {
		set_report(report, AA_RESULT_INVALID_ARGUMENT,
				   AA_LEGACY_IMPORT_ISSUE_INVALID_READER, 0, SIZE_MAX);
		return AA_RESULT_INVALID_ARGUMENT;
	}
	size_t count = reader->count(reader->context);
	if (count != aa_program_count(destination)) {
		set_report(report, AA_RESULT_INVALID_ARGUMENT,
				   AA_LEGACY_IMPORT_ISSUE_IDENTITY_MISMATCH, 0, SIZE_MAX);
		return AA_RESULT_INVALID_ARGUMENT;
	}
	if (count > SIZE_MAX / sizeof(aa_imported_line_t) ||
		count > SIZE_MAX / sizeof(aa_instruction_t)) {
		set_report(report, AA_RESULT_ALLOCATION_FAILED,
				   AA_LEGACY_IMPORT_ISSUE_DOMAIN_REJECTED, 0, SIZE_MAX);
		return AA_RESULT_ALLOCATION_FAILED;
	}
	aa_imported_line_t *lines = count == 0 ? NULL : calloc(count, sizeof(*lines));
	aa_instruction_t *candidate = count == 0 ? NULL :
		calloc(count, sizeof(*candidate));
	if (count != 0 && (lines == NULL || candidate == NULL)) {
		free(lines);
		free(candidate);
		set_report(report, AA_RESULT_ALLOCATION_FAILED,
				   AA_LEGACY_IMPORT_ISSUE_DOMAIN_REJECTED, 0, SIZE_MAX);
		return AA_RESULT_ALLOCATION_FAILED;
	}
	aa_result_t result = AA_RESULT_OK;
	for (size_t position = 0; position < count; position++) {
		if (!reader->read_line(reader->context, position,
							  &lines[position].snapshot)) {
			result = AA_RESULT_INVALID_ARGUMENT;
			set_report(report, result, AA_LEGACY_IMPORT_ISSUE_READER_FAILED,
					   position, SIZE_MAX);
			goto cleanup_reconcile;
		}
		aa_legacy_line_snapshot_t *snapshot = &lines[position].snapshot;
		const aa_instruction_t *bound = aa_program_instruction_at(destination,
																  position);
		if (snapshot->identity == NULL ||
			snapshot->bound_instruction_id == AA_INSTRUCTION_ID_INVALID ||
			bound == NULL || bound->id != snapshot->bound_instruction_id ||
			aa_program_find_by_id(destination,
								 snapshot->bound_instruction_id) == NULL) {
			result = AA_RESULT_INVALID_ARGUMENT;
			set_report(report, result,
					   AA_LEGACY_IMPORT_ISSUE_IDENTITY_MISMATCH,
					   position, SIZE_MAX);
			goto cleanup_reconcile;
		}
		for (size_t prior = 0; prior < position; prior++) {
			if (lines[prior].snapshot.identity == snapshot->identity ||
				lines[prior].snapshot.bound_instruction_id ==
					snapshot->bound_instruction_id) {
				result = AA_RESULT_INVALID_ARGUMENT;
				set_report(report, result,
						   AA_LEGACY_IMPORT_ISSUE_IDENTITY_MISMATCH,
						   position, SIZE_MAX);
				goto cleanup_reconcile;
			}
		}
		result = convert_legacy_opcode(snapshot->opcode,
									   &lines[position].opcode);
		if (result != AA_RESULT_OK) {
			set_report(report, result, AA_LEGACY_IMPORT_ISSUE_UNKNOWN_OPCODE,
					   position, SIZE_MAX);
			goto cleanup_reconcile;
		}
		candidate[position] = aa_instruction_create(lines[position].opcode);
		candidate[position].id = snapshot->bound_instruction_id;
		bool is_jump = lines[position].opcode == AA_OPCODE_JMP ||
			lines[position].opcode == AA_OPCODE_JE ||
			lines[position].opcode == AA_OPCODE_JNE;
		if (snapshot->has_operand_2 &&
			(lines[position].opcode == AA_OPCODE_LABEL || is_jump)) {
			result = AA_RESULT_INVALID_OPERAND;
			set_report(report, result, AA_LEGACY_IMPORT_ISSUE_INVALID_LINE,
					   position, 1);
			goto cleanup_reconcile;
		}
		if (!is_jump && snapshot->jump_target_identity != NULL) {
			result = AA_RESULT_INVALID_OPERAND;
			set_report(report, result, AA_LEGACY_IMPORT_ISSUE_INVALID_LINE,
					   position, 0);
			goto cleanup_reconcile;
		}
		if (is_jump && !snapshot->has_operand_1 &&
			snapshot->jump_target_identity != NULL) {
			result = AA_RESULT_INVALID_OPERAND;
			set_report(report, result, AA_LEGACY_IMPORT_ISSUE_INVALID_LINE,
					   position, 0);
			goto cleanup_reconcile;
		}
		if (!is_jump && lines[position].opcode != AA_OPCODE_LABEL) {
			result = aa_legacy_instruction_from_snapshot(snapshot,
												 &candidate[position], report);
			if (result != AA_RESULT_OK) {
				if (report != NULL) {
					report->position = position;
				}
				goto cleanup_reconcile;
			}
			candidate[position].id = snapshot->bound_instruction_id;
		}
	}
	for (size_t position = 0; position < count; position++) {
		if (lines[position].opcode != AA_OPCODE_JMP &&
			lines[position].opcode != AA_OPCODE_JE &&
			lines[position].opcode != AA_OPCODE_JNE) {
			continue;
		}
		const aa_legacy_line_snapshot_t *snapshot = &lines[position].snapshot;
		if (!snapshot->has_operand_1) {
			continue;
		}
		size_t target_position = find_line_by_identity(
			lines, count, snapshot->jump_target_identity);
		if (snapshot->jump_target_identity == NULL ||
			target_position == SIZE_MAX) {
			result = AA_RESULT_UNRESOLVED_SYMBOL;
			set_report(report, result,
					   AA_LEGACY_IMPORT_ISSUE_JUMP_TARGET_MISSING,
					   position, 0);
			goto cleanup_reconcile;
		}
		if (lines[target_position].opcode != AA_OPCODE_LABEL) {
			result = AA_RESULT_UNRESOLVED_SYMBOL;
			set_report(report, result,
					   AA_LEGACY_IMPORT_ISSUE_JUMP_TARGET_NOT_LABEL,
					   position, 0);
			goto cleanup_reconcile;
		}
		candidate[position].operands[0] = aa_operand_label_reference(
			lines[target_position].snapshot.bound_instruction_id);
	}
	result = aa_program_reconcile(destination, candidate, count);
	if (result != AA_RESULT_OK) {
		set_report(report, result, AA_LEGACY_IMPORT_ISSUE_DOMAIN_REJECTED,
				   0, SIZE_MAX);
	}

cleanup_reconcile:
	free(candidate);
	free(lines);
	return result;
}