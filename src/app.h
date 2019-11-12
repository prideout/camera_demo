#pragma once

#include <par/par_camera_control.h>
#include <par/par_msquares.h>

#include <sokol/sokol_gfx.h>

#include <nanort/nanort_c.h>

#include "camera.h"
#include "gui.h"

#define kSidebarWidth (300)
#define kFov (33)
#define kNearPlane (1)
#define kFarPlane (1000)

typedef enum { VISUAL_MODE_2D, VISUAL_MODE_3D } VisualMode;

typedef struct {
    float modelview[16];
    float inverse_mv[16];
    float projection[16];
} Uniforms;

typedef struct {
    Uniforms uniforms;
    sg_pipeline pipeline;
    sg_bindings bindings;
    sg_image_desc texture_desc;
    sg_image texture;
    int num_elements;
} GraphicsState;

typedef struct {
    bool van_wijk;
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
    float min_corner[3];
    float max_corner[3];
} App;

void app_init(App* app);
void app_draw(App* app);
void app_start_camera_transition(App* app);
void app_start_raytrace(App* app);

bool app_intersects_mesh(const float origin[3], const float dir[3], float* t, void* userdata);
