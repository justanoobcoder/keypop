#define _POSIX_C_SOURCE 200809L
#define _USE_MATH_DEFINES
#include "draw.h"
#include "shm.h"
#include <cairo.h>
#include <math.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

// Helper to separate modifiers from key
// e.g., "Ctrl+Alt+Enter" -> mods="Ctrl+Alt+", key="Enter"
static void parse_segment(const char *segment, char *mods, char *key) {
    mods[0] = '\0';
    key[0] = '\0';

    const char *last_plus = strrchr(segment, '+');
    if (last_plus) {
        size_t len = strlen(segment);
        if (len > 0 && segment[len - 1] == '+') {
            strcpy(key, segment);
            return;
        }
        int mod_len = last_plus - segment + 1;
        strncpy(mods, segment, mod_len);
        mods[mod_len] = '\0';
        strcpy(key, last_plus + 1);
    } else {
        strcpy(key, segment);
    }
}

// Draw specific icons
static void draw_icon(cairo_t *cr, const char *key_name, double x, double y,
                      double size, const double *color) {
    cairo_save(cr);
    cairo_set_source_rgba(cr, color[0], color[1], color[2], color[3]);
    cairo_set_line_width(cr, size * 0.08);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);

    if (strcmp(key_name, "Enter") == 0) {
        double cx = x + size * 0.5;
        double cy = y - size * 0.3;
        double w = size * 0.4;
        double h = size * 0.3;

        cairo_move_to(cr, cx + w, cy - h);
        cairo_line_to(cr, cx, cy - h);
        cairo_line_to(cr, cx, cy);
        cairo_line_to(cr, cx - w / 2, cy);

        cairo_move_to(cr, cx - w / 2 + size * 0.1, cy - size * 0.1);
        cairo_line_to(cr, cx - w / 2, cy);
        cairo_line_to(cr, cx - w / 2 + size * 0.1, cy + size * 0.1);
        cairo_stroke(cr);

    } else if (strcmp(key_name, "Left") == 0) {
        double cx = x + size * 0.5;
        double cy = y - size * 0.3;
        double w = size * 0.3;

        cairo_move_to(cr, cx + w, cy);
        cairo_line_to(cr, cx - w, cy);
        cairo_move_to(cr, cx - w + size * 0.15, cy - size * 0.15);
        cairo_line_to(cr, cx - w, cy);
        cairo_line_to(cr, cx - w + size * 0.15, cy + size * 0.15);
        cairo_stroke(cr);

    } else if (strcmp(key_name, "Right") == 0) {
        double cx = x + size * 0.5;
        double cy = y - size * 0.3;
        double w = size * 0.3;

        cairo_move_to(cr, cx - w, cy);
        cairo_line_to(cr, cx + w, cy);
        cairo_move_to(cr, cx + w - size * 0.15, cy - size * 0.15);
        cairo_line_to(cr, cx + w, cy);
        cairo_line_to(cr, cx + w - size * 0.15, cy + size * 0.15);
        cairo_stroke(cr);

    } else if (strcmp(key_name, "Up") == 0) {
        double cx = x + size * 0.5;
        double cy = y - size * 0.3;
        double h = size * 0.3;

        cairo_move_to(cr, cx, cy + h);
        cairo_line_to(cr, cx, cy - h);
        cairo_move_to(cr, cx - size * 0.15, cy - h + size * 0.15);
        cairo_line_to(cr, cx, cy - h);
        cairo_line_to(cr, cx + size * 0.15, cy - h + size * 0.15);
        cairo_stroke(cr);

    } else if (strcmp(key_name, "Down") == 0) {
        double cx = x + size * 0.5;
        double cy = y - size * 0.3;
        double h = size * 0.3;

        cairo_move_to(cr, cx, cy - h);
        cairo_line_to(cr, cx, cy + h);
        cairo_move_to(cr, cx - size * 0.15, cy + h - size * 0.15);
        cairo_line_to(cr, cx, cy + h);
        cairo_line_to(cr, cx + size * 0.15, cy + h - size * 0.15);
        cairo_stroke(cr);

    } else if (strcmp(key_name, "Tab") == 0) {
        double cx = x + size * 0.5;
        double cy = y - size * 0.3;
        double w = size * 0.3;

        cairo_move_to(cr, cx - w, cy);
        cairo_line_to(cr, cx + w, cy);
        cairo_move_to(cr, cx + w, cy - size * 0.15);
        cairo_line_to(cr, cx + w, cy + size * 0.15);
        cairo_move_to(cr, cx + w - size * 0.15, cy - size * 0.15);
        cairo_line_to(cr, cx + w, cy);
        cairo_line_to(cr, cx + w - size * 0.15, cy + size * 0.15);
        cairo_stroke(cr);

    } else if (strcmp(key_name, "Backspace") == 0 ||
               strcmp(key_name, "BackSpace") == 0) {
        // ⌫ shape: left-pointing pentagon with X inside
        double cx = x + size * 0.55;
        double cy = y - size * 0.3;
        double w = size * 0.35;   // half-width of box
        double h = size * 0.22;   // half-height of box
        double tip = size * 0.18; // depth of left arrow tip

        // Pentagon outline (arrow pointing left)
        cairo_move_to(cr, cx - w, cy);           // left tip
        cairo_line_to(cr, cx - w + tip, cy - h); // top-left
        cairo_line_to(cr, cx + w, cy - h);       // top-right
        cairo_line_to(cr, cx + w, cy + h);       // bottom-right
        cairo_line_to(cr, cx - w + tip, cy + h); // bottom-left
        cairo_close_path(cr);
        cairo_stroke(cr);

        // X inside (centered in right portion of shape)
        double xc = cx + size * 0.1;
        double xs = h * 0.55;
        cairo_move_to(cr, xc - xs, cy - xs);
        cairo_line_to(cr, xc + xs, cy + xs);
        cairo_move_to(cr, xc + xs, cy - xs);
        cairo_line_to(cr, xc - xs, cy + xs);
        cairo_stroke(cr);

    } else if (strcmp(key_name, "Del") == 0 ||
               strcmp(key_name, "Delete") == 0) {
        double cx = x + size * 0.5;
        double cy = y - size * 0.3;
        double w = size * 0.25;

        cairo_move_to(cr, cx - w, cy);
        cairo_line_to(cr, cx + w, cy);
        cairo_move_to(cr, cx + w - size * 0.12, cy - size * 0.12);
        cairo_line_to(cr, cx + w, cy);
        cairo_line_to(cr, cx + w - size * 0.12, cy + size * 0.12);

        cairo_move_to(cr, cx - w, cy - size * 0.08);
        cairo_line_to(cr, cx - w + size * 0.1, cy + size * 0.08);
        cairo_move_to(cr, cx - w + size * 0.1, cy - size * 0.08);
        cairo_line_to(cr, cx - w, cy + size * 0.08);
        cairo_stroke(cr);

    } else if (strncmp(key_name, "F", 1) == 0 && strlen(key_name) >= 2 &&
               strlen(key_name) <= 3) {
        double cx = x + size * 0.5;
        double cy = y - size * 0.3;
        double box_w = size * 0.45;
        double box_h = size * 0.35;

        cairo_set_line_width(cr, size * 0.05);
        cairo_rectangle(cr, cx - box_w / 2, cy - box_h / 2, box_w, box_h);
        cairo_stroke(cr);
        cairo_restore(cr);
        return;

    } else if (strcmp(key_name, "Caps") == 0) {
        double cx = x + size * 0.5;
        double cy = y - size * 0.3;
        double h = size * 0.25;

        cairo_move_to(cr, cx, cy - h);
        cairo_line_to(cr, cx - size * 0.1, cy - h + size * 0.12);
        cairo_move_to(cr, cx, cy - h);
        cairo_line_to(cr, cx + size * 0.1, cy - h + size * 0.12);

        cairo_move_to(cr, cx - size * 0.15, cy + h);
        cairo_line_to(cr, cx, cy);
        cairo_line_to(cr, cx + size * 0.15, cy + h);
        cairo_move_to(cr, cx - size * 0.08, cy + h * 0.4);
        cairo_line_to(cr, cx + size * 0.08, cy + h * 0.4);
        cairo_stroke(cr);

    } else if (strcmp(key_name, "Home") == 0) {
        double cx = x + size * 0.5;
        double cy = y - size * 0.3;
        double w = size * 0.25;

        cairo_move_to(cr, cx - w, cy);
        cairo_line_to(cr, cx, cy - w);
        cairo_line_to(cr, cx + w, cy);
        cairo_move_to(cr, cx - w * 0.8, cy);
        cairo_line_to(cr, cx - w * 0.8, cy + w);
        cairo_line_to(cr, cx + w * 0.8, cy + w);
        cairo_line_to(cr, cx + w * 0.8, cy);
        cairo_stroke(cr);

    } else if (strcmp(key_name, "End") == 0) {
        double cx = x + size * 0.5;
        double cy = y - size * 0.3;
        double w = size * 0.25;

        cairo_move_to(cr, cx - w, cy - w);
        cairo_line_to(cr, cx - w, cy + w);
        cairo_line_to(cr, cx + w, cy + w);
        cairo_move_to(cr, cx + w - size * 0.12, cy + w - size * 0.12);
        cairo_line_to(cr, cx + w, cy + w);
        cairo_move_to(cr, cx + w, cy + w);
        cairo_line_to(cr, cx + w - size * 0.12, cy + w + size * 0.12);
        cairo_stroke(cr);

    } else if (strcmp(key_name, "PgUp") == 0) {
        double cx = x + size * 0.5;
        double cy = y - size * 0.3;
        double h = size * 0.15;

        cairo_move_to(cr, cx, cy - h);
        cairo_line_to(cr, cx - size * 0.12, cy - h + size * 0.12);
        cairo_move_to(cr, cx, cy - h);
        cairo_line_to(cr, cx + size * 0.12, cy - h + size * 0.12);
        cairo_move_to(cr, cx, cy + h);
        cairo_line_to(cr, cx - size * 0.12, cy + h + size * 0.12);
        cairo_move_to(cr, cx, cy + h);
        cairo_line_to(cr, cx + size * 0.12, cy + h + size * 0.12);
        cairo_stroke(cr);

    } else if (strcmp(key_name, "PgDn") == 0) {
        double cx = x + size * 0.5;
        double cy = y - size * 0.3;
        double h = size * 0.15;

        cairo_move_to(cr, cx, cy - h);
        cairo_line_to(cr, cx - size * 0.12, cy - h - size * 0.12);
        cairo_move_to(cr, cx, cy - h);
        cairo_line_to(cr, cx + size * 0.12, cy - h - size * 0.12);
        cairo_move_to(cr, cx, cy + h);
        cairo_line_to(cr, cx - size * 0.12, cy + h - size * 0.12);
        cairo_move_to(cr, cx, cy + h);
        cairo_line_to(cr, cx + size * 0.12, cy + h - size * 0.12);
        cairo_stroke(cr);

    } else if (strcmp(key_name, "Esc") == 0) {
        double cx = x + size * 0.5;
        double cy = y - size * 0.3;
        double r = size * 0.25;

        cairo_arc(cr, cx, cy, r, 0, 2 * M_PI);
        cairo_stroke(cr);
        cairo_set_line_width(cr, size * 0.06);
        cairo_move_to(cr, cx - r * 0.5, cy - r * 0.5);
        cairo_line_to(cr, cx + r * 0.5, cy + r * 0.5);
        cairo_move_to(cr, cx + r * 0.5, cy - r * 0.5);
        cairo_line_to(cr, cx - r * 0.5, cy + r * 0.5);
        cairo_stroke(cr);

    } else if (strcmp(key_name, "Play") == 0) {
        double cx = x + size * 0.5;
        double cy = y - size * 0.3;
        double w = size * 0.2;

        cairo_move_to(cr, cx - w, cy - w);
        cairo_line_to(cr, cx + w, cy);
        cairo_line_to(cr, cx - w, cy + w);
        cairo_close_path(cr);
        cairo_stroke(cr);

    } else if (strcmp(key_name, "Pause") == 0) {
        double cx = x + size * 0.5;
        double cy = y - size * 0.3;
        double h = size * 0.3;
        double w = size * 0.06;

        cairo_rectangle(cr, cx - size * 0.12, cy - h / 2, w, h);
        cairo_rectangle(cr, cx + size * 0.06, cy - h / 2, w, h);
        cairo_stroke(cr);

    } else if (strcmp(key_name, "Vol+") == 0 || strcmp(key_name, "Bri+") == 0) {
        double cx = x + size * 0.5;
        double cy = y - size * 0.3;

        cairo_move_to(cr, cx - size * 0.15, cy);
        cairo_line_to(cr, cx + size * 0.15, cy);
        cairo_move_to(cr, cx, cy - size * 0.15);
        cairo_line_to(cr, cx, cy + size * 0.15);
        cairo_stroke(cr);

    } else if (strcmp(key_name, "Vol-") == 0 || strcmp(key_name, "Bri-") == 0) {
        double cx = x + size * 0.5;
        double cy = y - size * 0.3;

        cairo_move_to(cr, cx - size * 0.2, cy);
        cairo_line_to(cr, cx + size * 0.2, cy);
        cairo_stroke(cr);

    } else if (strcmp(key_name, "Mute") == 0) {
        double cx = x + size * 0.5;
        double cy = y - size * 0.3;

        cairo_move_to(cr, cx - size * 0.2, cy - size * 0.1);
        cairo_line_to(cr, cx - size * 0.05, cy - size * 0.2);
        cairo_line_to(cr, cx - size * 0.05, cy + size * 0.2);
        cairo_line_to(cr, cx - size * 0.2, cy + size * 0.1);
        cairo_close_path(cr);
        cairo_stroke(cr);

        cairo_move_to(cr, cx + size * 0.05, cy - size * 0.15);
        cairo_line_to(cr, cx + size * 0.2, cy + size * 0.15);
        cairo_move_to(cr, cx + size * 0.2, cy - size * 0.15);
        cairo_line_to(cr, cx + size * 0.05, cy + size * 0.15);
        cairo_stroke(cr);
    }

    cairo_restore(cr);
}

static int is_icon_key(const char *key) {
    if (strcmp(key, "Enter") == 0 || strcmp(key, "Left") == 0 ||
        strcmp(key, "Right") == 0 || strcmp(key, "Up") == 0 ||
        strcmp(key, "Down") == 0 || strcmp(key, "Tab") == 0 ||
        strcmp(key, "Backspace") == 0 || strcmp(key, "BackSpace") == 0 ||
        strcmp(key, "Del") == 0 || strcmp(key, "Delete") == 0 ||
        strcmp(key, "Caps") == 0 || strcmp(key, "Home") == 0 ||
        strcmp(key, "End") == 0 || strcmp(key, "PgUp") == 0 ||
        strcmp(key, "PgDn") == 0 || strcmp(key, "Play") == 0 ||
        strcmp(key, "Pause") == 0 || strcmp(key, "Prev") == 0 ||
        strcmp(key, "Next") == 0 || strcmp(key, "Vol+") == 0 ||
        strcmp(key, "Vol-") == 0 || strcmp(key, "Bri+") == 0 ||
        strcmp(key, "Bri-") == 0 || strcmp(key, "Mute") == 0) {
        return 1;
    }
    if (key[0] == 'F' && strlen(key) >= 2 && strlen(key) <= 3) {
        int fnum = atoi(key + 1);
        if (fnum >= 1 && fnum <= 12)
            return 1;
    }
    return 0;
}

// Split a segment text into base part and "×N" suffix (if present).
// The × character is UTF-8: 0xC3 0x97.
// Returns pointer to the "×N" part within seg, or NULL if not found.
static const char *find_count_suffix(const char *seg) {
    // Search for × (0xC3 0x97)
    for (const char *p = seg; *p; p++) {
        if ((unsigned char)p[0] == 0xC3 && (unsigned char)p[1] == 0x97) {
            return p; // points to "×N"
        }
    }
    return NULL;
}

void redraw(struct client_state *state) {
    if (!state->surface || !state->window_visible)
        return;

    const int stride = state->width * 4;
    const size_t size = stride * state->height;

    int fd = allocate_shm_file(size);
    if (fd == -1)
        return;

    void *data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        close(fd);
        return;
    }

    struct wl_shm_pool *pool = wl_shm_create_pool(state->shm, fd, size);
    struct wl_buffer *buffer = wl_shm_pool_create_buffer(
        pool, 0, state->width, state->height, stride, WL_SHM_FORMAT_ARGB8888);
    wl_shm_pool_destroy(pool);
    close(fd);

    cairo_surface_t *cs = cairo_image_surface_create_for_data(
        data, CAIRO_FORMAT_ARGB32, state->width, state->height, stride);
    cairo_t *cr = cairo_create(cs);

    // Clear
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.0);
    cairo_paint(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

    // Background
    const double r = 0;
    cairo_new_sub_path(cr);
    cairo_arc(cr, state->width - r, r, r, -M_PI / 2, 0);
    cairo_arc(cr, state->width - r, state->height - r, r, 0, M_PI / 2);
    cairo_arc(cr, r, state->height - r, r, M_PI / 2, M_PI);
    cairo_arc(cr, r, r, r, M_PI, 3 * M_PI / 2);
    cairo_close_path(cr);
    cairo_set_source_rgba(cr, state->bg_color[0], state->bg_color[1],
                          state->bg_color[2], state->bg_color[3]);
    cairo_fill(cr);

    // Font setup
    cairo_select_font_face(cr, "Monospace", CAIRO_FONT_SLANT_NORMAL,
                           CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, state->font_size);
    cairo_set_source_rgba(cr, state->text_color[0], state->text_color[1],
                          state->text_color[2], state->text_color[3]);

    cairo_font_extents_t font_extents;
    cairo_font_extents(cr, &font_extents);

    const double icon_size = state->font_size;
    const double small_font_size = state->font_size * 0.60; // size for ×N
    const double max_width = state->width - PADDING - RIGHT_PADDING;
    const double y_pos = (state->height - font_extents.height) / 2.0 +
                         font_extents.ascent + TOP_BOTTOM_PADDING - 7.0;

    int start_seg = 0;
    double seg_widths[MAX_SEGMENTS];
    int seg_is_icon[MAX_SEGMENTS];
    char seg_keys[MAX_SEGMENTS][32];
    char seg_mods[MAX_SEGMENTS][64];

    // First pass: measure all segments
    int current_char_idx = 0;
    for (int i = 0; i < state->seg_count; i++) {
        int len = state->seg_lengths[i];
        char snippet[128];
        snprintf(snippet, sizeof(snippet), "%.*s", len,
                 state->display_buf + current_char_idx);
        current_char_idx += len;

        char mods[64], key[32];
        parse_segment(snippet, mods, key);

        // Check if key has ×N suffix (e.g. "j×5")
        const char *count_suffix = find_count_suffix(key);
        char base_key[32] = {0};
        char count_part[16] = {0};
        if (count_suffix) {
            int base_len = count_suffix - key;
            strncpy(base_key, key, base_len);
            base_key[base_len] = '\0';
            strcpy(count_part, count_suffix); // "×5"
        } else {
            strcpy(base_key, key);
        }

        cairo_text_extents_t mod_extents;
        cairo_text_extents(cr, mods, &mod_extents);
        double w = mod_extents.x_advance;

        if (is_icon_key(base_key)) {
            seg_is_icon[i] = 1;
            strcpy(seg_keys[i], base_key);
            w += icon_size;
        } else {
            seg_is_icon[i] = 0;
            // Measure base key at normal font size
            cairo_set_font_size(cr, state->font_size);
            cairo_text_extents_t key_extents;
            cairo_text_extents(cr, base_key, &key_extents);
            w += key_extents.x_advance;
            snprintf(seg_mods[i], sizeof(seg_mods[i]), "%s%s", mods, base_key);
        }

        // Add width of ×N part at small font size
        if (count_suffix && strlen(count_part) > 0) {
            cairo_set_font_size(cr, small_font_size);
            cairo_text_extents_t count_extents;
            cairo_text_extents(cr, count_part, &count_extents);
            w += count_extents.x_advance;
            cairo_set_font_size(cr, state->font_size); // restore
        }

        if (seg_is_icon[i]) {
            strcpy(seg_mods[i], mods);
        }

        seg_widths[i] = w;
    }

    // Calculate fit from end
    double width_so_far = 0;
    for (int i = state->seg_count - 1; i >= 0; i--) {
        if (width_so_far + seg_widths[i] > max_width) {
            start_seg = i + 1;
            break;
        }
        width_so_far += seg_widths[i];
    }

    // Draw phase
    double current_x = state->width - RIGHT_PADDING - width_so_far;
    if (current_x < PADDING)
        current_x = PADDING;

    const double *draw_color = state->text_color;

    // Reset current_char_idx for draw pass
    current_char_idx = 0;
    for (int i = 0; i < start_seg; i++)
        current_char_idx += state->seg_lengths[i];

    for (int i = start_seg; i < state->seg_count; i++) {
        // Reconstruct snippet to re-parse for count suffix in draw pass
        int len = state->seg_lengths[i];
        char snippet[128];
        snprintf(snippet, sizeof(snippet), "%.*s", len,
                 state->display_buf + current_char_idx);
        current_char_idx += len;

        char mods[64], key[32];
        parse_segment(snippet, mods, key);

        const char *count_suffix = find_count_suffix(key);
        char base_key[32] = {0};
        char count_part[16] = {0};
        if (count_suffix) {
            int base_len = count_suffix - key;
            strncpy(base_key, key, base_len);
            base_key[base_len] = '\0';
            strcpy(count_part, count_suffix);
        } else {
            strcpy(base_key, key);
        }

        // Apply combo color to last segment
        if (i == state->seg_count - 1 && state->use_combo_color) {
            draw_color = state->current_combo_color;
            cairo_set_source_rgba(cr, draw_color[0], draw_color[1],
                                  draw_color[2], draw_color[3]);
        } else {
            draw_color = state->text_color;
            cairo_set_source_rgba(cr, draw_color[0], draw_color[1],
                                  draw_color[2], draw_color[3]);
        }

        // Draw mods text (e.g. "Ctrl+")
        cairo_set_font_size(cr, state->font_size);
        cairo_move_to(cr, current_x, y_pos);
        cairo_show_text(cr, mods);
        cairo_text_extents_t ext;
        cairo_text_extents(cr, mods, &ext);
        current_x += ext.x_advance;

        // Draw icon or base key text
        if (seg_is_icon[i]) {
            if (i == state->seg_count - 1 && state->use_combo_color) {
                draw_icon(cr, base_key, current_x, y_pos, icon_size,
                          state->current_combo_color);
            } else {
                draw_icon(cr, base_key, current_x, y_pos, icon_size,
                          state->text_color);
            }
            current_x += icon_size;
        } else {
            // Draw base key text at normal size
            cairo_set_font_size(cr, state->font_size);
            cairo_move_to(cr, current_x, y_pos);
            cairo_show_text(cr, base_key);
            cairo_text_extents_t key_ext;
            cairo_text_extents(cr, base_key, &key_ext);
            current_x += key_ext.x_advance;
        }

        // Draw ×N at smaller font size, shifted up slightly (superscript-like)
        if (count_suffix && strlen(count_part) > 0) {
            cairo_set_font_size(cr, small_font_size);
            double superscript_offset = state->font_size * 0.18;
            cairo_move_to(cr, current_x, y_pos - superscript_offset);
            cairo_show_text(cr, count_part);
            cairo_text_extents_t count_ext;
            cairo_text_extents(cr, count_part, &count_ext);
            current_x += count_ext.x_advance;
            cairo_set_font_size(cr, state->font_size); // restore
        }
    }

    // Draw mouse click display
    if (state->mouse.lmb || state->mouse.rmb || state->mouse.mmb) {
        char mouse_info[128];
        char buttons[32] = "";
        if (state->mouse.lmb)
            strcat(buttons, "LMB ");
        if (state->mouse.rmb)
            strcat(buttons, "RMB ");
        if (state->mouse.mmb)
            strcat(buttons, "MMB ");
        snprintf(mouse_info, sizeof(mouse_info), "%s (%d, %d)", buttons,
                 state->mouse.x, state->mouse.y);

        cairo_set_source_rgba(cr, state->text_color[0], state->text_color[1],
                              state->text_color[2], state->text_color[3]);
        cairo_select_font_face(cr, "Monospace", CAIRO_FONT_SLANT_NORMAL,
                               CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(cr, state->font_size * 0.5);

        cairo_text_extents_t mouse_ext;
        cairo_text_extents(cr, mouse_info, &mouse_ext);
        double mouse_x = (state->width - mouse_ext.width) / 2.0;
        double mouse_y = state->height - 10;

        cairo_move_to(cr, mouse_x, mouse_y);
        cairo_show_text(cr, mouse_info);
    }

    cairo_destroy(cr);
    cairo_surface_destroy(cs);
    munmap(data, size);

    wl_surface_attach(state->surface, buffer, 0, 0);
    wl_surface_damage_buffer(state->surface, 0, 0, state->width, state->height);
    wl_surface_commit(state->surface);

    if (state->buffer)
        wl_buffer_destroy(state->buffer);
    state->buffer = buffer;
}
