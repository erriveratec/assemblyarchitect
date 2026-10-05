#ifndef AA_MIGRATION_LEGACY_PROGRAM_ADAPTER_H
#define AA_MIGRATION_LEGACY_PROGRAM_ADAPTER_H

#include <stdbool.h>
#include <stddef.h>

#include "domain/program.h"

typedef struct aa_legacy_line_snapshot {
	const void *identity;
	int opcode;
	bool has_operand_1;
	int operand_1;
	bool has_operand_2;
	int operand_2;
	const void *jump_target_identity;
	int line_state;
} aa_legacy_line_snapshot_t;

typedef struct aa_legacy_program_reader {
	size_t (*count)(const void *context);
	bool (*read_line)(const void *context, size_t position,
					  aa_legacy_line_snapshot_t *line);
	const void *context;
} aa_legacy_program_reader_t;

typedef enum aa_legacy_import_issue {
	AA_LEGACY_IMPORT_ISSUE_NONE = 0,
	AA_LEGACY_IMPORT_ISSUE_INVALID_READER,
	AA_LEGACY_IMPORT_ISSUE_READER_FAILED,
	AA_LEGACY_IMPORT_ISSUE_DUPLICATE_IDENTITY,
	AA_LEGACY_IMPORT_ISSUE_UNKNOWN_OPCODE,
	AA_LEGACY_IMPORT_ISSUE_UNKNOWN_OPERAND,
	AA_LEGACY_IMPORT_ISSUE_INVALID_LINE,
	AA_LEGACY_IMPORT_ISSUE_JUMP_TARGET_MISSING,
	AA_LEGACY_IMPORT_ISSUE_JUMP_TARGET_NOT_LABEL,
	AA_LEGACY_IMPORT_ISSUE_DOMAIN_REJECTED
} aa_legacy_import_issue_t;

typedef struct aa_legacy_import_report {
	aa_result_t result;
	aa_legacy_import_issue_t issue;
	size_t position;
	size_t operand_position;
} aa_legacy_import_report_t;

/* The reader and snapshot identities are borrowed only for this synchronous
 * call. No legacy pointer or presentation state is retained by the program.
 */
aa_result_t aa_legacy_program_import(
	aa_program_t *destination,
	const aa_legacy_program_reader_t *reader,
	aa_legacy_import_report_t *report);

/* Converts a non-label, non-jump line snapshot without importing a program.
 * Editor presentation state is ignored; absent operands remain AA_OPERAND_NONE.
 */
aa_result_t aa_legacy_instruction_from_snapshot(
	const aa_legacy_line_snapshot_t *snapshot,
	aa_instruction_t *instruction,
	aa_legacy_import_report_t *report);

#endif