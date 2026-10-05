#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <SDL.h>

#include "aux.h"
#include "gameplay/level_presentation_lp.h"
#include "level_config.h"
#include "levels_lv.h"
#include "text_tx.h"

typedef struct level_config_fixture_t {
	char path[512];
	char directory[512];
	bool directory_created;
	bool file_created;
} level_config_fixture_t;

static level_config_fixture_t fixture;

static void remove_fixture(void)
{
	if (fixture.file_created)
		unlink(fixture.path);
	if (fixture.directory_created)
		rmdir(fixture.directory);
}

static void get_level_config_path(int level_id, char *path, size_t path_size)
{
	char relative_path[64];
	snprintf(relative_path, sizeof(relative_path), "data/levels/%02d/level.cfg",
	         level_id);
	ax_get_resource_path(path, path_size, relative_path);
}

static int find_missing_level_config(void)
{
	char path[512];
	for (int level_id = LV_LEVEL_QUANTITY - 1; level_id >= 17; level_id--) {
		get_level_config_path(level_id, path, sizeof(path));
		if (access(path, F_OK) != 0 && errno == ENOENT)
			return level_id;
	}
	return -1;
}

static bool create_fixture(int level_id)
{
	char relative_directory[64];
	snprintf(relative_directory, sizeof(relative_directory), "data/levels/%02d",
	         level_id);
	ax_get_resource_path(fixture.directory, sizeof(fixture.directory),
	                     relative_directory);
	get_level_config_path(level_id, fixture.path, sizeof(fixture.path));

	if (mkdir(fixture.directory, 0700) == 0) {
		fixture.directory_created = true;
	} else if (errno != EEXIST) {
		return false;
	}

	int descriptor = open(fixture.path, O_WRONLY | O_CREAT | O_EXCL, 0600);
	if (descriptor < 0)
		return false;
	FILE *file = fdopen(descriptor, "w");
	if (file == NULL) {
		close(descriptor);
		unlink(fixture.path);
		return false;
	}
	fixture.file_created = true;
	return fclose(file) == 0;
}

static bool write_fixture_config(int level_id, const char *property)
{
	char content[256];
	int  length = snprintf(content, sizeof(content), "[level %d]\n%s\n",
	                       level_id, property);
	if (length < 0 || (size_t)length >= sizeof(content))
		return false;

	FILE *file = fopen(fixture.path, "w");
	if (file == NULL)
		return false;
	bool written = fputs(content, file) >= 0;
	return fclose(file) == 0 && written;
}

int main(void)
{
	assert(SDL_Init(0) == 0);
	assert(atexit(remove_fixture) == 0);

	char message[512];
	lp_configure(false, false, false, false, true);
	assert(lp_are_operand_highlights_enabled());
	assert(lc_load_hover_message(0, message, sizeof(message)) == SUCCESS);
	assert(strcmp(message, "ARCHITECTURE INTRODUCTION") == 0);
	assert(lp_are_operand_highlights_enabled());

	assert(lc_load_hover_message(2, message, sizeof(message)) == SUCCESS);
	assert(strcmp(message,
	              "DATA COMSUMPTION, REGISTER PERSISTANCE, AND INSTRUCTION DUPLICATION") == 0);
	assert(lp_are_operand_highlights_enabled());

	char small_buffer[8];
	assert(lc_load_hover_message(0, small_buffer, sizeof(small_buffer)) == SUCCESS);
	assert(strcmp(small_buffer, "ARCHITE") == 0);
	assert(small_buffer[sizeof(small_buffer) - 1] == '\0');

	message[0] = 'x';
	int missing_level = find_missing_level_config();
	assert(missing_level >= 0);
	assert(lc_load_hover_message(missing_level, message, sizeof(message)) == FAIL);
	assert(message[0] == '\0');

	assert(lc_load_hover_message(-1, message, sizeof(message)) == FAIL);
	assert(lc_load_hover_message(LV_LEVEL_QUANTITY, message, sizeof(message)) ==
	       FAIL);
	assert(lc_load_hover_message(0, NULL, sizeof(message)) == FAIL);
	assert(lc_load_hover_message(0, message, 0) == FAIL);

	bool fixture_created = create_fixture(missing_level);
	assert(fixture_created);
	bool config_written =
	    write_fixture_config(missing_level, "challenge = test metadata only");
	assert(config_written);
	assert(lc_load_hover_message(missing_level, message, sizeof(message)) == FAIL);
	assert(message[0] == '\0');

	config_written =
	    write_fixture_config(missing_level, "hover_message =   TRIMMED MESSAGE   ");
	assert(config_written);
	assert(lc_load_hover_message(missing_level, message, sizeof(message)) ==
	       SUCCESS);
	assert(strcmp(message, "TRIMMED MESSAGE") == 0);

	tx_free_level_text_textures();
	assert(tx_get_message_texture(-1) == NULL);
	assert(tx_get_message_texture(0) == NULL);
	tx_set_and_allocate_msgs_array(LV_LEVEL_QUANTITY);
	assert(tx_get_message_texture(-1) == NULL);
	assert(tx_get_message_texture(LV_LEVEL_QUANTITY) == NULL);
	tx_set_and_allocate_msgs_array(LV_LEVEL_QUANTITY);
	tx_free_level_text_textures();
	tx_free_level_text_textures();
	lp_configure(false, false, false, false, false);

	SDL_Quit();
	return 0;
}