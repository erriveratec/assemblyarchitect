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
	b.x = dm_get_stage_imm_up().x + 12 * dm_get_value_box_wh().w;
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
texture_array_t *tx_get_message_texture(int pos) { return g_msgs[pos]; }

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
	assert(size > 0 && "Negative size");
	g_msgs_size = size;
	g_msgs      = calloc(size, sizeof(texture_array_t *));
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
	if (position < 0 || position >= g_msgs_size) {
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
	for (int i = 0; i < g_msgs_size; i++) {
		dw_free_texture_array(g_msgs[i]);
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

static SDL_Color tx_get_style_color(tx_text_style_t style)
{
	/* Semantic roles reuse the shared palette rather than defining new colors. */
	switch (style) {
	case TX_TEXT_STYLE_SYNTAX:
		return C_TERMINAL_GREEN;
	case TX_TEXT_STYLE_DIRECTIVE:
		return C_AMBER;
	case TX_TEXT_STYLE_WARNING:
		return C_ORANGE;
	case TX_TEXT_STYLE_BODY:
	default:
		return C_WHITE;
	}
}

static bool tx_append_empty_row(texture_array_t *destination)
{
	if (destination->size == INT_MAX ||
	    (size_t)(destination->size + 1) > SIZE_MAX / sizeof(*destination->t)) {
		return false;
	}

	texture_t **rows = realloc(destination->t,
	                           (size_t)(destination->size + 1) * sizeof(*rows));
	if (rows == NULL) {
		return false;
	}

	destination->t = rows;
	destination->t[destination->size] = NULL;
	destination->size++;
	return true;
}

static bool tx_append_wrapped_rows(texture_array_t *destination,
	                                  texture_array_t *wrapped)
{
	if (wrapped->size <= 0 || wrapped->t == NULL ||
	    wrapped->size > INT_MAX - destination->size ||
	    (size_t)(destination->size + wrapped->size) >
	        SIZE_MAX / sizeof(*destination->t)) {
		return false;
	}

	int total_size = destination->size + wrapped->size;
	texture_t **rows = realloc(destination->t,
	                           (size_t)total_size * sizeof(*rows));
	if (rows == NULL) {
		return false;
	}

	destination->t = rows;
	memcpy(destination->t + destination->size, wrapped->t,
	       (size_t)wrapped->size * sizeof(*rows));
	destination->size = total_size;
	free(wrapped->t);
	free(wrapped);
	return true;
}

texture_array_t *tx_create_styled_text_box_message(
	const tx_text_box_options_t *options, const char *message)
{
	if (message == NULL || message[0] == '\0') {
		return NULL;
	}

	SDL_Rect content;
	int      text_height = 0;
	if (!tx_get_text_box_rects(options, NULL, &content, &text_height)) {
		return NULL;
	}

	size_t message_length = strlen(message);
	if (message_length == SIZE_MAX) {
		return NULL;
	}

	char *line = malloc(message_length + 1);
	texture_array_t *result = calloc(1, sizeof(*result));
	if (line == NULL || result == NULL) {
		free(line);
		free(result);
		return NULL;
	}

	int max_rows = content.h / text_height;
	size_t line_start = 0;
	while (line_start <= message_length) {
		size_t line_end = line_start;
		while (line_end < message_length && message[line_end] != '\n') {
			line_end++;
		}

		size_t line_length = line_end - line_start;
		memcpy(line, message + line_start, line_length);
		line[line_length] = '\0';
		tx_text_style_t style = tx_parse_tutorial_line(line);

		if (line[0] == '\0') {
			if (result->size >= max_rows || !tx_append_empty_row(result)) {
				goto failure;
			}
		} else {
			texture_array_t *wrapped = dw_create_text_tex_array_by_h(
			    content.w, text_height, tx_get_style_color(style), line);
			if (wrapped == NULL) {
				goto failure;
			}

			bool row_overflow = wrapped->size > max_rows - result->size;
			for (int row = 0; !row_overflow && row < wrapped->size; row++) {
				texture_t *texture = wrapped->t[row];
				if (texture != NULL &&
				    (texture->h <= 0 ||
				     (int64_t)texture->w * text_height / texture->h >
				         content.w)) {
					row_overflow = true;
				}
			}
			if (row_overflow || !tx_append_wrapped_rows(result, wrapped)) {
				dw_free_texture_array(wrapped);
				goto failure;
			}
		}

		if (line_end == message_length) {
			break;
		}
		line_start = line_end + 1;
	}

	free(line);
	return result;

failure:
	free(line);
	dw_free_texture_array(result);
	return NULL;
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
