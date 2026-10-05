#include "domain/program.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

struct aa_program {
	aa_instruction_t *instructions;
	size_t count;
	size_t capacity;
	aa_instruction_id_t next_id;
	uint64_t revision;
};

static size_t opcode_operand_count(aa_opcode_t opcode)
{
	switch (opcode) {
	case AA_OPCODE_MOV:
	case AA_OPCODE_ADD:
	case AA_OPCODE_CMP:
		return 2;
	case AA_OPCODE_JMP:
	case AA_OPCODE_JE:
	case AA_OPCODE_JNE:
		return 1;
	case AA_OPCODE_LABEL:
		return 0;
	default:
		return SIZE_MAX;
	}
}

static bool is_register(const aa_operand_t *operand)
{
	return operand->kind == AA_OPERAND_REGISTER &&
		operand->value.reg >= AA_REGISTER_RAX &&
		operand->value.reg <= AA_REGISTER_RDI;
}

static bool operand_value_is_valid(const aa_operand_t *operand)
{
	switch (operand->kind) {
	case AA_OPERAND_NONE:
		return true;
	case AA_OPERAND_REGISTER:
		return is_register(operand);
	case AA_OPERAND_BUFFER:
		return operand->value.buffer == AA_BUFFER_INPUT ||
			operand->value.buffer == AA_BUFFER_OUTPUT;
	case AA_OPERAND_IMMEDIATE:
		return true;
	case AA_OPERAND_LABEL_REFERENCE:
		return operand->value.label_instruction_id != AA_INSTRUCTION_ID_INVALID;
	default:
		return false;
	}
}

static bool operand_is_compatible(const aa_instruction_t *instruction,
								  size_t position,
								  const aa_operand_t *operand)
{
	if (operand->kind == AA_OPERAND_NONE) {
		return true;
	}
	if (instruction->opcode == AA_OPCODE_JMP ||
		instruction->opcode == AA_OPCODE_JE ||
		instruction->opcode == AA_OPCODE_JNE) {
		return position == 0 && operand->kind == AA_OPERAND_LABEL_REFERENCE;
	}
	if (position == 0) {
		if (is_register(operand)) {
			return true;
		}
	if (instruction->opcode != AA_OPCODE_MOV ||
			operand->kind != AA_OPERAND_BUFFER ||
			operand->value.buffer != AA_BUFFER_OUTPUT) {
			return false;
		}
		const aa_operand_t *source = &instruction->operands[1];
		return source->kind == AA_OPERAND_NONE || is_register(source);
	}
	if (position == 1) {
		if (operand->kind == AA_OPERAND_REGISTER) {
			return is_register(operand);
		}
		if (instruction->operands[0].kind != AA_OPERAND_REGISTER) {
			return false;
		}
		return operand->kind == AA_OPERAND_IMMEDIATE ||
			(operand->kind == AA_OPERAND_BUFFER &&
			 operand->value.buffer == AA_BUFFER_INPUT);
	}
	return false;
}

static aa_instruction_t *find_mutable_by_id(aa_program_t *program,
											 aa_instruction_id_t id)
{
	for (size_t position = 0; position < program->count; position++) {
		if (program->instructions[position].id == id) {
			return &program->instructions[position];
		}
	}
	return NULL;
}

aa_result_t aa_program_reserve(aa_program_t *program, size_t capacity)
{
	if (program == NULL) {
		return AA_RESULT_INVALID_ARGUMENT;
	}
	if (capacity <= program->capacity) {
		return AA_RESULT_OK;
	}
	size_t new_capacity = program->capacity == 0 ? 8 : program->capacity;
	while (new_capacity < capacity) {
		if (new_capacity > SIZE_MAX / 2) {
			new_capacity = capacity;
			break;
		}
		new_capacity *= 2;
	}
	if (new_capacity > SIZE_MAX / sizeof(*program->instructions)) {
		return AA_RESULT_ALLOCATION_FAILED;
	}
	aa_instruction_t *instructions = realloc(program->instructions,
											new_capacity * sizeof(*instructions));
	if (instructions == NULL) {
		return AA_RESULT_ALLOCATION_FAILED;
	}
	program->instructions = instructions;
	program->capacity = new_capacity;
	return AA_RESULT_OK;
}

static aa_result_t reserve_instruction(aa_program_t *program)
{
	if (program->count < program->capacity) {
		return AA_RESULT_OK;
	}
	if (program->count == SIZE_MAX) {
		return AA_RESULT_ALLOCATION_FAILED;
	}
	return aa_program_reserve(program, program->count + 1);
}

aa_program_t *aa_program_create(void)
{
	aa_program_t *program = calloc(1, sizeof(*program));
	if (program != NULL) {
		program->next_id = 1;
	}
	return program;
}

void aa_program_destroy(aa_program_t *program)
{
	if (program == NULL) {
		return;
	}
	free(program->instructions);
	free(program);
}

void aa_program_clear(aa_program_t *program)
{
	if (program != NULL && program->count != 0) {
		program->count = 0;
		program->revision++;
	}
}

size_t aa_program_count(const aa_program_t *program)
{
	return program == NULL ? 0 : program->count;
}

const aa_instruction_t *aa_program_instruction_at(const aa_program_t *program,
												 size_t position)
{
	if (program == NULL || position >= program->count) {
		return NULL;
	}
	return &program->instructions[position];
}

const aa_instruction_t *aa_program_find_by_id(const aa_program_t *program,
											 aa_instruction_id_t id)
{
	if (program == NULL || id == AA_INSTRUCTION_ID_INVALID) {
		return NULL;
	}
	for (size_t position = 0; position < program->count; position++) {
		if (program->instructions[position].id == id) {
			return &program->instructions[position];
		}
	}
	return NULL;
}

uint64_t aa_program_revision(const aa_program_t *program)
{
	return program == NULL ? 0 : program->revision;
}

aa_instruction_t aa_instruction_create(aa_opcode_t opcode)
{
	aa_instruction_t instruction = {0};
	instruction.opcode = opcode;
	size_t operand_count = opcode_operand_count(opcode);
	instruction.operand_count = operand_count == SIZE_MAX ? 0 : operand_count;
	return instruction;
}

aa_operand_t aa_operand_register(aa_register_t reg)
{
	aa_operand_t operand = {.kind = AA_OPERAND_REGISTER};
	operand.value.reg = reg;
	return operand;
}

aa_operand_t aa_operand_buffer(aa_buffer_t buffer)
{
	aa_operand_t operand = {.kind = AA_OPERAND_BUFFER};
	operand.value.buffer = buffer;
	return operand;
}

aa_operand_t aa_operand_immediate(int value)
{
	aa_operand_t operand = {.kind = AA_OPERAND_IMMEDIATE};
	operand.value.immediate = value;
	return operand;
}

aa_operand_t aa_operand_label_reference(aa_instruction_id_t label_id)
{
	aa_operand_t operand = {.kind = AA_OPERAND_LABEL_REFERENCE};
	operand.value.label_instruction_id = label_id;
	return operand;
}

static aa_result_t instruction_validate(const aa_instruction_t *instruction,
										   bool allow_missing_operands)
{
	if (instruction == NULL) {
		return AA_RESULT_INVALID_ARGUMENT;
	}
	size_t expected_count = opcode_operand_count(instruction->opcode);
	if (expected_count == SIZE_MAX) {
		return AA_RESULT_INVALID_OPCODE;
	}
	if (instruction->operand_count != expected_count) {
		return AA_RESULT_INVALID_OPERAND;
	}
	for (size_t position = 0; position < expected_count; position++) {
		const aa_operand_t *operand = &instruction->operands[position];
		if (!operand_value_is_valid(operand)) {
			return AA_RESULT_INVALID_OPERAND;
		}
		if (operand->kind == AA_OPERAND_NONE) {
			if (allow_missing_operands) {
				continue;
			}
			return AA_RESULT_INVALID_OPERAND;
		}
		if (!operand_is_compatible(instruction, position, operand)) {
			return AA_RESULT_INCOMPATIBLE_OPERAND;
		}
	}
	return AA_RESULT_OK;
}

aa_result_t aa_instruction_validate(const aa_instruction_t *instruction)
{
	return instruction_validate(instruction, false);
}

aa_result_t aa_program_insert(aa_program_t *program, size_t position,
							  const aa_instruction_t *instruction,
							  aa_instruction_id_t *created_id)
{
	if (program == NULL || instruction == NULL) {
		return AA_RESULT_INVALID_ARGUMENT;
	}
	if (position > program->count) {
		return AA_RESULT_OUT_OF_RANGE;
	}
	const aa_instruction_t candidate = *instruction;
	if (opcode_operand_count(candidate.opcode) == SIZE_MAX) {
		return AA_RESULT_INVALID_OPCODE;
	}
	if (candidate.operand_count != opcode_operand_count(candidate.opcode)) {
		return AA_RESULT_INVALID_OPERAND;
	}
	for (size_t operand_position = 0;
		 operand_position < candidate.operand_count; operand_position++) {
		const aa_operand_t *operand = &candidate.operands[operand_position];
		if (!operand_value_is_valid(operand)) {
			return AA_RESULT_INVALID_OPERAND;
		}
		if (!operand_is_compatible(&candidate, operand_position, operand)) {
			return AA_RESULT_INCOMPATIBLE_OPERAND;
		}
	}
	if (program->next_id == AA_INSTRUCTION_ID_INVALID) {
		return AA_RESULT_ID_EXHAUSTED;
	}
	aa_result_t result = reserve_instruction(program);
	if (result != AA_RESULT_OK) {
		return result;
	}
	memmove(&program->instructions[position + 1],
			&program->instructions[position],
			(program->count - position) * sizeof(*program->instructions));
	aa_instruction_t inserted = candidate;
	inserted.id = program->next_id++;
	program->instructions[position] = inserted;
	program->count++;
	program->revision++;
	if (created_id != NULL) {
		*created_id = inserted.id;
	}
	return AA_RESULT_OK;
}

aa_result_t aa_program_append(aa_program_t *program,
							  const aa_instruction_t *instruction,
							  aa_instruction_id_t *created_id)
{
	if (program == NULL) {
		return AA_RESULT_INVALID_ARGUMENT;
	}
	return aa_program_insert(program, program->count, instruction, created_id);
}

aa_result_t aa_program_remove(aa_program_t *program, size_t position)
{
	if (program == NULL) {
		return AA_RESULT_INVALID_ARGUMENT;
	}
	if (position >= program->count) {
		return AA_RESULT_OUT_OF_RANGE;
	}
	memmove(&program->instructions[position],
			&program->instructions[position + 1],
			(program->count - position - 1) * sizeof(*program->instructions));
	program->count--;
	program->revision++;
	return AA_RESULT_OK;
}

aa_result_t aa_program_remove_by_id(aa_program_t *program,
									aa_instruction_id_t id)
{
	if (program == NULL || id == AA_INSTRUCTION_ID_INVALID) {
		return AA_RESULT_INVALID_ARGUMENT;
	}
	for (size_t position = 0; position < program->count; position++) {
		if (program->instructions[position].id == id) {
			return aa_program_remove(program, position);
		}
	}
	return AA_RESULT_OUT_OF_RANGE;
}

aa_result_t aa_program_move(aa_program_t *program, size_t from, size_t to)
{
	if (program == NULL) {
		return AA_RESULT_INVALID_ARGUMENT;
	}
	if (from >= program->count || to >= program->count) {
		return AA_RESULT_OUT_OF_RANGE;
	}
	if (from == to) {
		return AA_RESULT_OK;
	}
	aa_instruction_t moved = program->instructions[from];
	if (from < to) {
		memmove(&program->instructions[from], &program->instructions[from + 1],
				(to - from) * sizeof(*program->instructions));
	} else {
		memmove(&program->instructions[to + 1], &program->instructions[to],
				(from - to) * sizeof(*program->instructions));
	}
	program->instructions[to] = moved;
	program->revision++;
	return AA_RESULT_OK;
}

aa_result_t aa_program_replace_instruction(aa_program_t *program,
										   aa_instruction_id_t id,
										   aa_opcode_t opcode)
{
	if (program == NULL || id == AA_INSTRUCTION_ID_INVALID) {
		return AA_RESULT_INVALID_ARGUMENT;
	}
	if (opcode_operand_count(opcode) == SIZE_MAX) {
		return AA_RESULT_INVALID_OPCODE;
	}
	aa_instruction_t *instruction = find_mutable_by_id(program, id);
	if (instruction == NULL) {
		return AA_RESULT_OUT_OF_RANGE;
	}
	if (instruction->opcode == opcode) {
		return AA_RESULT_OK;
	}
	*instruction = aa_instruction_create(opcode);
	instruction->id = id;
	program->revision++;
	return AA_RESULT_OK;
}

aa_result_t aa_program_set_operand(aa_program_t *program,
								   aa_instruction_id_t instruction_id,
								   size_t operand_position,
								   aa_operand_t operand)
{
	if (program == NULL || instruction_id == AA_INSTRUCTION_ID_INVALID) {
		return AA_RESULT_INVALID_ARGUMENT;
	}
	aa_instruction_t *instruction = find_mutable_by_id(program, instruction_id);
	if (instruction == NULL) {
		return AA_RESULT_OUT_OF_RANGE;
	}
	if (operand_position >= instruction->operand_count) {
		return AA_RESULT_OUT_OF_RANGE;
	}
	if (!operand_value_is_valid(&operand)) {
		return AA_RESULT_INVALID_OPERAND;
	}
	aa_operand_t previous = instruction->operands[operand_position];
	instruction->operands[operand_position] = operand;
	if (!operand_is_compatible(instruction, operand_position, &operand)) {
		instruction->operands[operand_position] = previous;
		return AA_RESULT_INCOMPATIBLE_OPERAND;
	}
	program->revision++;
	return AA_RESULT_OK;
}

aa_result_t aa_program_clear_operand(aa_program_t *program,
									 aa_instruction_id_t instruction_id,
									 size_t operand_position)
{
	if (program == NULL || instruction_id == AA_INSTRUCTION_ID_INVALID) {
		return AA_RESULT_INVALID_ARGUMENT;
	}
	aa_instruction_t *instruction = find_mutable_by_id(program, instruction_id);
	if (instruction == NULL) {
		return AA_RESULT_OUT_OF_RANGE;
	}
	if (operand_position >= instruction->operand_count) {
		return AA_RESULT_OUT_OF_RANGE;
	}
	memset(&instruction->operands[operand_position], 0,
		   sizeof(instruction->operands[operand_position]));
	program->revision++;
	return AA_RESULT_OK;
}

aa_result_t aa_program_clone_instruction(aa_program_t *program,
										 aa_instruction_id_t source_id,
										 size_t destination_position,
										 aa_instruction_id_t *created_id)
{
	if (program == NULL || source_id == AA_INSTRUCTION_ID_INVALID) {
		return AA_RESULT_INVALID_ARGUMENT;
	}
	if (destination_position > program->count) {
		return AA_RESULT_OUT_OF_RANGE;
	}
	const aa_instruction_t *source = aa_program_find_by_id(program, source_id);
	if (source == NULL) {
		return AA_RESULT_OUT_OF_RANGE;
	}
	aa_instruction_t copy = *source;
	return aa_program_insert(program, destination_position, &copy, created_id);
}

aa_result_t aa_program_replace_from(aa_program_t *destination,
										const aa_program_t *source)
{
	if (destination == NULL || source == NULL) {
		return AA_RESULT_INVALID_ARGUMENT;
	}
	if (destination == source) {
		return AA_RESULT_OK;
	}
	aa_validation_report_t validation = aa_program_validate_partial(source, NULL);
	if (validation.result != AA_RESULT_OK) {
		return validation.result;
	}
	if (source->count > SIZE_MAX / sizeof(*destination->instructions)) {
		return AA_RESULT_ALLOCATION_FAILED;
	}
	if (source->count != 0) {
		if (destination->next_id == AA_INSTRUCTION_ID_INVALID) {
			return AA_RESULT_ID_EXHAUSTED;
		}
		uint64_t available_ids =
			(uint64_t)UINT32_MAX - destination->next_id + 1;
		if ((uint64_t)source->count > available_ids) {
			return AA_RESULT_ID_EXHAUSTED;
		}
	}
	aa_instruction_t *replacement = NULL;
	if (source->count != 0) {
		replacement = malloc(source->count * sizeof(*replacement));
		if (replacement == NULL) {
			return AA_RESULT_ALLOCATION_FAILED;
		}
		memcpy(replacement, source->instructions,
			   source->count * sizeof(*replacement));
		aa_instruction_id_t first_id = destination->next_id;
		for (size_t position = 0; position < source->count; position++) {
			replacement[position].id = (aa_instruction_id_t)(first_id + position);
			for (size_t operand_position = 0;
				 operand_position < replacement[position].operand_count;
				 operand_position++) {
				aa_operand_t *operand =
					&replacement[position].operands[operand_position];
				if (operand->kind != AA_OPERAND_LABEL_REFERENCE) {
					continue;
				}
				const aa_instruction_t *target = aa_program_find_by_id(
					source, operand->value.label_instruction_id);
				if (target == NULL || target->opcode != AA_OPCODE_LABEL) {
					free(replacement);
					return AA_RESULT_UNRESOLVED_SYMBOL;
				}
				size_t target_position =
					(size_t)(target - source->instructions);
				operand->value.label_instruction_id =
					(aa_instruction_id_t)(first_id + target_position);
			}
		}
	}
	aa_instruction_id_t next_id = destination->next_id;
	if (source->count != 0) {
		uint64_t next = (uint64_t)next_id + source->count;
		next_id = next > UINT32_MAX ? AA_INSTRUCTION_ID_INVALID :
			(aa_instruction_id_t)next;
	}
	free(destination->instructions);
	destination->instructions = replacement;
	destination->count = source->count;
	destination->capacity = source->count;
	destination->next_id = next_id;
	destination->revision++;
	return AA_RESULT_OK;
}

static bool instructions_equal(const aa_instruction_t *left,
							  const aa_instruction_t *right)
{
	if (left->id != right->id || left->opcode != right->opcode ||
		left->operand_count != right->operand_count) {
		return false;
	}
	for (size_t position = 0; position < left->operand_count; position++) {
		const aa_operand_t *left_operand = &left->operands[position];
		const aa_operand_t *right_operand = &right->operands[position];
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
			if (left_operand->value.immediate != right_operand->value.immediate) {
				return false;
			}
			break;
		case AA_OPERAND_LABEL_REFERENCE:
			if (left_operand->value.label_instruction_id !=
				right_operand->value.label_instruction_id) {
				return false;
			}
			break;
		default:
			return false;
		}
	}
	return true;
}

aa_result_t aa_program_reconcile(aa_program_t *destination,
								 const aa_instruction_t *instructions,
								 size_t count)
{
	if (destination == NULL || (count != 0 && instructions == NULL)) {
		return AA_RESULT_INVALID_ARGUMENT;
	}
	if (count > SIZE_MAX / sizeof(*instructions)) {
		return AA_RESULT_ALLOCATION_FAILED;
	}
	if (destination->next_id == AA_INSTRUCTION_ID_INVALID) {
		return AA_RESULT_ID_EXHAUSTED;
	}
	aa_program_t candidate = {
		.instructions = (aa_instruction_t *)instructions,
		.count = count,
		.capacity = count
	};
	aa_instruction_id_t max_id = AA_INSTRUCTION_ID_INVALID;
	for (size_t position = 0; position < count; position++) {
		const aa_instruction_t *instruction = &instructions[position];
		if (instruction->id == AA_INSTRUCTION_ID_INVALID) {
			return AA_RESULT_INVALID_ARGUMENT;
		}
		for (size_t prior = 0; prior < position; prior++) {
			if (instructions[prior].id == instruction->id) {
				return AA_RESULT_INVALID_ARGUMENT;
			}
		}
		if (instruction->id > max_id) {
			max_id = instruction->id;
		}
	}
	if (max_id == UINT32_MAX) {
		return AA_RESULT_ID_EXHAUSTED;
	}
	aa_validation_report_t validation = aa_program_validate_partial(&candidate,
														 NULL);
	if (validation.result != AA_RESULT_OK) {
		return validation.result;
	}
	bool unchanged = destination->count == count;
	for (size_t position = 0; unchanged && position < count; position++) {
		unchanged = instructions_equal(&destination->instructions[position],
									   &instructions[position]);
	}
	uint64_t candidate_next = (uint64_t)max_id + 1;
	if (unchanged) {
		if (candidate_next > destination->next_id) {
			destination->next_id = (aa_instruction_id_t)candidate_next;
		}
		return AA_RESULT_OK;
	}
	aa_instruction_id_t next_id = destination->next_id;
	if (candidate_next > next_id) {
		next_id = (aa_instruction_id_t)candidate_next;
	}
	aa_instruction_t *replacement = NULL;
	if (count != 0) {
		replacement = malloc(count * sizeof(*replacement));
		if (replacement == NULL) {
			return AA_RESULT_ALLOCATION_FAILED;
		}
		memcpy(replacement, instructions, count * sizeof(*replacement));
	}
	free(destination->instructions);
	destination->instructions = replacement;
	destination->count = count;
	destination->capacity = count;
	destination->next_id = next_id;
	destination->revision++;
	return AA_RESULT_OK;
}

static aa_validation_report_t program_validate(const aa_program_t *program,
											  const aa_program_rules_t *rules,
											  bool allow_missing_operands)
{
	aa_validation_report_t report = {
		.result = AA_RESULT_OK,
		.instruction_id = AA_INSTRUCTION_ID_INVALID,
		.position = 0,
		.operand_position = SIZE_MAX
	};
	if (program == NULL) {
		report.result = AA_RESULT_INVALID_ARGUMENT;
		return report;
	}
	if (rules != NULL && rules->instruction_limit != 0 &&
		program->count > rules->instruction_limit) {
		report.result = AA_RESULT_INSTRUCTION_LIMIT_REACHED;
		report.position = rules->instruction_limit;
		report.instruction_id =
			program->instructions[report.position].id;
		return report;
	}
	for (size_t position = 0; position < program->count; position++) {
		const aa_instruction_t *instruction = &program->instructions[position];
		aa_result_t result = instruction_validate(instruction,
											 allow_missing_operands);
		if (result != AA_RESULT_OK) {
			report.result = result;
			report.instruction_id = instruction->id;
			report.position = position;
			for (size_t operand_position = 0;
				 operand_position < instruction->operand_count;
				 operand_position++) {
				const aa_operand_t *operand =
					&instruction->operands[operand_position];
				if (operand->kind == AA_OPERAND_NONE ||
					(result == AA_RESULT_INVALID_OPERAND &&
					 !operand_value_is_valid(operand)) ||
					(result == AA_RESULT_INCOMPATIBLE_OPERAND &&
					 !operand_is_compatible(instruction, operand_position,
										 operand))) {
					report.operand_position = operand_position;
					break;
				}
			}
			return report;
		}
		for (size_t operand_position = 0;
			 operand_position < instruction->operand_count;
			 operand_position++) {
			const aa_operand_t *operand = &instruction->operands[operand_position];
			if (operand->kind == AA_OPERAND_LABEL_REFERENCE) {
				const aa_instruction_t *label = aa_program_find_by_id(
					program, operand->value.label_instruction_id);
				if (label == NULL || label->opcode != AA_OPCODE_LABEL) {
					report.result = AA_RESULT_UNRESOLVED_SYMBOL;
					report.instruction_id = instruction->id;
					report.position = position;
					report.operand_position = operand_position;
					return report;
				}
			}
		}
	}
	return report;
}

aa_validation_report_t aa_program_validate(const aa_program_t *program,
											   const aa_program_rules_t *rules)
{
	return program_validate(program, rules, false);
}

aa_validation_report_t aa_program_validate_partial(
	const aa_program_t *program,
	const aa_program_rules_t *rules)
{
	return program_validate(program, rules, true);
}