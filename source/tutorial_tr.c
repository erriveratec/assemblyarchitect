#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "arrow_ar.h"
#include "aux.h"
#include "mouse_ms.h"
#include "text_tx.h"
#include "tutorial_tr.h"
#include "code_line_cl.h"

#define TUTORIAL_PATH_FORMAT "data/levels/%02d/tutorial.cfg"
#define TUTORIAL_MAX_STEPS 32

static tutorial_step_t g_steps[TUTORIAL_MAX_STEPS];
static int g_step_count;

/*
 * Renders the first active tutorial step that matches
 * the current gameplay state.
 */
void tr_update(const cs_context_t *context)
{
    if (context == NULL) {
        return;
    }

    const tutorial_step_t *step =
       tr_get_matching_step(context);

    if (step == NULL) {
        return;    }

    tr_render_step(step->name);
}


static char *trim(char *text)
{
    while (isspace((unsigned char)*text)) text++;
    char *end = text + strlen(text);
    while (end > text && isspace((unsigned char)end[-1])) end--;
    *end = '\0';
    return text;
}

static bool parse_box(const char *text, tutorial_box_t *box)
{
    if (strcmp(text, "big") == 0) *box = TUTORIAL_BOX_BIG;
    else if (strcmp(text, "upper") == 0) *box = TUTORIAL_BOX_UPPER;
    else if (strcmp(text, "upper_right") == 0) *box = TUTORIAL_BOX_UPPER_RIGHT;
    else if (strcmp(text, "center") == 0) *box = TUTORIAL_BOX_CENTER;
    else if (strcmp(text, "center_right") == 0) *box = TUTORIAL_BOX_CENTER_RIGHT;
    else if (strcmp(text, "lower") == 0) *box = TUTORIAL_BOX_LOWER;
    else if (strcmp(text, "code") == 0) *box = TUTORIAL_BOX_CODE;
    else if (strcmp(text, "instruction") == 0) *box = TUTORIAL_BOX_INSTRUCTION;
    else return false;
    return true;
}

static bool parse_header(const char *text, tutorial_header_t *header)
{
    if (strcmp(text, "none") == 0) *header = TUTORIAL_HEADER_NONE;
    else if (strcmp(text, "system_message") == 0) *header = TUTORIAL_HEADER_SYSTEM_MESSAGE;
    else if (strcmp(text, "system_notice") == 0) *header = TUTORIAL_HEADER_SYSTEM_NOTICE;
    else if (strcmp(text, "system_warning") == 0) *header = TUTORIAL_HEADER_SYSTEM_WARNING;
    else if (strcmp(text, "instruction") == 0) *header = TUTORIAL_HEADER_INSTRUCTION;
    else return false;
    return true;
}

static bool parse_dismiss(const char *text, tutorial_dismiss_t *dismiss)
{
    if (strcmp(text, "none") == 0) *dismiss = TUTORIAL_DISMISS_NONE;
    else if (strcmp(text, "mouse_press") == 0) *dismiss = TUTORIAL_DISMISS_MOUSE_PRESS;
    else if (strcmp(text, "mouse_release") == 0) *dismiss = TUTORIAL_DISMISS_MOUSE_RELEASE;
    else return false;
    return true;
}

static bool parse_arrow(const char *text, int *arrow_id)
{
    if (strcmp(text, "none") == 0) *arrow_id = -1;
    else if (strcmp(text, "instruction") == 0) *arrow_id = AR_INS;
    else if (strcmp(text, "challenge") == 0) *arrow_id = AR_CHALLENGE;
    else if (strcmp(text, "drop") == 0) *arrow_id = AR_DROP;
    else if (strcmp(text, "zero_flag") == 0) *arrow_id = AR_ZF;
    else if (strcmp(text, "code") == 0) *arrow_id = AR_CODE;
    else if (strcmp(text, "delete") == 0) *arrow_id = AR_DEL;
    else if (strcmp(text, "operand_two") == 0) *arrow_id = AR_OP2;
    else if (strcmp(text, "input_buffer") == 0) *arrow_id = AR_IB;
    else if (strcmp(text, "output_buffer") == 0) *arrow_id = AR_OB;
    else if (strcmp(text, "register") == 0) *arrow_id = AR_REG;
    else if (strcmp(text, "play") == 0) *arrow_id = AR_PLAY;
    else if (strcmp(text, "step") == 0) *arrow_id = AR_STEP;
    else if (strcmp(text, "fast") == 0) *arrow_id = AR_FAST;
    else if (strcmp(text, "error") == 0) *arrow_id = AR_ERROR;
    else if (strcmp(text, "immediate") == 0) *arrow_id = AR_IMM_UP;
    else return false;
    return true;
}

static bool parse_instruction_id(
    const char *text,
    int *instruction_id
)
{
    if (text == NULL || instruction_id == NULL) {
        return false;
    }

    if (strcmp(text, "MOV") == 0) {
        *instruction_id = MOV;
    } else if (strcmp(text, "ADD") == 0) {
        *instruction_id = ADD;
    } else if (strcmp(text, "LABEL") == 0) {
        *instruction_id = LABEL;
    } else if (strcmp(text, "JMP") == 0) {
        *instruction_id = JMP;
    } else if (strcmp(text, "CMP") == 0) {
        *instruction_id = CMP;
    } else if (strcmp(text, "JE") == 0) {
        *instruction_id = JE;
    } else if (strcmp(text, "JNE") == 0) {
        *instruction_id = JNE;
    } else {
        return false;
    }

    return true;
}

static bool add_arrow(
    tutorial_step_t *step,
    const char *text
)
{
    if (step == NULL || text == NULL) {
        return false;
    }

    if (step->arrow_count >= TUTORIAL_MAX_ARROWS) {
        return false;
    }

    int arrow_id = -1;

    if (!parse_arrow(text, &arrow_id)) {
        return false;
    }

    if (arrow_id < 0) {
        return true;
    }

    step->arrow_ids[step->arrow_count] = arrow_id;
    step->arrow_count++;

    return true;
}

static bool parse_arrows(
    tutorial_step_t *step,
    const char *text
)
{
    if (step == NULL || text == NULL) {
        return false;
    }

    char buffer[128];

    int copied = snprintf(
        buffer,
        sizeof(buffer),
        "%s",
        text
    );

    if (copied < 0 ||
        copied >= (int)sizeof(buffer)) {
        return false;
    }

    step->arrow_count = 0;

    char *saveptr = NULL;

    char *token = strtok_r(
        buffer,
        ",",
        &saveptr
    );

    while (token != NULL) {
        char *arrow_name = trim(token);

        if (!add_arrow(step, arrow_name)) {
            step->arrow_count = 0;
            return false;
        }

        token = strtok_r(
            NULL,
            ",",
            &saveptr
        );
    }

    return true;
}


static tutorial_step_t *find_step(const char *name)
{
    for (int index = 0; index < g_step_count; index++) {
        if (strcmp(g_steps[index].name, name) == 0) return &g_steps[index];
    }
    return NULL;
}

void tr_clear(void)
{
    memset(g_steps, 0, sizeof(g_steps));
    g_step_count = 0;
}

int tr_get_step_count(void) { return g_step_count; }

const tutorial_step_t *tr_get_step(int index)
{
    if (index < 0 || index >= g_step_count) return NULL;
    return &g_steps[index];
}

const tutorial_step_t *tr_get_named_step(const char *name)
{
    return find_step(name);
}

bool tr_is_active(const char *name)
{
    tutorial_step_t *step = find_step(name);
    return step != NULL && step->active;
}

bool tr_step_matches_current_state(const char *name,
                                   const cs_context_t *context)
{
    tutorial_step_t *step = find_step(name);
    if (step == NULL || !step->active || context == NULL) return false;
    if (step->when_code_size >= 0 && step->when_code_size != context->code_size) return false;
    if (step->when_holding >= 0 && step->when_holding != context->holding_instruction) return false;
    if (step->when_held_instruction_id >= 0 && step->when_held_instruction_id !=
        context->held_instruction_id) {
        return false;
    }
    if (step->when_held_instruction_not_id >= 0 && step->when_held_instruction_not_id ==
        context->held_instruction_id) {
        return false;
    }
    if (step->when_operand_pending >= 0 && step->when_operand_pending != context->operand_pending) return false;
    if (step->when_code_sorted >= 0 && step->when_code_sorted != context->code_sorted) return false;
    if (step->when_play_state >= 0 && step->when_play_state != context->playing) return false;
    if (step->when_operation_flag >= 0 && step->when_operation_flag != context->operation_flag) return false;
    return true;
}

const tutorial_step_t *tr_get_matching_step(const cs_context_t *context)
{
    if (context == NULL) return NULL;
    for (int index = 0; index < g_step_count; index++) {
        if (tr_step_matches_current_state(g_steps[index].name, context)) return &g_steps[index];
    }
    return NULL;
}

void tr_deactivate(const char *name)
{
    tutorial_step_t *step = find_step(name);
    if (step != NULL) step->active = false;
}

static int get_text_box(tutorial_box_t box)
{
    switch (box) {
        case TUTORIAL_BOX_BIG: return TX_BIG_BOX;
        case TUTORIAL_BOX_UPPER: return TX_UPPER_BOX;
        case TUTORIAL_BOX_UPPER_RIGHT: return TX_UPPER_RIGHT_BOX;
        case TUTORIAL_BOX_CENTER: return TX_CENTER_BOX;
        case TUTORIAL_BOX_CENTER_RIGHT: return TX_CENTER_RIGHT_BOX;
        case TUTORIAL_BOX_LOWER: return TX_LOWER_BOX;
        case TUTORIAL_BOX_CODE: return TX_CODE_BOX;
        case TUTORIAL_BOX_INSTRUCTION: return TX_INS_BOX;
    }
    return TX_BIG_BOX;
}

static int get_header(tutorial_header_t header) { return TX_NONE + header; }

void tr_render_step(const char *name)
{
    tutorial_step_t *step = find_step(name);
    if (step == NULL || !step->active) return;
    int step_id = (int)(step - g_steps);
    tx_text_box(get_text_box(step->box), step_id, get_header(step->header));
    for (int index = 0; index < step->arrow_count; index++) {
        ar_display_arrow(step->arrow_ids[index]);
    }
    if ((step->dismiss == TUTORIAL_DISMISS_MOUSE_PRESS && ms_left_pressed()) ||
        (step->dismiss == TUTORIAL_DISMISS_MOUSE_RELEASE && ms_left_released())) {
        step->active = false;
        ms_reset_mouse_values();
    }
}

bool tr_load_level(int level_id)
{
    char relative_path[64];
    char path[512];
    snprintf(relative_path, sizeof(relative_path), TUTORIAL_PATH_FORMAT, level_id);
    ax_get_resource_path(path, sizeof(path), relative_path);
    FILE *file = fopen(path, "r");
    if (file == NULL) return false;

    tr_clear();
    char line[512];
    tutorial_step_t *step = NULL;
    bool reading_text = false;
    while (fgets(line, sizeof(line), file) != NULL) {
        char *text = trim(line);
        if (text[0] == '[' && strcmp(text, "[tutorial]") != 0) {
            if (g_step_count == TUTORIAL_MAX_STEPS) break;
            step = &g_steps[g_step_count++];
            sscanf(text, "[%63[^]]]", step->name);
            step->arrow_count = 0;
            step->when_code_size = -1;
            step->when_holding = -1;
            step->when_held_instruction_id = -1;
            step->when_held_instruction_not_id = -1;
            step->when_operand_pending = -1;
            step->when_code_sorted = -1;
            step->when_play_state = -1;
            step->when_operation_flag = -1;
            step->effect_code_editable = -1;
            step->effect_buffer_selectable = -1;
            step->effect_register_selectable = -1;
            step->effect_arrange_enabled = -1;
            step->effect_delete_enabled = -1;
            step->active = true;
            reading_text = false;
            continue;
        }
        if (step == NULL) continue;
        if (strcmp(text, "text_begin") == 0) { reading_text = true; continue; }
        if (strcmp(text, "text_end") == 0) { reading_text = false; continue; }
        if (reading_text) {
            size_t used = strlen(step->text);
            snprintf(step->text + used, sizeof(step->text) - used, "%s%s", used == 0 ? "" : "\n", text);
            continue;
        }
        char *equals = strchr(text, '=');
        if (equals == NULL) continue;
        *equals = '\0';
        char *key = trim(text);
        char *value = trim(equals + 1);
        if (strcmp(key, "box") == 0 && !parse_box(value, &step->box)) goto invalid;
        if (strcmp(key, "header") == 0 && !parse_header(value, &step->header)) goto invalid;
        if (strcmp(key, "dismiss") == 0 && !parse_dismiss(value, &step->dismiss)) goto invalid;
        if (strcmp(key, "arrow") == 0) {
         if (!parse_arrows(step, value)) {
            goto invalid;
            }
        }
        if (strcmp(key, "arrows") == 0) {
            if (!parse_arrows(step, value)) {
            goto invalid;
            }
        }
        if (strcmp(key, "when.code_size") == 0) step->when_code_size = atoi(value);
        if (strcmp(key, "when.holding") == 0) step->when_holding = atoi(value);
        if (strcmp(key, "when.held_instruction") == 0) {
        if (!parse_instruction_id(
            value,
            &step->when_held_instruction_id)) {
            goto invalid;
            }
        }
        if (strcmp(key, "when.held_instruction_not") == 0) {
            if (!parse_instruction_id(
            value,
            &step->when_held_instruction_not_id)) {
            goto invalid;
            }
        }
        if (strcmp(key, "when.operand_pending") == 0) step->when_operand_pending = atoi(value);
        if (strcmp(key, "when.code_sorted") == 0) step->when_code_sorted = atoi(value);
        if (strcmp(key, "when.play_state") == 0) step->when_play_state = atoi(value);
        if (strcmp(key, "when.operation_flag") == 0) step->when_operation_flag = atoi(value);
        if (strcmp(key, "effects.code_editable") == 0) step->effect_code_editable = atoi(value);
        if (strcmp(key, "effects.buffer_selectable") == 0) step->effect_buffer_selectable = atoi(value);
        if (strcmp(key, "effects.register_selectable") == 0) step->effect_register_selectable = atoi(value);
        if (strcmp(key, "effects.arrange_enabled") == 0) step->effect_arrange_enabled = atoi(value);
        if (strcmp(key, "effects.delete_enabled") == 0) step->effect_delete_enabled = atoi(value);
    }
    fclose(file);
    return g_step_count > 0;

invalid:
    fprintf(stderr, "tutorial.cfg: level %d has invalid metadata\n", level_id);
    fclose(file);
    tr_clear();
    return false;
}
