#pragma once

#include <par/par_camera_control.h>
#include <par/par_msquares.h>

#include <sokol/sokol_gfx.h>

#include <nanort/nanort_c.h>

#include "gui.h"

#define kSidebarWidth (300)
#define kNearPlane (0.001)
#define kFarPlane (100)

typedef enum { VISUAL_MODE_2D, VISUAL_MODE_3D } VisualMode;

typedef struct {
    float modelview[16];
    float inverse_mv[16];
    float projection[16];
    float map_extent[2];
    float map_center[2];
} Uniforms;

typedef struct {
    Uniforms uniforms;
    sg_pipeline terrain_pipeline;
    sg_pipeline ocean_pipeline;
    sg_bindings terrain_bindings;
    sg_bindings ocean_bindings;
    sg_image_desc texture_desc;
    sg_image texture;
    int num_elements;
} GraphicsState;

typedef struct {
    bool enabled;
    parcc_frame source;
    parcc_frame target;
    double start_time;
} CameraTransition;

typedef struct App {
    VisualMode visual_mode;
    CameraTransition transition;
    parcc_context* camera_controller;
    GraphicsState gfx;
    Gui* gui;
    par_msquares_mesh const* mesh;
    part_context* raytracer;
    bool has_frame[2];
    parcc_frame saved_frame[2];
    float min_corner[3];
    float max_corner[3];
} App;

void app_init(App* app);
void app_draw(App* app);

void app_goto_frame(App* app, parcc_frame goal);
void app_save_frame(App* app, int index);

bool app_intersects_mesh(const float origin[3], const float dir[3], float* t, void* userdata);
