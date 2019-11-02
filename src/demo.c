#include <sokol/sokol_app.h>
#include <sokol/sokol_time.h>

#include "app.h"
#include "gui.h"
#include "vec_float.h"

static App app = {0};

static void init() {
    stm_setup();
    app_init(&app);
}

static void draw() { app_draw(&app, stm_sec(stm_now())); }

static void handler(const sapp_event* event) {
    static float mouse_down_pos[2] = {0};
    if (gui_handle(app.gui, event)) {
        return;
    }
    const float vpwidth = sapp_width() - kSidebarWidth;
    const float vpheight = sapp_height();
    const float winx = (event->mouse_x - kSidebarWidth) / vpwidth;
    const float winy = 1.0 - event->mouse_y / vpheight;
    switch (event->type) {
        case SAPP_EVENTTYPE_MOUSE_DOWN: {
            mouse_down_pos[0] = winx;
            mouse_down_pos[1] = winy;
            parcc_grab_begin(app.camera_controller, winx, winy);
            break;
        }
        case SAPP_EVENTTYPE_MOUSE_UP:
            parcc_grab_end(app.camera_controller);
            if (winx == mouse_down_pos[0] && winy == mouse_down_pos[1]) {
                // click
            }
            break;
        case SAPP_EVENTTYPE_MOUSE_SCROLL:
            parcc_grab_update(app.camera_controller, winx, winy, event->scroll_y);
            break;
        case SAPP_EVENTTYPE_MOUSE_MOVE:
            parcc_grab_update(app.camera_controller, winx, winy, 0);
            break;
        case SAPP_EVENTTYPE_MOUSE_ENTER:
        case SAPP_EVENTTYPE_MOUSE_LEAVE:
        case SAPP_EVENTTYPE_TOUCHES_BEGAN:
        case SAPP_EVENTTYPE_TOUCHES_MOVED:
        case SAPP_EVENTTYPE_TOUCHES_ENDED:
        case SAPP_EVENTTYPE_TOUCHES_CANCELLED:
            break;
        default:
            break;
    }
}

static void cleanup() { sg_shutdown(); }

sapp_desc sokol_main(int argc, char* argv[]) {
    return (sapp_desc){
        .init_cb = init,
        .frame_cb = draw,
        .event_cb = handler,
        .cleanup_cb = cleanup,
        .width = 1280,
        .height = 720,
        .sample_count = 16,
        .window_title = "camera demo",
        .high_dpi = false,
    };
}
