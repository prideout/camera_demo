#define SOKOL_GLCORE33
#define SOKOL_GL_IMPL

#include "gui.h"
#include "app.h"

#include <microui/microui.h>
#include <sokol/util/sokol_gl.h>

#include <stdio.h>

struct GuiImpl {
    mu_Context ctx;
    mu_Container window;
    mu_Container load_dialog;
    App* app;
    sg_image atlas_img;
    sgl_pipeline pip;
    int sidebar_width;
    bool open_load_dialog;
};

const mu_Color kTooltipColor = {96, 128, 255, 255};
const mu_Color kActiveColor = {230, 230, 230, 255};
const mu_Color kDisabledColor = {255, 255, 255, 100};

static void define_ui(Gui* gui) {
    mu_Context* ctx = &gui->ctx;
    App* app = gui->app;
    mu_begin(ctx);

    if (!gui->window.inited) {
        mu_init_window(ctx, &gui->window, 0);
    }

    gui->window.rect.x = gui->window.rect.y = 0;
    gui->window.rect.w = gui->sidebar_width;
    gui->window.rect.h = sapp_height();

    const int htile = app->hover_tile;
    const int stile = app->selected_tile;
    const size_t ncols = app->tile_db.num_columns;
    const size_t nrows = app->tile_db.num_rows;
    const int row = htile / ncols;
    const int col = htile % ncols;
    const size_t ntiles = app->tile_db.num_active_tiles;
    char buf[64];
    char tooltip[64] = {0};

    mu_begin_window_ex(ctx, &gui->window, "", MU_OPT_NOTITLE | MU_OPT_NORESIZE);
    mu_layout_row(ctx, 3, (int[]){93, 93, 93}, 0);

    const char* kNewGrid = "New";
    if (mu_button(ctx, kNewGrid) || gui->open_load_dialog) {
        gui->open_load_dialog = true;
    }
    if (ctx->hover == ctx->last_id) {
        sprintf(tooltip, "shortcut: N");
    }

    const char* kLoadGrid = "Load";
    mu_button(ctx, kLoadGrid);
    if (ctx->hover == ctx->last_id) {
        sprintf(tooltip, "shortcut: L");
    }

    const char* kSaveGrid = "Save";
    mu_button(ctx, kSaveGrid);
    if (ctx->hover == ctx->last_id) {
        sprintf(tooltip, "shortcut: S");
    }

    if (app->hover_tile == -1) {
        mu_label(ctx, "--");
    } else {
        sprintf(buf, "%c%d   (%d, %d)", 'A' + col, row + 1, col, row);
        mu_label(ctx, buf);
    }
    sprintf(buf, "%zu x %zu", ncols, nrows);
    mu_label(ctx, buf);
    sprintf(buf, "%zu active tiles.", ntiles);
    mu_label(ctx, buf);

    mu_layout_row(ctx, 2, (int[]){50, 50}, 0);

    // 2D / 3D radio buttons.
    ctx->style->colors[MU_COLOR_TEXT] = kActiveColor;
    ctx->style->spacing = 0;
    mu_button(ctx, "2D");
    if (ctx->hover == ctx->last_id) {
        sprintf(tooltip, "shortcut: 2");
    }
    ctx->style->colors[MU_COLOR_TEXT] = kDisabledColor;
    ctx->style->spacing = 4;
    mu_button(ctx, "3D");
    if (ctx->hover == ctx->last_id) {
        sprintf(tooltip, "shortcut: 3");
    }
    ctx->style->colors[MU_COLOR_TEXT] = kActiveColor;

    // Toggle / Select radio buttons.

    mu_layout_row(ctx, 2, (int[]){146, -1}, 0);
    int show = app->show_edges;
    mu_checkbox(ctx, &show, "Show edges");
    app->show_edges = show;
    if (ctx->hover == ctx->last_id) {
        sprintf(tooltip, "shortcut: C");
    }

    mu_layout_row(ctx, 2, (int[]){146, -1}, 0);
    if (mu_button(ctx, "Nav to Center")) {
        app_start_camera_transition(app, -1);
    }
    if (ctx->hover == ctx->last_id) {
        sprintf(tooltip, "shortcut: Spacebar");
    }

    if (stile != -1) {
        const int row = stile / ncols;
        const int col = stile % ncols;
        sprintf(buf, "Nav to %c%d", 'A' + col, row + 1);
        if (mu_button(ctx, buf)) {
            app_start_camera_transition(app, stile);
        }
        if (ctx->hover == ctx->last_id) {
            sprintf(tooltip, "shortcut: G");
        }
    } else {
        mu_label(ctx, "");
    }

    mu_layout_row(ctx, 1, (int[]){-1}, 0);
    ctx->style->colors[MU_COLOR_TEXT] = kTooltipColor;

    if (tooltip[0]) {
        mu_label(ctx, tooltip);
    }

    if (app->selected_segment != TileCenter) {
        const int srow = stile / ncols;
        const int scol = stile % ncols;
        if (app->hover_tile == -1 || !TILE_IS_ACTIVE(app->tile_db.states, app->hover_tile)) {
            sprintf(buf, "Connecting %c%d...", 'A' + scol, srow + 1);
        } else {
            const int hrow = app->hover_tile / ncols;
            const int hcol = app->hover_tile % ncols;
            sprintf(buf, "Connecting %c%d to %c%d...", 'A' + scol, srow + 1, 'A' + hcol, hrow + 1);
        }
        mu_label(ctx, buf);
    }

    ctx->style->colors[MU_COLOR_TEXT] = kActiveColor;

    mu_end_window(ctx);

    static char kNewGridSize[60];

    if (gui->open_load_dialog) {
        mu_open_popup(ctx, &gui->load_dialog);
        sprintf(kNewGridSize, "%zu x %zu", ncols, nrows);
    }

    if (mu_begin_popup(ctx, &gui->load_dialog)) {
        mu_layout_row(ctx, 2, (int[]){110, 70}, 0);
        mu_label(ctx, "Enter new grid size:");
        int submitted = 0;
        if (mu_textbox(ctx, kNewGridSize, sizeof(kNewGridSize)) & MU_RES_SUBMIT) {
            mu_set_focus(ctx, ctx->last_id);
            submitted = 1;
        }
        if (gui->open_load_dialog) {
            mu_set_focus(ctx, ctx->last_id);
            gui->open_load_dialog = false;
        }
        mu_layout_row(ctx, 2, (int[]){30, 50}, 0);
        if (mu_button(ctx, "Ok")) {
            submitted = 1;
        }
        char* divider = strchr(kNewGridSize, 'x');
        if (submitted && divider) {
            *divider = 0;
            int width = atoi(kNewGridSize) & 255;
            int height = atoi(divider + 1) & 255;
            if (width > 0 && height > 0) {
                app_new_grid(app, width, height);
                gui->load_dialog.open = 0;
            }
        }
        if (mu_button(ctx, "Cancel")) {
            gui->load_dialog.open = 0;
        }
        if (submitted && gui->load_dialog.open) {
            puts("Bad format in size string.");
        }
        mu_end_popup(ctx);
    }

    mu_end(ctx);
}

static int text_width_cb(mu_Font font, const char* text, int len);
static int text_height_cb(mu_Font font);

static void r_init(Gui* gui);
static void r_begin(Gui* gui, int disp_width, int disp_height);
static void r_end(void);
static void r_draw(void);
static void r_push_quad(mu_Rect dst, mu_Rect src, mu_Color color);
static void r_draw_rect(mu_Rect rect, mu_Color color);
static void r_draw_text(const char* text, mu_Vec2 pos, mu_Color color);
static void r_draw_icon(int id, mu_Rect rect, mu_Color color);
static int r_get_text_width(const char* text, int len);
static int r_get_text_height(void);
static void r_set_clip_rect(mu_Rect rect);

static void define_ui(Gui* gui);
static void render_ui(Gui* gui);

Gui* gui_create(App* app, int sidebar_width) {
    Gui* retval = malloc(sizeof(struct GuiImpl));
    memset(retval, 0, sizeof(struct GuiImpl));
    retval->app = app;
    retval->sidebar_width = sidebar_width;
    sgl_setup(&(sgl_desc_t){0});
    r_init(retval);
    mu_init(&retval->ctx);
    retval->ctx.text_width = text_width_cb;
    retval->ctx.text_height = text_height_cb;
    return retval;
}

void gui_destroy(Gui* gui) {
    sgl_shutdown();
    free(gui);
}

bool gui_handle(Gui* gui, const sapp_event* ev) {
    static const char key_map[512] = {
        [SAPP_KEYCODE_LEFT_SHIFT] = MU_KEY_SHIFT,  [SAPP_KEYCODE_RIGHT_SHIFT] = MU_KEY_SHIFT,
        [SAPP_KEYCODE_LEFT_CONTROL] = MU_KEY_CTRL, [SAPP_KEYCODE_RIGHT_CONTROL] = MU_KEY_CTRL,
        [SAPP_KEYCODE_LEFT_ALT] = MU_KEY_ALT,      [SAPP_KEYCODE_RIGHT_ALT] = MU_KEY_ALT,
        [SAPP_KEYCODE_ENTER] = MU_KEY_RETURN,      [SAPP_KEYCODE_BACKSPACE] = MU_KEY_BACKSPACE,
    };
    mu_Context* ctx = &gui->ctx;
    switch (ev->type) {
        case SAPP_EVENTTYPE_MOUSE_DOWN:
            mu_input_mousedown(ctx, (int)ev->mouse_x, (int)ev->mouse_y, (1 << ev->mouse_button));
            break;
        case SAPP_EVENTTYPE_MOUSE_UP:
            mu_input_mouseup(ctx, (int)ev->mouse_x, (int)ev->mouse_y, (1 << ev->mouse_button));
            break;
        case SAPP_EVENTTYPE_MOUSE_MOVE:
            mu_input_mousemove(ctx, (int)ev->mouse_x, (int)ev->mouse_y);
            break;
        case SAPP_EVENTTYPE_MOUSE_SCROLL:
            mu_input_scroll(ctx, (int)ev->scroll_x, (int)ev->scroll_y);
            break;
        case SAPP_EVENTTYPE_KEY_DOWN:
            mu_input_keydown(ctx, key_map[ev->key_code & 511]);
            break;
        case SAPP_EVENTTYPE_KEY_UP:
            mu_input_keyup(ctx, key_map[ev->key_code & 511]);
            break;
        case SAPP_EVENTTYPE_CHAR: {
            char key = ev->char_code & 255;
            if (gui->load_dialog.open) {
                if (key != 127) {
                    char txt[2] = {key, 0};
                    mu_input_text(ctx, txt);
                }
                if (key == 27) {
                    gui->load_dialog.open = 0;
                }
                break;
            }
            if (key == 27) {
                sapp_request_quit();
            }
            if (key == 'c') {
                gui->app->show_edges = !gui->app->show_edges;
                gui->app->edge_gfx.dirty = true;
            }
            if (key == 'n') {
                gui->open_load_dialog = true;
            }
            if (key == 'g' && gui->app->selected_tile != -1) {
                app_start_camera_transition(gui->app, gui->app->selected_tile);
            }
            if (key == ' ') {
                app_start_camera_transition(gui->app, -1);
            }
            break;
        }
        default:
            break;
    }
    switch (ev->type) {
        case SAPP_EVENTTYPE_MOUSE_DOWN:
        case SAPP_EVENTTYPE_MOUSE_UP:
        case SAPP_EVENTTYPE_MOUSE_MOVE:
        case SAPP_EVENTTYPE_MOUSE_SCROLL:
            return gui->ctx.hover_root != NULL;
        default:
            break;
    }
    return false;
}

void gui_draw(Gui* gui) {
    render_ui(gui);
    define_ui(gui);
}

static void render_ui(Gui* gui) {
    r_begin(gui, sapp_width(), sapp_height());
    mu_Command* cmd = 0;
    while (mu_next_command(&gui->ctx, &cmd)) {
        switch (cmd->type) {
            case MU_COMMAND_TEXT:
                r_draw_text(cmd->text.str, cmd->text.pos, cmd->text.color);
                break;
            case MU_COMMAND_RECT:
                r_draw_rect(cmd->rect.rect, cmd->rect.color);
                break;
            case MU_COMMAND_ICON:
                r_draw_icon(cmd->icon.id, cmd->icon.rect, cmd->icon.color);
                break;
            case MU_COMMAND_CLIP:
                r_set_clip_rect(cmd->clip.rect);
                break;
        }
    }
    r_end();
    r_draw();
}

static int text_width_cb(mu_Font font, const char* text, int len) {
    if (len == -1) {
        len = (int)strlen(text);
    }
    return r_get_text_width(text, len);
}

static int text_height_cb(mu_Font font) { return r_get_text_height(); }

static void r_init(Gui* gui) {
    /* atlas image data is in atlas.inl file, this only contains alpha
       values, need to expand this to RGBA8
    */
    uint32_t rgba8_size = ATLAS_WIDTH * ATLAS_HEIGHT * 4;
    uint32_t* rgba8_pixels = (uint32_t*)malloc(rgba8_size);
    for (int y = 0; y < ATLAS_HEIGHT; y++) {
        for (int x = 0; x < ATLAS_WIDTH; x++) {
            uint32_t index = y * ATLAS_WIDTH + x;
            rgba8_pixels[index] = 0x00FFFFFF | ((uint32_t)atlas_texture[index] << 24);
        }
    }
    gui->atlas_img = sg_make_image(
        &(sg_image_desc){.width = ATLAS_WIDTH,
                         .height = ATLAS_HEIGHT,
                         /* LINEAR would be better for text quality in HighDPI, but the
                            atlas texture is "leaking" from neighbouring pixels unfortunately
                         */
                         .min_filter = SG_FILTER_NEAREST,
                         .mag_filter = SG_FILTER_NEAREST,
                         .content = {.subimage[0][0] = {.ptr = rgba8_pixels, .size = rgba8_size}}});
    gui->pip = sgl_make_pipeline(
        &(sg_pipeline_desc){.blend = {.enabled = true,
                                      .src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
                                      .dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA}});

    free(rgba8_pixels);
}

static void r_begin(Gui* gui, int disp_width, int disp_height) {
    sgl_defaults();
    sgl_push_pipeline();
    sgl_load_pipeline(gui->pip);
    sgl_enable_texture();
    sgl_texture(gui->atlas_img);
    sgl_matrix_mode_projection();
    sgl_push_matrix();
    sgl_ortho(0.0f, (float)disp_width, (float)disp_height, 0.0f, -1.0f, +1.0f);
    sgl_begin_quads();
}

static void r_end(void) {
    sgl_end();
    sgl_pop_matrix();
    sgl_pop_pipeline();
}

static void r_draw(void) { sgl_draw(); }

static void r_push_quad(mu_Rect dst, mu_Rect src, mu_Color color) {
    float u0 = (float)src.x / (float)ATLAS_WIDTH;
    float v0 = (float)src.y / (float)ATLAS_HEIGHT;
    float u1 = (float)(src.x + src.w) / (float)ATLAS_WIDTH;
    float v1 = (float)(src.y + src.h) / (float)ATLAS_HEIGHT;

    float x0 = (float)dst.x;
    float y0 = (float)dst.y;
    float x1 = (float)(dst.x + dst.w);
    float y1 = (float)(dst.y + dst.h);

    sgl_c4b(color.r, color.g, color.b, color.a);
    sgl_v2f_t2f(x0, y0, u0, v0);
    sgl_v2f_t2f(x1, y0, u1, v0);
    sgl_v2f_t2f(x1, y1, u1, v1);
    sgl_v2f_t2f(x0, y1, u0, v1);
}

static void r_draw_rect(mu_Rect rect, mu_Color color) {
    r_push_quad(rect, atlas[ATLAS_WHITE], color);
}

static void r_draw_text(const char* text, mu_Vec2 pos, mu_Color color) {
    mu_Rect dst = {pos.x, pos.y, 0, 0};
    for (const char* p = text; *p; p++) {
        mu_Rect src = atlas[ATLAS_FONT + (unsigned char)*p];
        dst.w = src.w;
        dst.h = src.h;
        r_push_quad(dst, src, color);
        dst.x += dst.w;
    }
}

static void r_draw_icon(int id, mu_Rect rect, mu_Color color) {
    mu_Rect src = atlas[id];
    int x = rect.x + (rect.w - src.w) / 2;
    int y = rect.y + (rect.h - src.h) / 2;
    r_push_quad(mu_rect(x, y, src.w, src.h), src, color);
}

static int r_get_text_width(const char* text, int len) {
    int res = 0;
    for (const char* p = text; *p && len--; p++) {
        res += atlas[ATLAS_FONT + (unsigned char)*p].w;
    }
    return res;
}

static int r_get_text_height(void) { return 18; }

static void r_set_clip_rect(mu_Rect rect) {
    sgl_end();
    sgl_scissor_rect(rect.x, rect.y, rect.w, rect.h, true);
    sgl_begin_quads();
}
