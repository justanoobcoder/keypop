#include "buffer.h"
#include <string.h>

static void buf_shift_left(struct client_state *state) {
    if (state->seg_count == 0)
        return;
    int len_to_remove = state->seg_lengths[0];
    memmove(state->display_buf, state->display_buf + len_to_remove,
            state->display_len - len_to_remove + 1);
    state->display_len -= len_to_remove;
    memmove(state->seg_lengths, state->seg_lengths + 1,
            (state->seg_count - 1) * sizeof(int));
    state->seg_count--;
}

void buf_append(struct client_state *state, const char *text) {
    size_t text_len = strlen(text);
    if (text_len == 0)
        return;
    if (state->seg_count >= MAX_SEGMENTS)
        buf_shift_left(state);
    while (state->display_len + text_len >= MAX_DISPLAY_LEN) {
        buf_shift_left(state);
        if (state->seg_count == 0) {
            state->display_len = 0;
            break;
        }
    }
    memcpy(state->display_buf + state->display_len, text, text_len + 1);
    state->display_len += text_len;
    state->seg_lengths[state->seg_count++] = text_len;
}

void buf_pop_last_seg(struct client_state *state) {
    if (state->seg_count == 0)
        return;
    int len_to_remove = state->seg_lengths[state->seg_count - 1];
    if (len_to_remove > (int)state->display_len)
        len_to_remove = state->display_len;
    state->display_len -= len_to_remove;
    state->display_buf[state->display_len] = '\0';
    state->seg_count--;
}

void buf_backspace(struct client_state *state) {
    if (state->seg_count == 0)
        return;
    int len_to_remove = state->seg_lengths[--state->seg_count];
    if (len_to_remove > (int)state->display_len)
        len_to_remove = state->display_len;
    state->display_len -= len_to_remove;
    state->display_buf[state->display_len] = '\0';
}

void buf_delete_word(struct client_state *state) {
    if (state->seg_count == 0)
        return;

    int deleted_something = 0;

    while (state->seg_count > 0) {
        int last_seg_idx = state->seg_count - 1;
        int last_len = state->seg_lengths[last_seg_idx];
        char *last_seg_start =
            state->display_buf + state->display_len - last_len;
        if (strcmp(last_seg_start, " ") == 0) {
            buf_backspace(state);
            deleted_something = 1;
        } else {
            break;
        }
    }

    while (state->seg_count > 0) {
        int last_seg_idx = state->seg_count - 1;
        int last_len = state->seg_lengths[last_seg_idx];
        char *last_seg_start =
            state->display_buf + state->display_len - last_len;
        if (strcmp(last_seg_start, " ") == 0) {
            break;
        }
        buf_backspace(state);
        deleted_something = 1;
    }

    (void)deleted_something;
}
