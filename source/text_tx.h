#ifndef TEXT_TX_H
#define TEXT_TX_H
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "draw_dw.h"

enum text_box_positions {
	TX_BOX_MIN,
	TX_INS_BOX,
	TX_UPPER_BOX,
	TX_UPPER_RIGHT_BOX,
	TX_CENTER_BOX,
	TX_CENTER_RIGHT_BOX,
	TX_LOWER_BOX,
	TX_CODE_BOX,
	TX_STAGEBUTTON_BOX,
	TX_CENTER_UP_BOX,
	TX_ERROR_BOX,
	TX_BOX_MAX
};

typedef struct tx_text_box_options_t {
	enum text_box_positions position;
	bool large_box;
	bool large_text;
} tx_text_box_options_t;

typedef struct tx_text_layout_info_t {
	int required_rows;
	int visible_rows;
	int overflow_rows;
	bool overflowed;
} tx_text_layout_info_t;

typedef enum tx_text_style_t {
	TX_TEXT_STYLE_BODY,
	TX_TEXT_STYLE_SYNTAX,
	TX_TEXT_STYLE_DIRECTIVE,
	TX_TEXT_STYLE_WARNING
} tx_text_style_t;

typedef struct tx_text_fragment_t {
	texture_t *texture;
	int x_offset;
} tx_text_fragment_t;

typedef struct tx_styled_row_t {
	tx_text_fragment_t *fragments;
	int fragment_count;
	int rendered_width;
} tx_styled_row_t;

/* Owns its rows and every fragment texture stored in them. */
typedef struct tx_styled_text_t {
	tx_styled_row_t *rows;
	int row_count;
} tx_styled_text_t;

/* Output pointers may be NULL, but at least one output must be requested. */
bool tx_get_text_box_rects(const tx_text_box_options_t *options,
						   SDL_Rect *box, SDL_Rect *content,
						   int *text_height);

void tx_init_global_msgs();
void tx_text_box(int pos, int msg_id, int header);
void tx_bottom_msg(const tx_text_box_options_t *options, int msg_id);
void tx_free_level_text_textures();
void tx_set_and_allocate_msgs_array(int size);
void tx_set_message_in_array(int pos, char *msg, int w, int h);
void tx_set_single_line_message(int position, const char *message);
bool tx_draw_create_typewriter_text(texture_t **t, SDL_Rect r, const char *text,
                                    size_t *index, SDL_Color color);
texture_array_t *tx_get_message_texture(int pos);
SDL_Rect         tx_get_text_box_wh();
texture_array_t *tx_create_text_box_message(
	const tx_text_box_options_t *options, const char *message);

/* Tutorial authoring prefixes style a complete logical source line and are
 * stripped before rendering: @syntax marks assembly forms and operands,
 * @directive marks the required operation or execution action, and @warning
 * marks important constraints. Unprefixed lines use body style; unknown
 * prefixes remain ordinary visible text. */
tx_styled_text_t *tx_create_styled_text_box_message(
	const tx_text_box_options_t *options, const char *message,
	tx_text_layout_info_t *layout_info);

void tx_draw_styled_text_box_message(const tx_text_box_options_t *options,
									const tx_styled_text_t *message, int header);
void tx_free_styled_text(tx_styled_text_t *message);

void tx_text_box_texture(const tx_text_box_options_t *options,
			 texture_array_t *message, int header);

enum gbl_msgs { TX_MSG_CLICKANY, TX_MSG_PRESSPLAY, TX_MSG_PRESSBACK };

enum header { TX_NONE, TX_SYSMES, TX_SYSNOT, TX_SYSWAR, TX_INS };

enum msgs {
	MSG0,
	MSG1,
	MSG2,
	MSG3,
	MSG4,
	MSG5,
	MSG6,
	MSG7,
	MSG8,
	MSG9,
	MSG10,
	MSG11,
	MSG12,
	MSG13,
	MSG14,
	MSG15,
	MSG16,
	MSG17,
	MSG18,
	MSG19,
	MSG20,
};

#endif
