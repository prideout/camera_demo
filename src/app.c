#include <assert.h>
#include <math.h>
#include <stdio.h>

#include <nanort/nanort_c.h>

#include <par/par_camera_control.h>
#include <par/par_msquares.h>
#include <par/par_shaders.h>

#include <stb/stb_image.h>
#include <stb/stb_image_resize.h>

#include <sokol/sokol_gfx.h>
#include <sokol/sokol_time.h>

#include "app.h"
#include "ray_float.h"
#include "vec_float.h"

#define IMAX(a, b) (a > b ? a : b)

typedef struct {
    float x, y, z, w;
} vec4;

typedef struct {
    float x, y;
} vec2;

static void create_mesh(App* app, const char* filename) {
    int nchan;
    int width, height;
    stbi_uc* u8_data = stbi_load(filename, &width, &height, &nchan, 1);
    assert(u8_data);
    printf("%s :: width = %d, height = %d\n", filename, width, height);
    float* float_data = malloc(sizeof(float) * width * height);
    for (int i = 0; i < width * height; i++) {
        const float h = (float)u8_data[i] / 255.0f;
        float_data[i] = 2.0 * h * h * h / 15.0;
    }
    stbi_image_free(u8_data);

    const int cellsize = 5;
    par_msquares_meshlist* meshes =
        par_msquares_grayscale(float_data, width, height, cellsize, 0.0f, PAR_MSQUARES_HEIGHTS);

    assert(meshes);
    free(float_data);

    int nmeshes = par_msquares_get_count(meshes);

    par_msquares_mesh const* mesh = app->mesh = par_msquares_get_mesh(meshes, 0);
    printf("mesh 0 of %d : %d verts, %d triangles (dim = %d)\n", nmeshes, mesh->npoints,
           mesh->ntriangles, mesh->dim);

    float3_set(app->min_corner, 5000, 5000, 5000);
    float3_set(app->max_corner, -5000, -5000, -5000);
    for (int i = 0; i < mesh->npoints * 3; i += 3) {
        float3_min(app->min_corner, app->min_corner, mesh->points + i);
        float3_max(app->max_corner, app->max_corner, mesh->points + i);
    }

    printf("bounds = ");
    float3_print(stdout, app->min_corner);
    float3_print(stdout, app->max_corner);
    puts("");

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

    app->gfx.terrain_bindings = (sg_bindings){
        .vertex_buffers[0] = positions_buffer,
        .fs_images[0] = app->gfx.texture,
        .index_buffer = index_buffer,
    };

    app->gfx.ocean_bindings = (sg_bindings){
        .vertex_buffers[0] = positions_buffer,
        .fs_images[0] = app->gfx.texture,
    };

    app->gfx.num_elements = mesh->ntriangles * 3;
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

bool app_intersects_mesh(const float origin[3], const float dir[3], float* t, void* userdata) {
    App* app = userdata;
    part_ray ray = {
        .org = {origin[0], origin[1], origin[2]},
        .dir = {dir[0], dir[1], dir[2]},
        .min_t = 0.0f,
        .max_t = 9999.0f,
    };
    part_intersection isect;
    if (!part_trace(app->raytracer, ray, &isect)) {
        return false;
    }
    *t = isect.t;
    return true;
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
    printf("Created terrain mesh in %.0f ms\n", stm_ms(stm_diff(stm_now(), start_mesh)));

    const uint64_t start_bvh = stm_now();
    part_mesh mesh = {
        .vertices = app->mesh->points,
        .num_vertices = app->mesh->npoints,
        .triangles = app->mesh->triangles,
        .num_triangles = app->mesh->ntriangles,
    };
    app->raytracer = part_create_context((part_config){.bin_size = 5}, mesh);
    printf("Created raytracer BVH in %.0f ms\n", stm_ms(stm_diff(stm_now(), start_bvh)));

    const parcc_float extent[2] = {
        app->max_corner[0] - app->min_corner[0],
        app->max_corner[1] - app->min_corner[1],
    };

    parcc_float center[3];
    float3_lerp(center, app->min_corner, app->max_corner, 0.5);

    app->gfx.uniforms.map_extent[0] = extent[0];
    app->gfx.uniforms.map_extent[1] = extent[1];
    app->gfx.uniforms.map_center[0] = center[0];
    app->gfx.uniforms.map_center[1] = center[1];

    const parcc_properties props = {
        .mode = PARCC_ORBIT,
        .viewport_width = sapp_width() - kSidebarWidth,
        .viewport_height = sapp_height(),
        .near_plane = kNearPlane,
        .far_plane = kFarPlane,
        .fov_orientation = PARCC_HORIZONTAL,
        .home_target = {center[0], center[1], 0},

        // MAP MODE PROPERTIES:
        .map_extent = {extent[0], extent[1]},
        .map_min_distance = app->max_corner[2],
        .raycast_function = app_intersects_mesh,
        .raycast_userdata = (void*)app,

        // ORBIT MODE PROPERTIES:
        .home_vector = {0, 0, 1.75},
    };
    app->camera_controller = parcc_create_context(&props);

    app->gui = gui_create(app, kSidebarWidth);

    parsh_context* shaders = parsh_create_context_from_file("src/demo.glsl");
    parsh_add_block(shaders, "prefix", "#version 330\n");

    sg_shader_uniform_block_desc block = {
        .size = sizeof(Uniforms),
        .uniforms[0].name = "modelview",
        .uniforms[0].type = SG_UNIFORMTYPE_MAT4,
        .uniforms[1].name = "inverse_modelview",
        .uniforms[1].type = SG_UNIFORMTYPE_MAT4,
        .uniforms[2].name = "projection",
        .uniforms[2].type = SG_UNIFORMTYPE_MAT4,
        .uniforms[3].name = "map_extent",
        .uniforms[3].type = SG_UNIFORMTYPE_FLOAT2,
        .uniforms[4].name = "map_center",
        .uniforms[4].type = SG_UNIFORMTYPE_FLOAT2,
    };

    sg_shader terrain_program = sg_make_shader(&(sg_shader_desc){
        .vs.uniform_blocks[0] = block,
        .fs.images[0].name = "terrain",
        .fs.images[0].type = SG_IMAGETYPE_2D,
        .vs.source = parsh_get_blocks(shaders, "prefix terrain.vs"),
        .fs.source = parsh_get_blocks(shaders, "prefix terrain.fs"),
    });

    app->gfx.terrain_pipeline = sg_make_pipeline(&(sg_pipeline_desc){
        .shader = terrain_program,
        .blend.enabled = false,
        .depth_stencil.depth_compare_func = SG_COMPAREFUNC_LESS,
        .depth_stencil.depth_write_enabled = true,
        .rasterizer.cull_mode = SG_CULLMODE_NONE,
        .index_type = SG_INDEXTYPE_UINT16,
        .layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT3,
        .layout.attrs[0].buffer_index = 0,
    });

    sg_shader ocean_program = sg_make_shader(&(sg_shader_desc){
        .vs.uniform_blocks[0] = block,
        .fs.images[0].name = "ocean",
        .fs.images[0].type = SG_IMAGETYPE_2D,
        .vs.source = parsh_get_blocks(shaders, "prefix ocean.vs"),
        .fs.source = parsh_get_blocks(shaders, "prefix terrain.fs"),
    });

    app->gfx.terrain_pipeline = sg_make_pipeline(&(sg_pipeline_desc){
        .shader = terrain_program,
        .blend.enabled = false,
        .depth_stencil.depth_compare_func = SG_COMPAREFUNC_LESS,
        .depth_stencil.depth_write_enabled = true,
        .rasterizer.cull_mode = SG_CULLMODE_NONE,
        .index_type = SG_INDEXTYPE_UINT16,
        .layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT3,
        .layout.attrs[0].buffer_index = 0,
    });

    app->gfx.ocean_pipeline = sg_make_pipeline(&(sg_pipeline_desc){
        .shader = ocean_program,
        .blend.enabled = false,
        .depth_stencil.depth_compare_func = SG_COMPAREFUNC_LESS,
        .depth_stencil.depth_write_enabled = true,
        .rasterizer.cull_mode = SG_CULLMODE_NONE,
        .layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT3,
        .layout.attrs[0].buffer_index = 0,
    });
}

void app_draw(App* app) {
    const double seconds = stm_sec(stm_now());

    if (app->transition.enabled) {
        const CameraTransition anim = app->transition;
        const double elapsed = seconds - anim.start_time;
        const double kTransitionSpeed = 3;
        const double duration = parcc_get_interpolation_duration(anim.source, anim.target);
        const double t = kTransitionSpeed * elapsed / duration;
        if (t >= 1.0) {
            parcc_goto_frame(app->camera_controller, anim.target);
            app->transition.enabled = false;
        } else {
            parcc_frame frame = parcc_interpolate_frames(anim.source, anim.target, t);
            parcc_goto_frame(app->camera_controller, frame);
        }
    }

    float view[16];
    parcc_get_matrices(app->camera_controller, app->gfx.uniforms.projection, view);

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
    sg_apply_pipeline(app->gfx.terrain_pipeline);
    sg_apply_bindings(&app->gfx.terrain_bindings);
    sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &app->gfx.uniforms, sizeof(Uniforms));
    sg_draw(0, app->gfx.num_elements, 1);

    sg_apply_pipeline(app->gfx.ocean_pipeline);
    sg_apply_bindings(&app->gfx.ocean_bindings);
    sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &app->gfx.uniforms, sizeof(Uniforms));
    sg_draw(0, 6, 1);

    sg_apply_viewport(0, 0, sapp_width(), vp_height, false);
    gui_draw(app->gui);

    sg_end_pass();
    sg_commit();
}

void app_goto_frame(App* app, parcc_frame goal) {
    parcc_properties props;
    parcc_get_properties(app->camera_controller, &props);
    if (app->transition.enabled) {
        return;
    }
    app->transition.start_time = stm_sec(stm_now());
    app->transition.source = parcc_get_current_frame(app->camera_controller);
    app->transition.target = goal;
    app->transition.enabled = true;
}

void app_save_frame(App* app, int index) {
    app->saved_frame[index] = parcc_get_current_frame(app->camera_controller);
    app->has_frame[index] = true;
}

void app_clear_frames(App* app) {
    app->has_frame[0] = false;
    app->has_frame[1] = false;
}
