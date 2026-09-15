#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "aux.h"
#include "code_window_cw.h"
#include "file_fl.h"
#include "levels_lv.h"
#include "storage/save_sv.h"

#define SAVE_DIRECTORY "data/saves"
#define SAVE_FORMAT_HEADER "# Assembly Architect save format v1\n"
#define CODE_BEGIN "code_begin"
#define CODE_END "code_end"

static bool get_player_save_path(int player_id, char *path, size_t path_size)
{
	char relative_path[64];
	int written = snprintf(relative_path, sizeof(relative_path),
			"%s/player_%02d.sav", SAVE_DIRECTORY, player_id);
	if (written < 0 || (size_t)written >= sizeof(relative_path)) return false;
	ax_get_resource_path(path, path_size, relative_path);
	return true;
}

static bool ensure_save_directory(void)
{
	char path[512];
	ax_get_resource_path(path, sizeof(path), SAVE_DIRECTORY);
	if (mkdir(path, 0755) == 0 || errno == EEXIST) return true;
	perror("Could not create save directory");
	return false;
}

static bool write_default_save(int player_id)
{
	char path[512];
	if (!get_player_save_path(player_id, path, sizeof(path))) return false;
	FILE *file = fopen(path, "w");
	if (file == NULL) {
		perror("Could not create player save");
		return false;
	}

	fprintf(file, "%s[player]\nid = %d\n\n", SAVE_FORMAT_HEADER, player_id);
	for (int level_id = 0; level_id < LV_LEVEL_QUANTITY; level_id++) {
		fprintf(file, "[level %02d]\nunlocked = %s\n%s\n", level_id,
			level_id == 0 ? "true" : "false", CODE_BEGIN);
		if (level_id == 1) {
			fputs(FL_L1_CODE_1, file);
			fputs(FL_L1_CODE_2, file);
			fputs(FL_L1_CODE_3, file);
		} else if (level_id == 8) {
			fputs(FL_L8_CODE_1, file);
			fputs(FL_L8_CODE_2, file);
		}
		fprintf(file, "%s\n\n", CODE_END);
	}
	if (fclose(file) != 0) {
		perror("Could not finish player save");
		return false;
	}
	return true;
}

static bool replace_save_file(const char *temporary_path, const char *save_path)
{
	if (rename(temporary_path, save_path) == 0) return true;
	perror("Could not replace player save");
	remove(temporary_path);
	return false;
}

void sv_save_init_default(void)
{
	if (!ensure_save_directory()) return;
	for (int player_id = SV_ARCHITECT_EXECUTOR_X;
		 player_id <= SV_ARCHITECT_HANDLER_Z; player_id++) {
		char path[512];
		if (!get_player_save_path(player_id, path, sizeof(path))) continue;
		if (access(path, F_OK) != 0) write_default_save(player_id);
	}
}

void sv_load_architect(int player_id, bool *levels_array)
{
	assert(player_id >= SV_ARCHITECT_EXECUTOR_X &&
		player_id <= SV_ARCHITECT_HANDLER_Z);
	assert(levels_array != NULL);
	for (int level_id = 0; level_id < LV_LEVEL_QUANTITY; level_id++) {
		levels_array[level_id] = false;
	}

	char path[512];
	if (!get_player_save_path(player_id, path, sizeof(path))) return;
	FILE *file = fopen(path, "r");
	if (file == NULL) return;

	char line[512];
	int level_id = -1;
	while (fgets(line, sizeof(line), file) != NULL) {
		if (sscanf(line, "[level %d]", &level_id) == 1) continue;
		if (level_id >= 0 && level_id < LV_LEVEL_QUANTITY &&
			strncmp(line, "unlocked = ", 11) == 0) {
			levels_array[level_id] = strcmp(line + 11, "true\n") == 0;
			level_id = -1;
		}
	}
	fclose(file);
}

void sv_load_level_code(int player_id, int level_id)
{
	char path[512];
	if (!get_player_save_path(player_id, path, sizeof(path))) return;
	FILE *file = fopen(path, "r");
	if (file == NULL) return;

	char line[512];
	bool selected = false;
	bool reading_code = false;
	while (fgets(line, sizeof(line), file) != NULL) {
		int found_level = -1;
		if (sscanf(line, "[level %d]", &found_level) == 1) {
			selected = found_level == level_id;
			reading_code = false;
			continue;
		}
		if (!selected) continue;
		if (strncmp(line, CODE_BEGIN, strlen(CODE_BEGIN)) == 0) {
			reading_code = true;
			continue;
		}
		if (strncmp(line, CODE_END, strlen(CODE_END)) == 0) break;
		if (reading_code && line[0] != '\n') {
			line[strcspn(line, "\n")] = '\0';
			cw_add_saved_line(line);
		}
	}
	fclose(file);
	cw_update_saved_jump_instructions();
}

void sv_save_level_code(int player_id, int level_id)
{
	char path[512];
	char temporary_path[544];
	if (!get_player_save_path(player_id, path, sizeof(path))) return;
	snprintf(temporary_path, sizeof(temporary_path), "%s.tmp", path);
	FILE *source = fopen(path, "r");
	FILE *temporary = fopen(temporary_path, "w");
	if (source == NULL || temporary == NULL) {
		perror("Could not save player level");
		if (source != NULL) fclose(source);
		if (temporary != NULL) fclose(temporary);
		return;
	}

	char line[512];
	bool selected = false;
	while (fgets(line, sizeof(line), source) != NULL) {
		int found_level = -1;
		if (sscanf(line, "[level %d]", &found_level) == 1) {
			selected = found_level == level_id;
			fputs(line, temporary);
			continue;
		}
		if (!selected) {
			fputs(line, temporary);
			continue;
		}
		if (strncmp(line, "unlocked = ", 11) == 0) {
			fputs("unlocked = true\n", temporary);
			continue;
		}
		if (strncmp(line, CODE_BEGIN, strlen(CODE_BEGIN)) == 0) {
			fputs(CODE_BEGIN "\n", temporary);
			write_player_code_to_file(temporary);
			while (fgets(line, sizeof(line), source) != NULL &&
				strncmp(line, CODE_END, strlen(CODE_END)) != 0) {}
			fputs(CODE_END "\n", temporary);
			selected = false;
			continue;
		}
		fputs(line, temporary);
	}
	fclose(source);
	if (fclose(temporary) != 0) {
		perror("Could not finish player save");
		remove(temporary_path);
		return;
	}
	replace_save_file(temporary_path, path);
}

void sv_unlock_level(int player_id, int level_id)
{
	char path[512];
	char temporary_path[544];
	if (!get_player_save_path(player_id, path, sizeof(path))) return;
	snprintf(temporary_path, sizeof(temporary_path), "%s.tmp", path);
	FILE *source = fopen(path, "r");
	FILE *temporary = fopen(temporary_path, "w");
	if (source == NULL || temporary == NULL) {
		perror("Could not unlock player level");
		if (source != NULL) fclose(source);
		if (temporary != NULL) fclose(temporary);
		return;
	}

	char line[512];
	bool selected = false;
	while (fgets(line, sizeof(line), source) != NULL) {
		int found_level = -1;
		if (sscanf(line, "[level %d]", &found_level) == 1) selected = found_level == level_id;
		if (selected && strncmp(line, "unlocked = ", 11) == 0) {
			fputs("unlocked = true\n", temporary);
			selected = false;
		} else {
			fputs(line, temporary);
		}
	}
	fclose(source);
	if (fclose(temporary) != 0) {
		perror("Could not finish player save");
		remove(temporary_path);
		return;
	}
	replace_save_file(temporary_path, path);
}
