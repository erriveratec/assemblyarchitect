#ifndef AA_DOMAIN_PROGRAM_H
#define AA_DOMAIN_PROGRAM_H

#include <stddef.h>
#include <stdint.h>

typedef uint32_t aa_instruction_id_t;

#define AA_INSTRUCTION_ID_INVALID ((aa_instruction_id_t)0)

typedef enum aa_opcode {
	AA_OPCODE_INVALID = 0,
	AA_OPCODE_MOV,
	AA_OPCODE_ADD,
	AA_OPCODE_LABEL,
	AA_OPCODE_JMP,
	AA_OPCODE_CMP,
	AA_OPCODE_JE,
	AA_OPCODE_JNE
} aa_opcode_t;

/* LABEL has no semantic operand; its stable instruction ID is its symbol ID.
 * Display numbering is derived from program order, and removing or replacing
 * a referenced label leaves a reference that whole-program validation rejects.
 */

typedef enum aa_operand_kind {
	AA_OPERAND_NONE = 0,
	AA_OPERAND_REGISTER,
	AA_OPERAND_BUFFER,
	AA_OPERAND_IMMEDIATE,
	AA_OPERAND_LABEL_REFERENCE
} aa_operand_kind_t;

typedef enum aa_register {
	AA_REGISTER_INVALID = 0,
	AA_REGISTER_RAX,
	AA_REGISTER_RBX,
	AA_REGISTER_RCX,
	AA_REGISTER_RDX,
	AA_REGISTER_RDI
} aa_register_t;

typedef enum aa_buffer {
	AA_BUFFER_INVALID = 0,
	AA_BUFFER_INPUT,
	AA_BUFFER_OUTPUT
} aa_buffer_t;

typedef struct aa_operand {
	aa_operand_kind_t kind;
	union {
		aa_register_t reg;
		aa_buffer_t buffer;
		int immediate;
		aa_instruction_id_t label_instruction_id;
	} value;
} aa_operand_t;

typedef struct aa_instruction {
	aa_instruction_id_t id;
	aa_opcode_t opcode;
	aa_operand_t operands[2];
	size_t operand_count;
} aa_instruction_t;

typedef enum aa_result {
	AA_RESULT_OK = 0,
	AA_RESULT_INVALID_ARGUMENT,
	AA_RESULT_OUT_OF_RANGE,
	AA_RESULT_ALLOCATION_FAILED,
	AA_RESULT_INVALID_OPCODE,
	AA_RESULT_INVALID_OPERAND,
	AA_RESULT_INCOMPATIBLE_OPERAND,
	AA_RESULT_INSTRUCTION_LIMIT_REACHED,
	AA_RESULT_UNRESOLVED_SYMBOL,
	AA_RESULT_ID_EXHAUSTED
} aa_result_t;

typedef struct aa_program aa_program_t;

typedef struct aa_program_rules {
	size_t instruction_limit;
} aa_program_rules_t;

typedef struct aa_validation_report {
	aa_result_t result;
	aa_instruction_id_t instruction_id;
	size_t position;
	size_t operand_position;
} aa_validation_report_t;

/*
 * Programs are allocated and destroyed by the caller. Insert and append copy
 * the supplied instruction and assign its stable ID. Query pointers are
 * borrowed and become invalid after any successful program mutation or when
 * the program is destroyed. Creation reports allocation failure with NULL;
 * mutation allocation failures return AA_RESULT_ALLOCATION_FAILED.
 */
aa_program_t *aa_program_create(void);
void aa_program_destroy(aa_program_t *program);
void aa_program_clear(aa_program_t *program);
/*
 * Reserves capacity without changing program contents, IDs, or revision.
 * A successful reserve may invalidate borrowed instruction pointers. A NULL
 * program returns AA_RESULT_INVALID_ARGUMENT; unrepresentable or unavailable
 * capacity returns AA_RESULT_ALLOCATION_FAILED without changing the program.
 */
aa_result_t aa_program_reserve(aa_program_t *program, size_t capacity);
size_t aa_program_count(const aa_program_t *program);
const aa_instruction_t *aa_program_instruction_at(const aa_program_t *program,
												 size_t position);
const aa_instruction_t *aa_program_find_by_id(const aa_program_t *program,
											 aa_instruction_id_t id);
uint64_t aa_program_revision(const aa_program_t *program);

aa_instruction_t aa_instruction_create(aa_opcode_t opcode);
aa_operand_t aa_operand_register(aa_register_t reg);
aa_operand_t aa_operand_buffer(aa_buffer_t buffer);
aa_operand_t aa_operand_immediate(int value);
aa_operand_t aa_operand_label_reference(aa_instruction_id_t label_id);

aa_result_t aa_program_insert(aa_program_t *program, size_t position,
							  const aa_instruction_t *instruction,
							  aa_instruction_id_t *created_id);
aa_result_t aa_program_append(aa_program_t *program,
							  const aa_instruction_t *instruction,
							  aa_instruction_id_t *created_id);
aa_result_t aa_program_remove(aa_program_t *program, size_t position);
aa_result_t aa_program_remove_by_id(aa_program_t *program,
									aa_instruction_id_t id);
aa_result_t aa_program_move(aa_program_t *program, size_t from, size_t to);
aa_result_t aa_program_replace_instruction(aa_program_t *program,
										   aa_instruction_id_t id,
										   aa_opcode_t opcode);
aa_result_t aa_program_set_operand(aa_program_t *program,
								   aa_instruction_id_t instruction_id,
								   size_t operand_position,
								   aa_operand_t operand);
aa_result_t aa_program_clear_operand(aa_program_t *program,
									 aa_instruction_id_t instruction_id,
									 size_t operand_position);
aa_result_t aa_program_clone_instruction(aa_program_t *program,
										 aa_instruction_id_t source_id,
										 size_t destination_position,
										 aa_instruction_id_t *created_id);
/* Atomically copies source into destination, assigning fresh destination IDs
 * and remapping label references. Self-replacement is a no-op; a successful
 * replacement invalidates borrowed pointers and increments destination revision.
 */
aa_result_t aa_program_replace_from(aa_program_t *destination,
										const aa_program_t *source);
/* Transactionally replaces semantic contents using caller-supplied stable IDs.
 * This identity-preserving reconciliation API is not a normal editor mutation:
 * IDs must be nonzero and unique, and all label references must resolve to
 * LABEL instructions. The input array is borrowed for the call only, may point
 * into the destination's borrowed instruction storage, and is copied before
 * replacement. Revision changes once only when semantics or ordering change;
 * next_id never moves backward and remains available for future appends. */
aa_result_t aa_program_reconcile(aa_program_t *destination,
								 const aa_instruction_t *instructions,
								 size_t count);

aa_result_t aa_instruction_validate(const aa_instruction_t *instruction);
aa_validation_report_t aa_program_validate(const aa_program_t *program,
											   const aa_program_rules_t *rules);
aa_validation_report_t aa_program_validate_partial(
	const aa_program_t *program,
	const aa_program_rules_t *rules);

#endif