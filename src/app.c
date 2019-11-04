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
    printf("%s :: width = %d, height = %d\n", filename, width, height);
    float* float_data = malloc(sizeof(float) * width * height);
    for (int i = 0; i < width * height; i++) {
        // float_data[i] = (255 - u8_data[i]) / 255.0f;
        float_data[i] = (float)u8_data[i] / 255.0f;
    }
    stbi_image_free(u8_data);

    const int cellsize = 5;
    par_msquares_meshlist* meshes =
        par_msquares_grayscale(float_data, width, height, cellsize, 0.0f, PAR_MSQUARES_HEIGHTS);

    assert(meshes);
    free(float_data);

    int nmeshes = par_msquares_get_count(meshes);

    par_msquares_mesh const* mesh = par_msquares_get_mesh(meshes, 0);
    printf("mesh 0 of %d : %d verts, %d triangles (dim = %d)\n", nmeshes, mesh->npoints,
           mesh->ntriangles, mesh->dim);

    app->min_corner[0] = 5000;
    app->max_corner[0] = -5000;
    app->min_corner[1] = 5000;
    app->max_corner[1] = -5000;
    app->min_corner[2] = 5000;
    app->max_corner[2] = -5000;

    for (int i = 0; i < mesh->npoints * 3; i += 3) {
        app->min_corner[0] = fmin(app->min_corner[0], mesh->points[i]);
        app->min_corner[1] = fmin(app->min_corner[1], mesh->points[i + 1]);
        app->min_corner[2] = fmin(app->min_corner[2], mesh->points[i + 2]);

        app->max_corner[0] = fmax(app->max_corner[0], mesh->points[i]);
        app->max_corner[1] = fmax(app->max_corner[1], mesh->points[i + 1]);
        app->max_corner[2] = fmax(app->max_corner[2], mesh->points[i + 2]);
    }

    printf("min corner %g %g %g\n", app->min_corner[0], app->min_corner[1], app->min_corner[2]);
    printf("max corner %g %g %g\n", app->max_corner[0], app->max_corner[1], app->max_corner[2]);

    sg_buffer positions_buffer = sg_make_buffer(&(sg_buffer_desc){
        .size = sizeof(float) * mesh->dim * mesh->npoints,
        .usage = SG_USAGE_IMMUTABLE,
        .content = mesh->points,
    });

    sg_buffer index_buffer = sg_make_buffer(&(sg_buffer_desc){
        .size = sizeof(uint16_t) * mesh->ntriangles * 3,
        .usage = SG_USAGE_IMMUTABLE,
        .content = mesh->triangles,
        .type = SG_BUFFERTYPE_INDEXBUFFER,
    });

    app->gfx.bindings = (sg_bindings){
        .vertex_buffers[0] = positions_buffer,
        .fs_images[0] = app->gfx.texture,
        .index_buffer = index_buffer,
    };

    app->gfx.num_elements = mesh->ntriangles * 3;

    par_msquares_free(meshes);
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
    create_mesh(app, "extras/terrain/landmass.png");
    printf("Created mesh in %.0f ms\n", stm_ms(stm_diff(stm_now(), start_mesh)));

    app->camera = camera_create();

    app_update_projection(app);

    const float cx = 0.5f * (app->min_corner[0] + app->max_corner[0]);
    const float cy = 0.5f * (app->min_corner[1] + app->max_corner[1]);
    printf("Center: %g %g\n", cx, cy);

    const float vpwidth = sapp_width() - kSidebarWidth;
    const float vpheight = sapp_height();
    const float fovy = camera_get_fovy_radians(app->camera);
    const float fovx = fovy * vpwidth / vpheight;
    const float kInitialDistance = 1.0 / tanf(fovx);
    const float eyepos[] = {cx, cy, kInitialDistance};
    const float target[] = {cx, cy, 0};
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
        .depth_stencil.depth_compare_func = SG_COMPAREFUNC_LESS,
        .depth_stencil.depth_write_enabled = true,
        .rasterizer.cull_mode = SG_CULLMODE_NONE,
        .index_type = SG_INDEXTYPE_UINT16,
        .layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT3,
        .layout.attrs[0].buffer_index = 0,
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
        .depth.action = SG_ACTION_CLEAR,
        .depth.val = 1.0f,
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
