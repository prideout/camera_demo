#include <assert.h>
#include <math.h>
#include <stdio.h>

#include <par/par_camera_control.h>
#include <par/par_msquares.h>
#include <par/par_shaders.h>

#include <stb/stb_image.h>
#include <stb/stb_image_resize.h>

#include <sokol/sokol_gfx.h>
#include <sokol/sokol_time.h>

#include "app.h"
#include "vec_float.h"

#define IMAX(a, b) (a > b ? a : b)

typedef struct {
    float x, y, z, w;
} vec4;

typedef struct {
    float x, y;
} vec2;

static vec4 positions[] = {
    {0, 0, 0, 1},  // 0
    {1, 0, 0, 1},  // 1
    {1, 1, 0, 1},  // 2
    {0, 1, 0, 1},  // 3
};

static const vec2 texcoords[] = {
    {0, 1},  // 0
    {1, 1},  // 1
    {1, 0},  // 2
    {0, 0},  // 3
};

static const uint16_t indices[] = {
    0, 1, 2,  //
    2, 3, 0,  //
};

void app_update_projection(App* app) {
    const float vpwidth = sapp_width() - kSidebarWidth;
    const float vpheight = sapp_height();
    camera_set_aspect(app->camera, vpwidth / vpheight);
    camera_perspective(app->camera, CAMERA_FOV_HORIZONTAL, kFov, kNearPlane, kFarPlane);
    camera_get_projection_matrixf(app->camera, app->gfx.uniforms.projection);
}

static void create_mesh(App* app, const char* filename) {
    int nchan;
    int width, height;
    stbi_uc* u8_data = stbi_load(filename, &width, &height, &nchan, 1);
    assert(u8_data);
    float* float_data = malloc(sizeof(float) * width * height);
    for (int i = 0; i < width * height; i++) {
        float_data[i] = (float)u8_data[i] / 255.0f;
    }
    stbi_image_free(u8_data);

    const int cellsize = 10;

    const float thresholds[5] = {
        .15f, .30f, .45f, .60f, .75f,
    };

    par_msquares_meshlist* meshes =
        par_msquares_grayscale_multi(float_data, width, height, cellsize, thresholds, 5,
                                     PAR_MSQUARES_SIMPLIFY | PAR_MSQUARES_HEIGHTS);

    assert(meshes);

    int num_meshes = par_msquares_get_count(meshes);
    printf("%d meshes\n", num_meshes);

    par_msquares_mesh const* mesh = par_msquares_get_mesh(meshes, 0);
    printf("mesh 0 : %d verts, %d triangles (dim = %d)\n", mesh->npoints, mesh->ntriangles,
           mesh->dim);

    par_msquares_free(meshes);
    free(float_data);
}

static void create_texture(App* app, const char* filename, int* width, int* height) {
    int nchan;
    stbi_uc* mip0 = stbi_load(filename, width, height, &nchan, 4);
    int mip0size = 4 * *width * *height;

    sg_image_desc image_desc = {
        .width = *width,
        .height = *height,
        .min_filter = SG_FILTER_LINEAR_MIPMAP_LINEAR,
        .mag_filter = SG_FILTER_LINEAR,
        .content.subimage[0][0].ptr = mip0,
        .content.subimage[0][0].size = mip0size,
    };

    stbi_uc* pmip = mip0;
    int pwidth = *width;
    int pheight = *height;
    int level;
    for (level = 1; level < SG_MAX_MIPMAPS; level++) {
        int hwidth = IMAX(pwidth / 2, 1);
        int hheight = IMAX(pheight / 2, 1);
        stbi_uc* hmip = malloc(hwidth * hheight * 4);
        if (!stbir_resize_uint8(pmip, pwidth, pheight, 0, hmip, hwidth, hheight, 0, 4)) {
            puts("Error with mipmap generation.\n");
        }
        image_desc.content.subimage[0][level].ptr = hmip;
        image_desc.content.subimage[0][level].size = hwidth * hheight * 4;
        if (hwidth == 1 && hheight == 1) {
            break;
        }
        pwidth = hwidth;
        pheight = hheight;
        pmip = hmip;
    }

    image_desc.num_mipmaps = level + 1;

    app->gfx.texture = sg_make_image(&image_desc);

    stbi_image_free(mip0);
    for (level = 1; level < SG_MAX_MIPMAPS; level++) {
        free((void*)image_desc.content.subimage[0][level].ptr);
    }
}

void app_init(App* app) {
    stm_setup();

    sg_setup(&(sg_desc){
        .mtl_device = sapp_metal_get_device(),
        .mtl_renderpass_descriptor_cb = sapp_metal_get_renderpass_descriptor,
        .mtl_drawable_cb = sapp_metal_get_drawable,
    });

    const uint64_t start_decode = stm_now();
    int width, height;
    create_texture(app, "extras/terrain/terrain.png", &width, &height);
    printf("Loaded %dx%d texture in %.0f ms\n", width, height,
           stm_ms(stm_diff(stm_now(), start_decode)));

    const uint64_t start_mesh = stm_now();
    create_mesh(app, "extras/terrain/elevation.png");
    printf("Created mesh in %.0f ms\n", stm_ms(stm_diff(stm_now(), start_mesh)));

    positions[1].x = positions[2].x = width;
    positions[2].y = positions[3].y = height;

    app->camera = camera_create();

    app_update_projection(app);

    const float vpwidth = sapp_width() - kSidebarWidth;
    const float vpheight = sapp_height();
    const float fovy = camera_get_fovy_radians(app->camera);
    const float fovx = fovy * vpwidth / vpheight;
    const float kInitialDistance = width / tanf(fovx);
    const float eyepos[] = {width / 2.0f, height / 2.0f, kInitialDistance};
    const float target[] = {width / 2.0f, height / 2.0f, 0};
    const float upward[] = {0, 1, 0};
    camera_look_atf(app->camera, eyepos, target, upward);
    printf("Distance is %f\n", kInitialDistance);

    app->camera_controller = parcc_create_context((parcc_config){
        .mode = PARCC_ORBIT,
        .viewport_width = 100,
        .viewport_height = 100,
        .fov_direction = PARCC_HORIZONTAL,
        .fov_radians = M_PI / 3,
        .content_aabb.min_corner = {0, 0, 0},
        .content_aabb.max_corner = {0, 0, 0},
    });

    app->gui = gui_create(app, kSidebarWidth);

    parsh_context* shaders = parsh_create_context_from_file("src/demo.glsl");
    parsh_add_block(shaders, "prefix", "#version 330\n");

    sg_buffer positions_buffer = sg_make_buffer(&(sg_buffer_desc){
        .size = sizeof(positions),
        .usage = SG_USAGE_IMMUTABLE,
        .content = positions,
    });

    sg_buffer texcoords_buffer = sg_make_buffer(&(sg_buffer_desc){
        .size = sizeof(texcoords),
        .usage = SG_USAGE_IMMUTABLE,
        .content = texcoords,
    });

    sg_buffer index_buffer = sg_make_buffer(&(sg_buffer_desc){
        .size = sizeof(indices),
        .usage = SG_USAGE_IMMUTABLE,
        .content = indices,
        .type = SG_BUFFERTYPE_INDEXBUFFER,
    });

    app->gfx.num_elements = 6;

    app->gfx.bindings = (sg_bindings){
        .vertex_buffers[0] = positions_buffer,  //
        .vertex_buffers[1] = texcoords_buffer,
        .fs_images[0] = app->gfx.texture,
        .index_buffer = index_buffer,
    };

    sg_shader program = sg_make_shader(&(sg_shader_desc){
        .vs.uniform_blocks[0].size = sizeof(Uniforms),
        .vs.uniform_blocks[0].uniforms[0].name = "modelview",
        .vs.uniform_blocks[0].uniforms[0].type = SG_UNIFORMTYPE_MAT4,
        .vs.uniform_blocks[0].uniforms[1].name = "inverse_modelview",
        .vs.uniform_blocks[0].uniforms[1].type = SG_UNIFORMTYPE_MAT4,
        .vs.uniform_blocks[0].uniforms[2].name = "projection",
        .vs.uniform_blocks[0].uniforms[2].type = SG_UNIFORMTYPE_MAT4,
        .fs.images[0].name = "terrain",
        .fs.images[0].type = SG_IMAGETYPE_2D,
        .vs.source = parsh_get_blocks(shaders, "prefix terrain.vs"),
        .fs.source = parsh_get_blocks(shaders, "prefix terrain.fs"),
    });

    app->gfx.pipeline = sg_make_pipeline(&(sg_pipeline_desc){
        .shader = program,
        .blend.enabled = false,
        .depth_stencil.depth_write_enabled = false,
        .rasterizer.cull_mode = SG_CULLMODE_NONE,
        .index_type = SG_INDEXTYPE_UINT16,
        .layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT4,
        .layout.attrs[0].buffer_index = 0,
        .layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT2,
        .layout.attrs[1].buffer_index = 1,
    });
}

void app_draw(App* app) {
    const double seconds = stm_sec(stm_now());
    app_update_projection(app);

    if (app->transition.van_wijk) {
        const CameraTransition trans = app->transition;
        const double duration = parcc_get_interpolation_duration(trans.source, trans.target);
        const double kTransitionSpeed = 3;
        double elapsed = kTransitionSpeed * (seconds - app->transition.start_time) / duration;
        if (elapsed >= 1.0) {
            elapsed = 1.0;
            app->transition.van_wijk = false;
        }
        const parcc_frame frame = parcc_interpolate_frames(trans.source, trans.target, elapsed);
        parcc_set_frame(app->camera_controller, frame);
    }

    float view[16];
    camera_get_view_matrixf(app->camera, view);

    float model[16];
    float16_identity(model);

    float16_multiply(app->gfx.uniforms.modelview, model, view);
    float16_copy(app->gfx.uniforms.inverse_mv, app->gfx.uniforms.modelview);
    float16_invert(app->gfx.uniforms.inverse_mv);

    const sg_pass_action pass_action = {
        .colors[0].action = SG_ACTION_CLEAR,
        .colors[0].val = {0.1f, 0.2f, 0.3f, 1.0f},
    };

    const float vp_width = sapp_width() - kSidebarWidth;
    const float vp_height = sapp_height();

    sg_begin_default_pass(&pass_action, sapp_width(), sapp_height());
    sg_apply_viewport(kSidebarWidth, 0, vp_width, vp_height, false);
    sg_apply_pipeline(app->gfx.pipeline);
    sg_apply_bindings(&app->gfx.bindings);
    sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &app->gfx.uniforms, sizeof(Uniforms));
    sg_draw(0, app->gfx.num_elements, 1);

    sg_apply_viewport(0, 0, sapp_width(), vp_height, false);
    gui_draw(app->gui);

    sg_end_pass();
    sg_commit();
}

void app_start_camera_transition(App* app) {
    if (app->transition.van_wijk) {
        return;
    }
    app->transition.start_time = stm_sec(stm_now());
    app->transition.source = parcc_get_current_frame(app->camera_controller);
    app->transition.target = parcc_get_home_frame(app->camera_controller, 10.0);
    app->transition.van_wijk = true;
}
