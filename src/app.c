#include <math.h>
#include <stdio.h>

#include <par/par_shaders.h>

#include <sokol/sokol_gfx.h>

#include "app.h"

typedef struct {
    float x, y, z, w;
} vec4;

typedef struct {
    float x, y;
} vec2;

static const vec4 positions[] = {
    {{-1, -1, 0, 1}},  // 0
    {{+1, -1, 0, 1}},  // 1
    {{+1, +1, 0, 1}},  // 2
    {{-1, +1, 0, 1}},  // 3
};

static const vec2 texcoords[] = {
    {{0, 01}},  // 0
    {{1, 01}},  // 1
    {{1, 11}},  // 2
    {{0, 11}},  // 3
};

void app_init(App* app) {
    sg_setup(&(sg_desc){
        .mtl_device = sapp_metal_get_device(),
        .mtl_renderpass_descriptor_cb = sapp_metal_get_renderpass_descriptor,
        .mtl_drawable_cb = sapp_metal_get_drawable,
    });

    app.gui = gui_create(&app, kSidebarWidth);
    parsh_context* shaders = parsh_create_context_from_file("src/demo.glsl");

    sg_buffer fullscreen_buffer = sg_make_buffer(&(sg_buffer_desc){
        .size = sizeof(fullscreen_verts),
        .usage = SG_USAGE_IMMUTABLE,
        .content = fullscreen_verts,
    });

    fullscreen_bindings = (sg_bindings){
        .vertex_buffers[0] = fullscreen_buffer,
        .fs_images[0] = fullscreen_texture,
    };

    sg_shader fullscreen_program = sg_make_shader(&(sg_shader_desc){
        .fs.uniform_blocks[0].size = sizeof(tvec4),
        .fs.uniform_blocks[0].uniforms[0].name = "resolution",
        .fs.uniform_blocks[0].uniforms[0].type = SG_UNIFORMTYPE_FLOAT4,
        .fs.images[0].name = "tex",
        .fs.images[0].type = SG_IMAGETYPE_2D,
        .vs.source = parsh_get_blocks(shaders, "prefix fullscreen.vs"),
        .fs.source = parsh_get_blocks(shaders, "prefix fullscreen.fs"),
    });

    fullscreen_pipeline = sg_make_pipeline(&(sg_pipeline_desc){
        .shader = fullscreen_program,
        .blend.enabled = false,
        .depth_stencil.depth_write_enabled = false,
        .rasterizer.cull_mode = SG_CULLMODE_NONE,
        .layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT4,
    });

    fullscreen_transition = (sg_pass_action){
        .colors[0].action = SG_ACTION_CLEAR,
        .colors[0].val = {0.0f, 0.0f, 0.0f, 0.0f},
    };
}

void app_draw(App* app, double seconds) {
    app_update_projection(&app);

    if (app.transition.van_wijk) {
        const CameraTransition trans = app.transition;
        const double duration = parcc_get_interpolation_duration(trans.source, trans.target);
        const double kTransitionSpeed = 3;
        double elapsed = kTransitionSpeed * (seconds - app.transition.start_time) / duration;
        if (elapsed >= 1.0) {
            elapsed = 1.0;
            app.transition.van_wijk = false;
        }
        const parcc_frame frame = parcc_interpolate_frames(trans.source, trans.target, elapsed);
        parcc_set_frame(app.camera_controller, frame);
    }

    float view[16];
    camera_get_view_matrixf(app.camera, view);

    float model[16];
    float16_identity(model);

    float16_multiply(app.gfx.uniforms.modelview, model, view);
    float16_copy(app.gfx.uniforms.inverse_mv, app.gfx.uniforms.modelview);
    float16_invert(app.gfx.uniforms.inverse_mv);

    const sg_pass_action pass_action = {
        .colors[0].action = SG_ACTION_CLEAR,
        .colors[0].val = {0.1f, 0.2f, 0.3f, 1.0f},
    };

    const float vp_width = sapp_width() - kSidebarWidth;
    const float vp_height = sapp_height();

    sg_begin_default_pass(&pass_action, sapp_width(), sapp_height());
    sg_apply_viewport(kSidebarWidth, 0, vp_width, vp_height, false);
    sg_apply_pipeline(app.gfx.pipeline);
    sg_apply_bindings(&app.gfx.bindings);
    sg_draw(0, app.gfx.num_elements, 1);

    sg_apply_viewport(0, 0, sapp_width(), vp_height, false);
    gui_draw(app.gui);

    sg_end_pass();
    sg_commit();
}

void app_start_camera_transition(App* app, int tile_index) {
    if (app->transition.van_wijk) {
        return;
    }
    app->transition.start_time = stm_sec(stm_now());
    app->transition.source = parcc_capture_state(app->camera_controller);
    app->transition.target = app->home_position;
    app->transition.van_wijk = true;
}

void app_update_projection(App* app) {
    const float vpwidth = sapp_width() - kSidebarWidth;
    const float vpheight = sapp_height();
    camera_set_aspect(app->camera, vpwidth / vpheight);
    camera_perspective(app->camera, CAMERA_FOV_VERTICAL, kFov, kNearPlane, kFarPlane);
    camera_get_projection_matrixf(app->camera, app->uniforms.projection);
}
