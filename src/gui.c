#define SOKOL_GLCORE33
#define SOKOL_GL_IMPL

#include "gui.h"
#include "app.h"

#include <microui/microui.h>

#include <sokol/sokol_gfx.h>
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

    mu_begin_window_ex(ctx, &gui->window, "", MU_OPT_NOTITLE | MU_OPT_NORESIZE);

    const parcc_config config = parcc_get_config(app->camera_controller);
    parcc_config new_config = config;

    mu_layout_row(ctx, 2, (int[]){-1, -1}, 0);
    int orbit = config.mode == PARCC_ORBIT;
    int map = config.mode == PARCC_MAP;
    mu_checkbox(ctx, &orbit, "Orbit mode");
    mu_checkbox(ctx, &map, "Map mode");
    new_config.mode = orbit ? PARCC_ORBIT : PARCC_MAP;

    int show_fov = 1;
    mu_header(ctx, &show_fov, "Field of view");
    int vert = config.fov_direction == PARCC_VERTICAL;
    int horiz = config.fov_direction == PARCC_HORIZONTAL;
    mu_checkbox(ctx, &vert, "Vertical");
    mu_checkbox(ctx, &horiz, "Horizontal");
    new_config.fov_direction = vert ? PARCC_VERTICAL : PARCC_HORIZONTAL;

    mu_label(ctx, "Degrees");
    float degrees = new_config.fov_radians * 180.0f / M_PI;
    mu_slider(ctx, &degrees, 10, 90);
    new_config.fov_radians = degrees * M_PI / 180.0f;

    mu_layout_row(ctx, 1, (int[]){-1}, 0);

    int raycast = 0;
    mu_checkbox(ctx, &raycast, "Raycast");

    mu_button(ctx, "Jump to Home Frame");

    mu_layout_row(ctx, 3, (int[]){-1, -1, -1}, 0);
    mu_button(ctx, "Save as Frame A");
    mu_button(ctx, "Jump to Frame A");
    mu_button(ctx, "Highlight Frame A");

    mu_button(ctx, "Save as Frame B");
    mu_button(ctx, "Jump to Frame B");
    mu_button(ctx, "Highlight Frame B");

    mu_end_window(ctx);

    // if (config != new_config) {
    //     parcc_set_config(app->camera_controller, new_config);
    // }

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
            if (key == 'g') {
                app_start_camera_transition(gui->app);
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
