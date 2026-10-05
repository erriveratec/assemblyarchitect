#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <limits.h>
#include <ctype.h>
#include <string.h>
#include <SDL_mixer.h>
#include "text_tx.h"
#include "draw_dw.h"
#include "sdl_config.h"
#include "code_window_cw.h"
#include "dimensions_dm.h"
#include "media/audio_au.h"
#include "aux.h"

#define MSG_CLICKANY "Click Anywhere"
#define MSG_PRESSPLAY "Press the Play Button"
#define MSG_PRESSBACK "Press the Back Button"
#define MSG_PRESSCONT "Press the Continue Button"

static const Uint32 TEXT_H_BOTTOM_MSG = 17;
static const Uint32 TEXT_BOX_H        = 360; // 1920/5
static const Uint32 TEXT_BOX_W        = 384;
static const Uint32 LARGE_TEXT_BOX_H  = 440;
static const Uint32 LARGE_TEXT_BOX_W  = 600;

static const Uint32 BORDER_OFS = 10;

char *SYSTEM_MESSAGE = "SYSTEM MESSAGE";
char *SYSTEM_NOTICE  = "SYSTEM NOTICE";
char *SYSTEM_WARNING = "SYSTEM WARNING";
char *INSTRUCTION    = "INSTRUCTION";

texture_t *g_system_message = NULL;
texture_t *g_system_notice  = NULL;
texture_t *g_system_warning = NULL;
texture_t *g_instruction    = NULL;

int               g_lvl_msgs_size;
texture_array_t **g_lvl_msgs = NULL;

int               g_msgs_size;
texture_array_t **g_msgs = NULL;

int               g_gbl_msgs_size;
texture_array_t **g_gbl_msgs = NULL;

static int      get_box_member(SDL_Rect *box, int member);
static int      get_h_bottom_msg();
static SDL_Rect get_text_box_center_up();
static int      get_border_ofs();
static SDL_Rect get_text_box_upper();
static SDL_Rect get_text_box_lower();
static SDL_Rect get_text_box_center();
static SDL_Rect get_text_box_ins();
static SDL_Rect get_text_box_code();
static SDL_Rect get_text_box_upper_right();
static SDL_Rect get_text_box_center_right();

/* Function: get_border_ofs
 * -----------------------------------------------------------------------------
 * Returns a border space of messages
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	The offset value
 */
static int get_border_ofs() { return dm_scale_to_res(BORDER_OFS); }

/* Function: tx_get_text_box_wh
 * -----------------------------------------------------------------------------
 * Return the width and heigth value of the text box
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the dimension of the object
 */
SDL_Rect tx_get_text_box_wh()
{
	SDL_Rect b;
	b.w = dm_scale_to_res(TEXT_BOX_W);
	b.h = dm_scale_to_res(TEXT_BOX_H);
	b.x = 0;
	b.y = 0;
	return b;
}

/* Function: get_text_box_upper_right
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
static SDL_Rect get_text_box_upper_right()
{
	SDL_Rect d = tx_get_text_box_wh();
	SDL_Rect b;
	b.w = d.w;
	b.h = d.h;
	b.x = dm_get_stage_imm_up().x + 12 * ax_get_value_box_size().w;
	b.y = dw_get_ofs_iface_filled_border();
	return b;

	return b;
}

/* Function: get_text_box_code
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
static SDL_Rect get_text_box_code()
{
	SDL_Rect cb = cw_get_stage_code_box();
	SDL_Rect d  = tx_get_text_box_wh();
	SDL_Rect b;
	b.w = d.w;
	b.h = d.h;
	b.x = cb.x + (cb.w - b.w) / 2;
	b.y = cb.y + cb.h - b.h;
	return b;
}
/* Function: get_text_box_ins
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
static SDL_Rect get_text_box_ins()
{

	SDL_Rect ib = dm_get_stage_instruction_box();
	SDL_Rect d  = tx_get_text_box_wh();
	SDL_Rect b;
	b.w = d.w;
	b.h = d.h;
	b.x = ib.x;
	b.y = ib.y + ib.h + dm_get_w_borders();
	return b;
}

/* Function: get_text_box_center_right
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
static SDL_Rect get_text_box_center_right()
{
	int      w = dm_get_screen_width();
	int      h = dm_get_screen_height();
	SDL_Rect d = tx_get_text_box_wh();

	SDL_Rect b;
	b.w = d.w;
	b.h = d.h;
	b.x = w / 2;
	b.y = h / 2 - b.h / 2;
	return b;
}

/* Function: get_text_box_center
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
static SDL_Rect get_text_box_center()
{
	int      w = dm_get_screen_width();
	int      h = dm_get_screen_height();
	SDL_Rect d = tx_get_text_box_wh();

	SDL_Rect b;
	b.w = d.w;
	b.h = d.h;
	b.x = w / 2 - b.w / 2;
	b.y = h / 2 - b.h / 2;
	return b;
}

/* Function: get_text_box_lower
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the text box lower
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
static SDL_Rect get_text_box_lower()
{
	int      w = dm_get_screen_width();
	int      h = dm_get_screen_height();
	SDL_Rect d = tx_get_text_box_wh();
	SDL_Rect b;
	b.w = d.w;
	b.h = d.h;
	b.x = w / 2 - b.w / 2;
	b.y = h - b.h - get_border_ofs();
	return b;
}
/* Function: get_text_box_upper
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
static SDL_Rect get_text_box_upper()
{
	int      w = dm_get_screen_width();
	SDL_Rect d = tx_get_text_box_wh();
	SDL_Rect b;
	b.w = d.w;
	b.h = d.h;
	b.x = w / 2 - b.w / 2;
	b.y = get_border_ofs();
	return b;
}

/* Function: get_text_box_center_up
 * -----------------------------------------------------------------------------
 * Returns the box dimensions for the object
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	SDL_Rect with the positions of the object
 */
static SDL_Rect get_text_box_center_up()
{
	int      w = dm_get_screen_width();
	int      h = dm_get_screen_height();
	SDL_Rect d = tx_get_text_box_wh();
	SDL_Rect b;
	b.w = d.w;
	b.h = d.h;
	b.x = w / 2 - b.w / 2;
	b.y = h / 4;
	return b;
}

/* Function: get_h_bottom_msg
 * -----------------------------------------------------------------------------
 * Returns the h value for the click anywhere message
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	int with the offset for the sel level buttons
 */
static int get_h_bottom_msg()
{
	int h = dm_scale_to_res(TEXT_H_BOTTOM_MSG);
	return h;
}

/* Function: tx_get_message_texture
 * -----------------------------------------------------------------------------
 * Retunr a given message texture loaded from a file
 *
 * Arguments:
 *	pos: position of the texture that wants to be retrieved
 *
 * Return:
 * 	texture_array_t pointer of the requested texture
 */
texture_array_t *tx_get_message_texture(int pos)
{
	if (g_msgs == NULL || pos < 0 || pos >= g_msgs_size)
		return NULL;
	return g_msgs[pos];
}

/* Function: tx_draw_create_typewriter_text
 * -----------------------------------------------------------------------------
 * This function is used to create the typewriter effect wheen neeeded
 *
 * Arguments:
 *	t: pointer to the texture t of the texture
 * 	text: text that will be progresively created
 *
 * Return:
 * 	bool indicating if the writing is complete
 */
bool tx_draw_create_typewriter_text(texture_t **t, SDL_Rect r, const char *text,
                                    size_t *index, SDL_Color color)
{
	bool   complete    = false;
	size_t full_length = strlen(text);
	if (*index < full_length) {
		(*index)++;
		char   buf[256];
		size_t n = (*index < sizeof(buf) - 1 ? *index : sizeof(buf) - 1);
		memcpy(buf, text, n);
		buf[n] = '\0';

		dw_free_texture(*t);

		*t = dw_create_text_tex(buf, color);
		dw_draw_texture_fit_h(r, *t);

		if (g_sfx_type && buf[n - 1] != ' ') {
			Mix_PlayChannel(-1, g_sfx_type, 0);
		}
	} else if (*index == full_length) {
		complete = true;
	}
	return complete;
}

/* Function: tx_set_and_allocate_msgs_array
 * -----------------------------------------------------------------------------
 * This function reserves the sapce required of the array of messages that
 * will be used on a level
 *
 * Arguments:
 *	size: size of the array textures
 *
 * Return:
 * 	Void.
 */
void tx_set_and_allocate_msgs_array(int size)
{
	tx_free_level_text_textures();
	if (size <= 0)
		return;

	g_msgs = calloc((size_t)size, sizeof(texture_array_t *));
	if (g_msgs == NULL) {
		fprintf(stderr, "tx_set_and_allocate_msgs_array: allocation failed\n");
		return;
	}
	g_msgs_size = size;
}

/* Function: tx_set_message_in_array
 * -----------------------------------------------------------------------------
 * Recives a message read from the file and stores it in the message array
 * on a give position
 *
 * Arguments:
 *	pos: position in the message array
 *  msg: message that will be set
 *	h: the height of each line of the message
 *	w: the width of the container of the message
 *
 * Return:
 * 	Void.
 */
void tx_set_message_in_array(int pos, char *msg, int w, int h)
{
	assert(pos >= 0 && "Invalid position");
	assert(msg != NULL && "NULL message");

	// pos--; // THIS WILL EXPLODE LATER

	// int h = (pos == 0) ? dm_get_h_big_text() : dm_get_h_msg();
	// int h = dm_get_h_msg();
	//	int w = dm_get_w_msg(dm_get_box_msg_wh());
	g_msgs[pos] = dw_create_text_tex_array_by_h(w, h, C_WHITE, msg);
}

void tx_set_single_line_message(int position, const char *message)
{
	if (g_msgs == NULL || position < 0 || position >= g_msgs_size) {
		fprintf(stderr,
		        "tx_set_single_line_message: "
		        "invalid position %d\n",
		        position);
		return;
	}

	if (message == NULL || message[0] == '\0') {
		fprintf(stderr,
		        "tx_set_single_line_message: "
		        "empty message at position %d\n",
		        position);
		return;
	}

	texture_array_t *array = calloc(1, sizeof(texture_array_t));

	if (array == NULL) {
		fprintf(stderr, "tx_set_single_line_message: "
		                "unable to allocate texture array\n");
		return;
	}

	array->size = 1;
	array->t    = calloc(1, sizeof(texture_t *));

	if (array->t == NULL) {
		free(array);
		fprintf(stderr, "tx_set_single_line_message: "
		                "unable to allocate texture entry\n");
		return;
	}

	array->t[0] = dw_create_text_tex((char *)message, C_WHITE);

	if (array->t[0] == NULL) {
		dw_free_texture_array(array);
		fprintf(stderr,
		        "tx_set_single_line_message: "
		        "unable to render position %d\n",
		        position);
		return;
	}

	if (g_msgs[position] != NULL) {
		dw_free_texture_array(g_msgs[position]);
	}

	g_msgs[position] = array;
}

/* Function: tx_init_global_msgs
 * -----------------------------------------------------------------------------
 * Creates the textures of global messages that are used across several levels.
 * This messages are used in the message boxes.
 *
 * Arguments:
 *	Void.
 *
 * Return:
 *	Void.
 */
void tx_init_global_msgs()
{
	g_gbl_msgs_size = 3;
	g_gbl_msgs      = malloc(sizeof(texture_array_t *) * g_gbl_msgs_size);

	int text_h = get_h_bottom_msg();

	int w = dw_get_iface_content_box(tx_get_text_box_wh()).w;
	g_gbl_msgs[TX_MSG_CLICKANY] =
	    dw_create_text_tex_array_by_h(w, text_h, C_WHITE, MSG_CLICKANY);
	g_gbl_msgs[TX_MSG_PRESSPLAY] =
	    dw_create_text_tex_array_by_h(w, text_h, C_SHADOWGREY, MSG_PRESSPLAY);
	g_gbl_msgs[TX_MSG_PRESSBACK] =
	    dw_create_text_tex_array_by_h(w, text_h, C_SHADOWGREY, MSG_PRESSBACK);

	text_h           = dw_get_h_iface_header_txt();
	g_system_message = dw_create_text_tex(SYSTEM_MESSAGE, C_GREY);
	g_system_notice  = dw_create_text_tex(SYSTEM_NOTICE, C_GREY);
	g_system_warning = dw_create_text_tex(SYSTEM_WARNING, C_GREY);
	g_instruction    = dw_create_text_tex(INSTRUCTION, C_GREY);
}

/* Function: tx_free_level_text_texture
 * -----------------------------------------------------------------------------
 * Free the level textures when a level is finished
 *
 * Arguments:
 *	Void.
 *
 * Return:
 * 	Void.
 */
void tx_free_level_text_textures()
{
	if (g_msgs != NULL) {
		for (int i = 0; i < g_msgs_size; i++) {
			if (g_msgs[i] != NULL)
				dw_free_texture_array(g_msgs[i]);
		}
	}
	free(g_msgs);
	g_msgs      = NULL;
	g_msgs_size = 0;
}

/* Function: get_box_member
 * -----------------------------------------------------------------------------
 * Returns the property member of a given text box by a pointer to the bos
 *
 * Arguments:
 *	box: pointer of the box that the member will be recovered
 *	member: the member of the box that will be recovered.
 *
 * Return:
 *	member of the box recovered.
 */
static int get_box_member(SDL_Rect *box, int member)
{
	assert(box != NULL && "Box pointer is NULL");
	assert(member > MEMBER_MIN && member < MEMBER_MAX && "Invalid member");
	int retval;
	switch (member) {
	case MEMBER_X:
		retval = box->x;
		break;
	case MEMBER_Y:
		retval = box->y;
		break;
	case MEMBER_W:
		retval = box->w;
		break;
	case MEMBER_H:
		retval = box->h;
		break;
	}
	return retval;
}

/* Function: tx_bottom_msg
 * -----------------------------------------------------------------------------
 * Display a message in the bottom of a give box
 *
 * Arguments:
 *	r: The rectangle box where the message will be shown
 *	msg_id: The messages that are displayed at the botttom are global
 *
 * Return:
 *	Void.
 */
void tx_bottom_msg(const tx_text_box_options_t *options, int msg_id)
{
	assert(msg_id >= 0 && msg_id < g_gbl_msgs_size && "Invalid msg_id");
	if (options == NULL) {
		return;
	}

	SDL_Rect         b;
	SDL_Rect         content;
	int              layout_text_height;
	int              text_h = get_h_bottom_msg();
	texture_array_t *a = g_gbl_msgs[msg_id];
	if (options->position == TX_ERROR_BOX) {
		b      = dm_get_text_box_error();
		b.y    = dm_get_text_box_error().y + dm_get_text_box_error().h * 4 / 6;
		b.h    = dm_get_text_box_error().h / 6;
	} else {
		if (!tx_get_text_box_rects(options, &b, &content,
	                           &layout_text_height)) {
			return;
		}
		if (options->position == TX_STAGEBUTTON_BOX) {
			text_h = dm_get_h_msg();
		}
		b.y += b.h / 2 - 2 * text_h;
	}
	dw_draw_wrapped_texture_by_h(b, text_h, a);
}

bool tx_get_text_box_rects(
	const tx_text_box_options_t *options, SDL_Rect *box, SDL_Rect *content,
	int *text_height)
{
	if (options == NULL ||
	    (box == NULL && content == NULL && text_height == NULL)) {
		return false;
	}
	enum text_box_positions position = options->position;
	SDL_Rect resolved_box;
	int resolved_text_height;

	switch (position) {
	case TX_INS_BOX:
		resolved_box = get_text_box_ins();
		resolved_text_height = dm_get_h_msg();
		break;

	case TX_UPPER_BOX:
		resolved_box = get_text_box_upper();
		resolved_text_height = dm_get_h_msg();
		break;

	case TX_UPPER_RIGHT_BOX:
		resolved_box = get_text_box_upper_right();
		resolved_text_height = dm_get_h_msg();
		break;

	case TX_CENTER_BOX:
		resolved_box = get_text_box_center();
		resolved_text_height = dm_get_h_msg();
		break;

	case TX_CENTER_RIGHT_BOX:
		resolved_box = get_text_box_center_right();
		resolved_text_height = dm_get_h_msg();
		break;

	case TX_LOWER_BOX:
		resolved_box = get_text_box_lower();
		resolved_text_height = dm_get_h_msg();
		break;

	case TX_CODE_BOX:
		resolved_box = get_text_box_code();
		resolved_text_height = dm_get_h_msg();
		break;

	case TX_STAGEBUTTON_BOX:
		resolved_box = dm_get_text_box_stagebutton();
		resolved_text_height = dm_get_h_msg();
		break;

	case TX_CENTER_UP_BOX:
		resolved_box = get_text_box_center_up();
		resolved_text_height = dm_get_h_msg();
		break;

	case TX_ERROR_BOX:
		resolved_box = dm_get_text_box_error();
		resolved_text_height = dm_get_h_big_text();
		break;

	default:
		return false;
	}

	if (options->large_box) {
		int width  = dm_scale_to_res(LARGE_TEXT_BOX_W);
		int height = dm_scale_to_res(LARGE_TEXT_BOX_H);
		int extra_width  = width - resolved_box.w;
		int extra_height = height - resolved_box.h;

		if (position == TX_CENTER_BOX || position == TX_UPPER_BOX ||
		    position == TX_LOWER_BOX || position == TX_CODE_BOX ||
		    position == TX_CENTER_UP_BOX) {
			resolved_box.x -= extra_width / 2;
		}
		if (position == TX_CENTER_BOX || position == TX_CENTER_RIGHT_BOX ||
		    position == TX_STAGEBUTTON_BOX) {
			resolved_box.y -= extra_height / 2;
		} else if (position == TX_LOWER_BOX || position == TX_CODE_BOX) {
			resolved_box.y -= extra_height;
		}
		resolved_box.w = width;
		resolved_box.h = height;
		if (resolved_box.x + resolved_box.w > dm_get_screen_width()) {
			resolved_box.x = dm_get_screen_width() - resolved_box.w;
		}
		if (resolved_box.y + resolved_box.h > dm_get_screen_height()) {
			resolved_box.y = dm_get_screen_height() - resolved_box.h;
		}
		if (resolved_box.x < 0) {
			resolved_box.x = 0;
		}
		if (resolved_box.y < 0) {
			resolved_box.y = 0;
		}
	}
	if (options->large_text) {
		resolved_text_height = dm_get_h_big_text();
	}

	SDL_Rect resolved_content = dw_get_iface_content_box(resolved_box);
	int64_t box_right = (int64_t)resolved_box.x + resolved_box.w;
	int64_t box_bottom = (int64_t)resolved_box.y + resolved_box.h;
	int64_t content_right = (int64_t)resolved_content.x + resolved_content.w;
	int64_t content_bottom = (int64_t)resolved_content.y + resolved_content.h;
	if (resolved_box.w <= 0 || resolved_box.h <= 0 ||
	    resolved_content.w <= 0 || resolved_content.h <= 0 ||
	    resolved_text_height <= 0 || resolved_content.x < resolved_box.x ||
	    resolved_content.y < resolved_box.y || content_right > box_right ||
	    content_bottom > box_bottom) {
		return false;
	}
	if (box != NULL) {
		*box = resolved_box;
	}
	if (content != NULL) {
		*content = resolved_content;
	}
	if (text_height != NULL) {
		*text_height = resolved_text_height;
	}
	return true;
}

static texture_t *tx_get_header_texture(int header)
{
	switch (header) {
	case TX_NONE:
		return NULL;

	case TX_SYSMES:
		return g_system_message;

	case TX_SYSNOT:
		return g_system_notice;

	case TX_SYSWAR:
		return g_system_warning;

	case TX_INS:
		return g_instruction;

	default:
		return NULL;
	}
}

void tx_text_box_texture(const tx_text_box_options_t *options,
			texture_array_t *message, int header)
{
	if (message == NULL) {
		fprintf(stderr, "tx_text_box_texture: message texture is NULL\n");
		return;
	}

	SDL_Rect box;
	SDL_Rect content;
	int      text_height = 0;

	if (!tx_get_text_box_rects(options, &box, &content, &text_height)) {
		fprintf(stderr, "tx_text_box_texture: invalid box options\n");
		return;
	}

	texture_t *header_texture = tx_get_header_texture(header);

	dw_draw_iface_box(box, header_texture);

	dw_draw_wrapped_texture_by_h(content, text_height, message);
}

texture_array_t *tx_create_text_box_message(
	const tx_text_box_options_t *options, const char *message)
{
	if (message == NULL || message[0] == '\0') {
		return NULL;
	}

	SDL_Rect box;
	SDL_Rect content;
	int      text_height = 0;

	if (!tx_get_text_box_rects(options, &box, &content, &text_height)) {
		return NULL;
	}

	return dw_create_text_tex_array_by_h(content.w, text_height, C_WHITE,
	                                     (char *)message);
}

typedef struct tx_style_prefix_t {
	const char *prefix;
	size_t length;
	tx_text_style_t style;
} tx_style_prefix_t;

typedef enum tx_token_class_t {
	TX_TOKEN_CLASS_TEXT,
	TX_TOKEN_CLASS_INSTRUCTION,
	TX_TOKEN_CLASS_REGISTER,
	TX_TOKEN_CLASS_BUFFER,
	TX_TOKEN_CLASS_FLAG,
	TX_TOKEN_CLASS_IMMEDIATE,
	TX_TOKEN_CLASS_LABEL,
	TX_TOKEN_CLASS_MEMORY,
	TX_TOKEN_CLASS_OPERATOR,
	TX_TOKEN_CLASS_ADDRESS
} tx_token_class_t;

typedef struct tx_styled_run_t {
	size_t start;
	size_t length;
	tx_text_style_t style;
	tx_token_class_t token_class;
} tx_styled_run_t;

typedef struct tx_arch_token_t {
	const char *spelling;
	tx_token_class_t token_class;
} tx_arch_token_t;

/* The game exposes architecture IDs and display lookups, but no safe lexical
 * classifier for scanning arbitrary tutorial prose. Keep its vocabulary here. */
static const tx_arch_token_t g_tutorial_arch_tokens[] = {
	{"MOV", TX_TOKEN_CLASS_INSTRUCTION},
	{"ADD", TX_TOKEN_CLASS_INSTRUCTION},
	{"line", TX_TOKEN_CLASS_INSTRUCTION},
	{"JMP", TX_TOKEN_CLASS_INSTRUCTION},
	{"CMP", TX_TOKEN_CLASS_INSTRUCTION},
	{"JE", TX_TOKEN_CLASS_INSTRUCTION},
	{"JNE", TX_TOKEN_CLASS_INSTRUCTION},
	{"rax", TX_TOKEN_CLASS_REGISTER},
	{"rbx", TX_TOKEN_CLASS_REGISTER},
	{"rcx", TX_TOKEN_CLASS_REGISTER},
	{"rdx", TX_TOKEN_CLASS_REGISTER},
	{"rdi", TX_TOKEN_CLASS_REGISTER},
	{"[ib]", TX_TOKEN_CLASS_BUFFER},
	{"[ob]", TX_TOKEN_CLASS_BUFFER},
	{"ZF", TX_TOKEN_CLASS_FLAG},
};

static const tx_style_prefix_t g_tutorial_style_prefixes[] = {
	{"@syntax ", sizeof("@syntax ") - 1, TX_TEXT_STYLE_SYNTAX},
	{"@directive ", sizeof("@directive ") - 1, TX_TEXT_STYLE_DIRECTIVE},
	{"@warning ", sizeof("@warning ") - 1, TX_TEXT_STYLE_WARNING},
};

static char *tx_trim_line(char *line)
{
	while (isspace((unsigned char)*line)) {
		line++;
	}

	char *end = line + strlen(line);
	while (end > line && isspace((unsigned char)end[-1])) {
		end--;
	}
	*end = '\0';
	return line;
}

static tx_text_style_t tx_parse_tutorial_line(char *line)
{
	/* Match only after optional indentation, then strip one recognized prefix. */
	char *content = tx_trim_line(line);

	for (size_t index = 0;
	     index < sizeof(g_tutorial_style_prefixes) /
	                 sizeof(g_tutorial_style_prefixes[0]);
	     index++) {
		const tx_style_prefix_t *prefix = &g_tutorial_style_prefixes[index];
		if (strncmp(content, prefix->prefix, prefix->length) == 0) {
			memmove(line, content + prefix->length,
			        strlen(content + prefix->length) + 1);
			tx_trim_line(line);
			return prefix->style;
		}
	}

	if (content != line) {
		memmove(line, content, strlen(content) + 1);
	}
	return TX_TEXT_STYLE_BODY;
}

static SDL_Color tx_get_line_style_color(tx_text_style_t style)
{
	/* Semantic roles reuse the shared palette rather than defining new colors. */
	switch (style) {
	case TX_TEXT_STYLE_SYNTAX:
		return C_TERMINAL_GREEN;
	case TX_TEXT_STYLE_DIRECTIVE:
		return C_WHITE;
	case TX_TEXT_STYLE_WARNING:
		return C_WHITE;
	case TX_TEXT_STYLE_BODY:
	default:
		return C_WHITE;
	}
}

static SDL_Color tx_get_token_color(tx_token_class_t token_class)
{
	switch (token_class) {
	case TX_TOKEN_CLASS_TEXT:
		return C_WHITE;
	default:
		return C_TERMINAL_GREEN;
	}
}

static bool tx_is_identifier_char(unsigned char character)
{
	return isalnum(character) != 0 || character == '_';
}

static bool tx_append_run(tx_styled_run_t *runs, size_t *run_count,
						  size_t start, size_t length,
						  tx_text_style_t style,
						  tx_token_class_t token_class)
{
	if (length == 0) {
		return true;
	}
	if (*run_count > 0) {
		tx_styled_run_t *previous = &runs[*run_count - 1];
		if (previous->start + previous->length == start &&
		    previous->style == style && previous->token_class == token_class) {
			previous->length += length;
			return true;
		}
	}
	runs[*run_count] = (tx_styled_run_t){start, length, style, token_class};
	(*run_count)++;
	return true;
}

static bool tx_classify_token(const char *line, size_t position,
						  size_t line_length, size_t *token_length,
						  tx_token_class_t *token_class)
{
	for (size_t index = 0;
	     index < sizeof(g_tutorial_arch_tokens) /
	                 sizeof(g_tutorial_arch_tokens[0]); index++) {
		const tx_arch_token_t *token = &g_tutorial_arch_tokens[index];
		size_t length = strlen(token->spelling);
		if (length > line_length - position ||
		    memcmp(line + position, token->spelling, length) != 0) {
			continue;
		}
		if (token->token_class != TX_TOKEN_CLASS_BUFFER &&
		    ((position > 0 &&
		      tx_is_identifier_char((unsigned char)line[position - 1])) ||
		     (position + length < line_length &&
		      tx_is_identifier_char((unsigned char)line[position + length])))) {
			continue;
		}
		*token_length = length;
		*token_class = token->token_class;
		return true;
	}
	return false;
}

static tx_styled_run_t *tx_tokenize_line(const char *line, size_t line_length,
									 tx_text_style_t line_style,
									 size_t *run_count)
{
	if (line_length == SIZE_MAX ||
	    line_length + 1 > SIZE_MAX / sizeof(tx_styled_run_t)) {
		return NULL;
	}
	tx_styled_run_t *runs = calloc(line_length + 1, sizeof(*runs));
	if (runs == NULL) {
		return NULL;
	}
	*run_count = 0;
	for (size_t position = 0; position < line_length;) {
		size_t token_length = 0;
		tx_token_class_t token_class = TX_TOKEN_CLASS_TEXT;
		if (tx_classify_token(line, position, line_length, &token_length,
		                      &token_class)) {
			tx_append_run(runs, run_count, position, token_length,
			              TX_TEXT_STYLE_SYNTAX, token_class);
			position += token_length;
		} else {
			size_t start = position++;
			while (position < line_length &&
			       !tx_classify_token(line, position, line_length,
			                          &token_length, &token_class)) {
				position++;
			}
			tx_append_run(runs, run_count, start, position - start,
			              line_style, TX_TOKEN_CLASS_TEXT);
		}
	}
	return runs;
}

static bool tx_measure_span(const char *line, size_t start, size_t end,
							int text_height, int *width)
{
	if (end < start || end - start == SIZE_MAX) {
		return false;
	}
	char *text = malloc(end - start + 1);
	if (text == NULL) {
		return false;
	}
	memcpy(text, line + start, end - start);
	text[end - start] = '\0';
	texture_t *texture = dw_create_text_tex(text, C_WHITE);
	free(text);
	if (texture == NULL || texture->h <= 0 || texture->w <= 0) {
		dw_free_texture(texture);
		return false;
	}
	int64_t measured = (int64_t)texture->w * text_height / texture->h;
	dw_free_texture(texture);
	if (measured > INT_MAX) {
		return false;
	}
	*width = (int)measured;
	return true;
}

static bool tx_row_append_fragment(tx_styled_row_t *row, const char *text,
								   size_t length, SDL_Color color,
								   int text_height)
{
	if (length == 0 || length == SIZE_MAX) {
		return length == 0;
	}
	char *fragment_text = malloc(length + 1);
	if (fragment_text == NULL) {
		return false;
	}
	memcpy(fragment_text, text, length);
	fragment_text[length] = '\0';
	bool only_space = true;
	for (size_t index = 0; index < length; index++) {
		if (!isspace((unsigned char)fragment_text[index])) {
			only_space = false;
			break;
		}
	}
	if (only_space) {
		texture_t *space_texture = dw_create_text_tex(fragment_text, color);
		int native_width = 0;
		int native_height = 0;
		bool measured = space_texture != NULL && space_texture->h > 0 &&
		                space_texture->w > 0;
		if (measured) {
			native_width = space_texture->w;
			native_height = space_texture->h;
		} else {
			measured = TTF_SizeText(g_font, fragment_text,
			                        &native_width, &native_height) == 0 &&
			           native_height > 0;
		}
		dw_free_texture(space_texture);
		free(fragment_text);
		if (!measured) {
			return false;
		}
		int64_t advance = (int64_t)native_width * text_height / native_height;
		if (advance < 0 || advance > INT_MAX - row->rendered_width) {
			return false;
		}
		row->rendered_width += (int)advance;
		return true;
	}
	texture_t *texture = dw_create_text_tex(fragment_text, color);
	free(fragment_text);
	if (texture == NULL || texture->h <= 0 || texture->w <= 0) {
		dw_free_texture(texture);
		return false;
	}
	int64_t width = (int64_t)texture->w * text_height / texture->h;
	if (width <= 0 || width > INT_MAX - row->rendered_width) {
		dw_free_texture(texture);
		return false;
	}
	if (row->fragment_count == INT_MAX ||
	    (size_t)(row->fragment_count + 1) > SIZE_MAX / sizeof(*row->fragments)) {
		dw_free_texture(texture);
		return false;
	}
	tx_text_fragment_t *fragments = realloc(
	    row->fragments, (size_t)(row->fragment_count + 1) * sizeof(*fragments));
	if (fragments == NULL) {
		dw_free_texture(texture);
		return false;
	}
	row->fragments = fragments;
	row->fragments[row->fragment_count++] = (tx_text_fragment_t){
	    .texture = texture, .x_offset = row->rendered_width};
	row->rendered_width += (int)width;
	return true;
}

static void tx_free_row(tx_styled_row_t *row)
{
	if (row == NULL) {
		return;
	}
	for (int index = 0; index < row->fragment_count; index++) {
		dw_free_texture(row->fragments[index].texture);
	}
	free(row->fragments);
	*row = (tx_styled_row_t){0};
}

void tx_free_styled_text(tx_styled_text_t *message)
{
	if (message == NULL) {
		return;
	}
	for (int index = 0; index < message->row_count; index++) {
		tx_free_row(&message->rows[index]);
	}
	free(message->rows);
	free(message);
}

static bool tx_append_row(tx_styled_text_t *message, tx_styled_row_t *row)
{
	if (message->row_count == INT_MAX ||
	    (size_t)(message->row_count + 1) > SIZE_MAX / sizeof(*message->rows)) {
		return false;
	}
	tx_styled_row_t *rows = realloc(
	    message->rows, (size_t)(message->row_count + 1) * sizeof(*rows));
	if (rows == NULL) {
		return false;
	}
	message->rows = rows;
	message->rows[message->row_count++] = *row;
	*row = (tx_styled_row_t){0};
	return true;
}

static bool tx_append_empty_styled_row(tx_styled_text_t *message)
{
	tx_styled_row_t row = {0};
	return tx_append_row(message, &row);
}

static bool tx_create_row_from_span(tx_styled_text_t *message,
									const char *line,
									const tx_styled_run_t *runs,
									size_t run_count,
									size_t start, size_t end,
									int text_height)
{
	tx_styled_row_t row = {0};
	for (size_t index = 0; index < run_count; index++) {
		const tx_styled_run_t *run = &runs[index];
		size_t run_end = run->start + run->length;
		size_t part_start = run->start > start ? run->start : start;
		size_t part_end = run_end < end ? run_end : end;
		if (part_start >= part_end) {
			continue;
		}
		SDL_Color color = run->token_class == TX_TOKEN_CLASS_TEXT
		                      ? tx_get_line_style_color(run->style)
		                      : tx_get_token_color(run->token_class);
		if (!tx_row_append_fragment(&row, line + part_start,
		                            part_end - part_start, color, text_height)) {
			tx_free_row(&row);
			return false;
		}
	}
	if (!tx_append_row(message, &row)) {
		tx_free_row(&row);
		return false;
	}
	return true;
}

static size_t tx_next_utf8_character(const char *text, size_t position,
									 size_t end)
{
	unsigned char first = (unsigned char)text[position];
	size_t count = first < 0x80 ? 1 : (first & 0xE0) == 0xC0 ? 2 :
	               (first & 0xF0) == 0xE0 ? 3 : (first & 0xF8) == 0xF0 ? 4 : 1;
	if (count > end - position) {
		return position + 1;
	}
	for (size_t index = 1; index < count; index++) {
		if (((unsigned char)text[position + index] & 0xC0) != 0x80) {
			return position + 1;
		}
	}
	return position + count;
}

static bool tx_wrap_styled_line(tx_styled_text_t *message, const char *line,
								size_t line_length,
								const tx_styled_run_t *runs,
								size_t run_count, int content_width,
								int text_height)
{
	size_t row_start = 0;
	while (row_start < line_length) {
		size_t cursor = row_start;
		size_t last_fit = row_start;
		while (cursor < line_length) {
			size_t word_end = cursor;
			while (word_end < line_length &&
			       !isspace((unsigned char)line[word_end])) {
				word_end = tx_next_utf8_character(line, word_end, line_length);
			}
			size_t chunk_end = word_end;
			while (chunk_end < line_length &&
			       isspace((unsigned char)line[chunk_end])) {
				chunk_end++;
			}
			int measured = 0;
			if (!tx_measure_span(line, row_start, chunk_end, text_height,
			                     &measured)) {
				return false;
			}
			if (measured <= content_width) {
				cursor = chunk_end;
				last_fit = cursor;
				continue;
			}
			if (last_fit > row_start) {
				if (!tx_create_row_from_span(message, line, runs, run_count,
				                             row_start, last_fit, text_height)) {
					return false;
				}
				row_start = last_fit;
				break;
			}
			if (word_end == row_start) {
				word_end = tx_next_utf8_character(line, row_start, line_length);
			}
			size_t split = row_start;
			for (size_t next = row_start; next < word_end;) {
				next = tx_next_utf8_character(line, next, word_end);
				if (!tx_measure_span(line, row_start, next, text_height,
				                     &measured)) {
					return false;
				}
				if (measured > content_width) {
					if (split == row_start) {
						split = next;
					}
					break;
				}
				split = next;
			}
			if (split == row_start ||
			    !tx_create_row_from_span(message, line, runs, run_count,
			                            row_start, split, text_height)) {
				return false;
			}
			row_start = split;
			break;
		}
		if (cursor == line_length) {
			if (!tx_create_row_from_span(message, line, runs, run_count,
			                             row_start, line_length, text_height)) {
				return false;
			}
			row_start = line_length;
		}
	}
	return true;
}

tx_styled_text_t *tx_create_styled_text_box_message(
	const tx_text_box_options_t *options, const char *message,
	tx_text_layout_info_t *layout_info)
{
	if (layout_info != NULL) {
		*layout_info = (tx_text_layout_info_t){0};
	}
	if (options == NULL || message == NULL || message[0] == '\0') {
		return NULL;
	}
	SDL_Rect content;
	int text_height = 0;
	if (!tx_get_text_box_rects(options, NULL, &content, &text_height) ||
	    content.w <= 0 || content.h <= 0 || text_height <= 0) {
		return NULL;
	}
	size_t message_length = strlen(message);
	if (message_length == SIZE_MAX) {
		return NULL;
	}
	char *line = malloc(message_length + 1);
	tx_styled_text_t *result = calloc(1, sizeof(*result));
	if (line == NULL || result == NULL) {
		free(line);
		tx_free_styled_text(result);
		return NULL;
	}
	size_t line_start = 0;
	while (line_start <= message_length) {
		size_t line_end = line_start;
		while (line_end < message_length && message[line_end] != '\n') {
			line_end++;
		}
		size_t line_length = line_end - line_start;
		memcpy(line, message + line_start, line_length);
		line[line_length] = '\0';
		tx_text_style_t line_style = tx_parse_tutorial_line(line);
		line_length = strlen(line);
		if (line_length == 0) {
			if (!tx_append_empty_styled_row(result)) {
				goto failure;
			}
		} else {
			size_t run_count = 0;
			tx_styled_run_t *runs = tx_tokenize_line(
			    line, line_length, line_style, &run_count);
			if (runs == NULL ||
			    !tx_wrap_styled_line(result, line, line_length, runs, run_count,
			                         content.w, text_height)) {
				free(runs);
				goto failure;
			}
			free(runs);
		}
		if (line_end == message_length) {
			break;
		}
		line_start = line_end + 1;
	}
	int max_rows = content.h / text_height;
	int visible_rows = result->row_count < max_rows ? result->row_count : max_rows;
	if (layout_info != NULL) {
		layout_info->required_rows = result->row_count;
		layout_info->visible_rows = visible_rows;
		layout_info->overflow_rows = result->row_count - visible_rows;
		layout_info->overflowed = layout_info->overflow_rows > 0;
	}
	free(line);
	return result;

failure:
	free(line);
	tx_free_styled_text(result);
	return NULL;
}

void tx_draw_styled_text_box_message(const tx_text_box_options_t *options,
									const tx_styled_text_t *message, int header)
{
	if (message == NULL || message->rows == NULL || message->row_count <= 0 ||
	    g_renderer == NULL) {
		return;
	}
	SDL_Rect box;
	SDL_Rect content;
	int text_height = 0;
	if (!tx_get_text_box_rects(options, &box, &content, &text_height) ||
	    content.w <= 0 || content.h <= 0 || text_height <= 0) {
		return;
	}
	for (int row_index = 0; row_index < message->row_count; row_index++) {
		const tx_styled_row_t *row = &message->rows[row_index];
		if (row->fragment_count < 0 || row->rendered_width < 0 ||
		    (row->fragment_count > 0 && row->fragments == NULL)) {
			return;
		}
		for (int fragment = 0; fragment < row->fragment_count; fragment++) {
			const tx_text_fragment_t *part = &row->fragments[fragment];
			if (part->texture == NULL || part->texture->texture == NULL ||
			    part->texture->w <= 0 || part->texture->h <= 0 ||
			    part->x_offset < 0) {
				return;
			}
		}
	}
	dw_draw_iface_box(box, tx_get_header_texture(header));
	int visible_rows = content.h / text_height;
	if (visible_rows > message->row_count) {
		visible_rows = message->row_count;
	}
	int64_t y = content.y;
	if (message->row_count <= content.h / text_height) {
		y += ((int64_t)content.h - (int64_t)message->row_count * text_height) / 2;
	}
	SDL_Rect old_clip;
	SDL_RenderGetClipRect(g_renderer, &old_clip);
	SDL_bool clipping_was_enabled = SDL_RenderIsClipEnabled(g_renderer);
	SDL_Rect drawing_clip = content;
	if (clipping_was_enabled) {
		SDL_IntersectRect(&content, &old_clip, &drawing_clip);
	}
	if (SDL_RenderSetClipRect(g_renderer, &drawing_clip) != 0) {
		SDL_RenderSetClipRect(g_renderer,
		                      clipping_was_enabled ? &old_clip : NULL);
		return;
	}
	for (int row_index = 0; row_index < visible_rows; row_index++) {
		const tx_styled_row_t *row = &message->rows[row_index];
		int64_t row_x = (int64_t)content.x +
		                ((int64_t)content.w - row->rendered_width) / 2;
		for (int fragment = 0; fragment < row->fragment_count; fragment++) {
			const tx_text_fragment_t *part = &row->fragments[fragment];
			int64_t x = row_x + part->x_offset;
			if (x < INT_MIN || x > INT_MAX || y < INT_MIN || y > INT_MAX) {
				continue;
			}
			SDL_Rect destination = {
				.x = (int)x, .y = (int)y, .w = content.w, .h = text_height};
			dw_draw_texture_fit_h(destination, part->texture);
		}
		y += text_height;
	}
	SDL_RenderSetClipRect(g_renderer,
	                      clipping_was_enabled ? &old_clip : NULL);
}

/* Function: tx_text_box
 * -----------------------------------------------------------------------------
 * This function displays a message box in differents part of the screen
 * according to an identifies.
 * Important to notice that the width of the message is defined when te text
 * texture is created.
 *
 * Arguments:
 *	pos: The position id of where is gonna be displayed.
 *	msg_id: The id of the message that will be shown
 *  header: The header that will accompany the text box
 *
 * Return:
 *	Void.
 */
void tx_text_box(int position, int message_id, int header)
{
	if (message_id < 0 || message_id >= g_msgs_size) {
		fprintf(stderr, "tx_text_box: invalid message id %d\n", message_id);
		return;
	}

	texture_array_t *message = g_msgs[message_id];

	if (message == NULL) {
		fprintf(stderr, "tx_text_box: message %d has no loaded text\n",
		        message_id);
		return;
	}

	tx_text_box_options_t options = {
	    .position = position,
	    .large_box = false,
	    .large_text = false,
	};
	tx_text_box_texture(&options, message, header);
}
