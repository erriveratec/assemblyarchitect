#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "aux.h"
#include "buffers_bf.h"
#include "code_window_cw.h"
#include "instruction_window_iw.h"
#include "level_config.h"
#include "levels_lv.h"
#include "registers_rg.h"
#include "stage_buttons_sb.h"
#include "immediates_im.h"
#include "gameplay/win_condition_wc.h"
#include "gameplay/interaction_rules_ir.h"
#include "gameplay/level_presentation_lp.h"

#define LEVEL_CONFIG_PATH_FORMAT "data/levels/%02d/level.cfg"
#define LINE_SIZE 512

typedef struct level_config_t {
	char challenge[LINE_SIZE];
	char instructions[128];
	char registers[128];
	char input_type[32];
	char input_modifier[32];
	char win_type[32];
	int  input_count;
	int  input_arg1;
	int  input_arg2;
	int  input_arg3;
	int  instruction_limit;
	int  win_arg1;
	int  win_arg2;
	int  win_arg3;
	bool win_flag;
	bool step_controls_enabled;
	bool immediates_visible;
	bool step_controls_enabled_set;
	bool immediates_visible_set;
	bool win_arg1_set;
	bool win_arg2_set;
	bool win_arg3_set;
	bool win_flag_set;
	bool register_hints_enabled;
	bool buffer_hints_enabled;
	bool immediate_hints_enabled;
	bool flag_boxes_visible;

	bool register_hints_enabled_set;
	bool buffer_hints_enabled_set;
	bool immediate_hints_enabled_set;
	bool flag_boxes_visible_set;
} level_config_t;

static char *trim(char *text)
{
	while (isspace((unsigned char)*text))
		text++;
	char *end = text + strlen(text);
	while (end > text && isspace((unsigned char)end[-1]))
		end--;
	*end = '\0';
	return text;
}

static bool parse_bool(const char *text) { return strcmp(text, "true") == 0; }

static bool validate_ui_properties(int level_id, const level_config_t *config)
{
	if (config == NULL) {
		fprintf(stderr, "level.cfg: level %d has no configuration\n", level_id);
		return false;
	}

	if (!config->step_controls_enabled_set) {
		fprintf(stderr,
		        "level.cfg: level %d is missing "
		        "ui.step_controls_enabled\n",
		        level_id);
		return false;
	}

	if (!config->immediates_visible_set) {
		fprintf(stderr,
		        "level.cfg: level %d is missing "
		        "ui.immediates_visible\n",
		        level_id);
		return false;
	}

	if (!config->register_hints_enabled_set) {
		fprintf(stderr,
		        "level.cfg: level %d is missing "
		        "ui.register_hints_enabled\n",
		        level_id);
		return false;
	}

	if (!config->buffer_hints_enabled_set) {
		fprintf(stderr,
		        "level.cfg: level %d is missing "
		        "ui.buffer_hints_enabled\n",
		        level_id);
		return false;
	}

	if (!config->immediate_hints_enabled_set) {
		fprintf(stderr,
		        "level.cfg: level %d is missing "
		        "ui.immediate_hints_enabled\n",
		        level_id);
		return false;
	}

	if (!config->flag_boxes_visible_set) {
		fprintf(stderr,
		        "level.cfg: level %d is missing "
		        "ui.flag_boxes_visible\n",
		        level_id);
		return false;
	}

	return true;
}

static bool validate_win_condition(int level_id, const level_config_t *config)
{
	const char *required[4]    = {NULL};
	bool        present[4]     = {false};
	int         required_count = 0;

	if (strcmp(config->win_type, "transform_input") == 0) {
		required[0]    = "win.repeat_1";
		required[1]    = "win.multiplier_2";
		required[2]    = "win.addend_3";
		required[3]    = "win.reverse_4";
		present[0]     = config->win_arg1_set;
		present[1]     = config->win_arg2_set;
		present[2]     = config->win_arg3_set;
		present[3]     = config->win_flag_set;
		required_count = 4;
	} else if (strcmp(config->win_type, "sum_groups") == 0) {
		required[0]    = "win.group_size_1";
		required[1]    = "win.insert_between_2";
		required[2]    = "win.inserted_value_3";
		present[0]     = config->win_arg1_set;
		present[1]     = config->win_flag_set;
		present[2]     = config->win_arg2_set;
		required_count = 3;
	} else if (strcmp(config->win_type, "copy_until") == 0) {
		required[0]    = "win.filter_enabled_1";
		required[1]    = "win.filter_value_2";
		required[2]    = "win.stop_value_3";
		present[0]     = config->win_arg1_set;
		present[1]     = config->win_arg2_set;
		present[2]     = config->win_arg3_set;
		required_count = 3;
	} else if (strcmp(config->win_type, "count_until") == 0) {
		required[0]    = "win.count_value_1";
		required[1]    = "win.stop_value_2";
		present[0]     = config->win_arg1_set;
		present[1]     = config->win_arg2_set;
		required_count = 2;
	} else if (strcmp(config->win_type, "decrement_offset") == 0) {
		required[0]    = "win.starting_offset_1";
		present[0]     = config->win_arg1_set;
		required_count = 1;
	} else {
		fprintf(stderr, "levels.cfg: level %d has unknown win.type '%s'\n",
		        level_id, config->win_type);
		return false;
	}

	bool valid = true;
	for (int index = 0; index < required_count; index++) {
		if (!present[index]) {
			fprintf(stderr, "levels.cfg: level %d win.type '%s' requires %s\n",
			        level_id, config->win_type, required[index]);
			valid = false;
		}
	}
	return valid;
}

static void add_instructions(char *items)
{
	for (char *item = strtok(items, ","); item != NULL;
	     item       = strtok(NULL, ",")) {
		item = trim(item);
		if (strcmp(item, "MOV") == 0)
			iw_add_instruction_to_list(MOV);
		else if (strcmp(item, "ADD") == 0)
			iw_add_instruction_to_list(ADD);
		else if (strcmp(item, "LABEL") == 0)
			iw_add_instruction_to_list(LABEL);
		else if (strcmp(item, "JMP") == 0)
			iw_add_instruction_to_list(JMP);
		else if (strcmp(item, "CMP") == 0)
			iw_add_instruction_to_list(CMP);
		else if (strcmp(item, "JE") == 0)
			iw_add_instruction_to_list(JE);
		else if (strcmp(item, "JNE") == 0)
			iw_add_instruction_to_list(JNE);
		else
			fprintf(stderr, "levels.cfg: unknown instruction '%s'\n", item);
	}
}

static void add_registers(char *items)
{
	for (char *item = strtok(items, ","); item != NULL;
	     item       = strtok(NULL, ",")) {
		item = trim(item);
		if (strcmp(item, "rax") == 0)
			rg_add_register_to_list(RAX);
		else if (strcmp(item, "rbx") == 0)
			rg_add_register_to_list(RBX);
		else if (strcmp(item, "rcx") == 0)
			rg_add_register_to_list(RCX);
		else if (strcmp(item, "rdx") == 0)
			rg_add_register_to_list(RDX);
		else if (strcmp(item, "rdi") == 0)
			rg_add_register_to_list(RDI);
		else
			fprintf(stderr, "levels.cfg: unknown register '%s'\n", item);
	}
}

static void apply_level(int level_id, level_config_t *config)
{
	input_properties_t input = {0};
	input.size               = config->input_count;
	input.type =
	    strcmp(config->input_type, "natural") == 0 ? NATURAL : NOT_ASSIGNED;
	input.mod = NONE;
	if (strcmp(config->input_modifier, "force") == 0)
		input.mod = FORCE;
	else if (strcmp(config->input_modifier, "increase") == 0)
		input.mod = INCREASE;
	input.mod_num1 = config->input_arg1;
	input.mod_num2 = config->input_arg2;
	input.mod_num3 = config->input_arg3;

	ir_rules_t rules = {.instruction_limit = config->instruction_limit,

	                    .code_editable           = true,
	                    .code_editable_exception = IR_NO_EXCEPTION,

	                    .buffer_selectable   = true,
	                    .register_selectable = true,
	                    .arrange_enabled     = true,
	                    .delete_enabled      = true};

	ir_set_base_rules(&rules);

	lp_configure(config->register_hints_enabled, config->buffer_hints_enabled,
	             config->immediate_hints_enabled, config->flag_boxes_visible);

	char title[32];
	char win_condition[64];
	snprintf(title, sizeof(title), "ADDRESS %02X", level_id);
	if (strcmp(config->win_type, "transform_input") == 0) {
		snprintf(win_condition, sizeof(win_condition), "WIN1 %d %d %d %s",
		         config->win_arg1, config->win_arg2, config->win_arg3,
		         config->win_flag ? "true" : "false");
	} else if (strcmp(config->win_type, "sum_groups") == 0) {
		snprintf(win_condition, sizeof(win_condition), "WIN2 %d %s %d",
		         config->win_arg1, config->win_flag ? "true" : "false",
		         config->win_arg2);
	} else if (strcmp(config->win_type, "copy_until") == 0) {
		snprintf(win_condition, sizeof(win_condition), "WIN3 %d %d %d",
		         config->win_arg1, config->win_arg2, config->win_arg3);
	} else if (strcmp(config->win_type, "count_until") == 0) {
		snprintf(win_condition, sizeof(win_condition), "WIN4 %d %d",
		         config->win_arg1, config->win_arg2);
	} else {
		snprintf(win_condition, sizeof(win_condition), "WIN5 %d",
		         config->win_arg1);
	}

	cw_set_stage_name(title);
	cw_set_challenge_text(config->challenge);
	iw_create_instruction_list();
	add_instructions(config->instructions);
	create_register_list();
	add_registers(config->registers);
	bf_set_input_properties(input);
	bf_generate_input_list();
	sb_set_step_btns_avail(config->step_controls_enabled);
	im_set_imm_up_avail(config->immediates_visible);
	wc_set_condition_from_text(win_condition);
	rg_update_register_box_position();
	iw_update_ins_box_size();
}

int lc_load_level(int level_id)
{
	char relative_path[64];
	char path[512];
	snprintf(relative_path, sizeof(relative_path), LEVEL_CONFIG_PATH_FORMAT,
	         level_id);
	ax_get_resource_path(path, sizeof(path), relative_path);
	FILE *file = fopen(path, "r");
	if (file == NULL)
		return FAIL;

	level_config_t config = {0};
	char           line[LINE_SIZE];
	bool           selected = false;
	bool           found    = false;
	while (fgets(line, sizeof(line), file) != NULL) {
		char *text = trim(line);
		if (*text == '\0' || *text == '#')
			continue;
		if (*text == '[') {
			int id = -1;
			if (sscanf(text, "[level %d]", &id) == 1) {
				selected = id == level_id;
				if (selected)
					found = true;
			} else
				selected = false;
			continue;
		}
		if (!selected)
			continue;
		char *equals = strchr(text, '=');
		if (equals == NULL)
			continue;
		*equals     = '\0';
		char *key   = trim(text);
		char *value = trim(equals + 1);
		if (strcmp(key, "challenge") == 0)
			snprintf(config.challenge, sizeof(config.challenge), "%s", value);
		else if (strcmp(key, "instructions") == 0)
			snprintf(config.instructions, sizeof(config.instructions), "%s",
			         value);
		else if (strcmp(key, "registers") == 0)
			snprintf(config.registers, sizeof(config.registers), "%s", value);
		else if (strcmp(key, "input.type") == 0)
			snprintf(config.input_type, sizeof(config.input_type), "%s", value);
		else if (strcmp(key, "input.modifier") == 0)
			snprintf(config.input_modifier, sizeof(config.input_modifier), "%s",
			         value);
		else if (strcmp(key, "input.count") == 0)
			config.input_count = atoi(value);
		else if (strcmp(key, "input.arg1") == 0)
			config.input_arg1 = atoi(value);
		else if (strcmp(key, "input.arg2") == 0)
			config.input_arg2 = atoi(value);
		else if (strcmp(key, "input.arg3") == 0)
			config.input_arg3 = atoi(value);
		else if (strcmp(key, "instruction_limit") == 0)
			config.instruction_limit = atoi(value);
		else if (strcmp(key, "ui.step_controls_enabled") == 0) {
			config.step_controls_enabled     = parse_bool(value);
			config.step_controls_enabled_set = true;
		} else if (strcmp(key, "ui.immediates_visible") == 0) {
			config.immediates_visible     = parse_bool(value);
			config.immediates_visible_set = true;
		} else if (strcmp(key, "ui.register_hints_enabled") == 0) {
			config.register_hints_enabled = parse_bool(value);

			config.register_hints_enabled_set = true;
		} else if (strcmp(key, "ui.buffer_hints_enabled") == 0) {
			config.buffer_hints_enabled = parse_bool(value);

			config.buffer_hints_enabled_set = true;
		} else if (strcmp(key, "ui.immediate_hints_enabled") == 0) {
			config.immediate_hints_enabled = parse_bool(value);

			config.immediate_hints_enabled_set = true;
		} else if (strcmp(key, "ui.flag_boxes_visible") == 0) {
			config.flag_boxes_visible = parse_bool(value);

			config.flag_boxes_visible_set = true;
		} else if (strcmp(key, "win.type") == 0)
			snprintf(config.win_type, sizeof(config.win_type), "%s", value);
		else if (strcmp(key, "win.repeat_1") == 0 ||
		         strcmp(key, "win.group_size_1") == 0 ||
		         strcmp(key, "win.filter_enabled_1") == 0 ||
		         strcmp(key, "win.count_value_1") == 0 ||
		         strcmp(key, "win.starting_offset_1") == 0) {
			config.win_arg1     = atoi(value);
			config.win_arg1_set = true;
		} else if (strcmp(key, "win.multiplier_2") == 0 ||
		           strcmp(key, "win.inserted_value_3") == 0 ||
		           strcmp(key, "win.filter_value_2") == 0 ||
		           strcmp(key, "win.stop_value_2") == 0) {
			config.win_arg2     = atoi(value);
			config.win_arg2_set = true;
		} else if (strcmp(key, "win.addend_3") == 0 ||
		           strcmp(key, "win.stop_value_3") == 0) {
			config.win_arg3     = atoi(value);
			config.win_arg3_set = true;
		} else if (strcmp(key, "win.reverse_4") == 0 ||
		           strcmp(key, "win.insert_between_2") == 0) {
			config.win_flag     = parse_bool(value);
			config.win_flag_set = true;
		}
	}
	fclose(file);
	if (!found || config.input_count <= 0 || config.instructions[0] == '\0' ||
	    config.registers[0] == '\0' || config.win_type[0] == '\0')
		return FAIL;
	if (!validate_ui_properties(level_id, &config)) {
		return FAIL;
	}
	if (!validate_win_condition(level_id, &config))
		return FAIL;
	apply_level(level_id, &config);
	return SUCCESS;
}
