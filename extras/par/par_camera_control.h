// CAMERA CONTROL :: https://prideout.net/blog/par_camera_control/
// Enables orbit controls (a.k.a. tumble, arcball, trackball) or pan-and-zoom like Google Maps.
//
// This simple library controls a camera that orbits or pans over a 3D object or terrain. Users can
// control their viewing position by grabbing and dragging locations in the scene. Sometimes this is
// known as "through-the-lens" camera control. No assumptions are made about the renderer or
// platform. In a sense, this is just a math library.
//
// When the controller is in ORBIT mode, it allows users to move the camera longitudinally or
// latitudinally, but does not allow for tilting the "up" vector. The idea behind this constraint is
// that people rarely crook their heads sideways when examining 3D objects. By design, quaternions
// are not used. Instead the orientation of the camera is defined by a Y-axis rotation followed by
// an X-axis rotation.
//
// Clients can also use this to implement "spin the object" functionality rather than "orbit the
// camera", but the latter is what I designed it for. The library takes a raycast callback to
// support precise grabbing behavior. If this is not required for your use case (e.g. a top-down
// terrain with an orthgraphic projection), provide NULL for the callback and the library will
// simply raycast against the axis-aligned bounding box that you provide.
//
//   #define PAR_CAMERA_CONTROL_IMPLEMENTATION
//   #include "par_camera_control.h"
//
//   static bool raycast(float origin[3], float dir[3], float* t, void* userdata) {
//      ... intersect with ground plane at z = 0
//   }
//
//   parcc_context* controller = parc_create_context((parcc_config) {
//       .foo = bar,
//       .raycast_function = raycast,
//       .raycast_userdata = userdata,
//   });
//
//  while (game_loop_is_alive) {
//      bool dirty = parcc_tick(controller, get_time_seconds());
//      if (dirty) { ... }
//   }
//   ....
//   parcc_destroy_context(controller);
//
// Distributed under the MIT License, see bottom of file.

#ifndef PAR_CAMERA_CONTROL_H
#define PAR_CAMERA_CONTROL_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef parcc_float
#define parcc_float float
#endif

// The camera controller can be configured using either a VERTICAL or HORIZONTAL field of view. This
// specifies which of the two FOV angles should be held constant. For example, if you use a
// horizontal FOV, shrinking the viewport width will change the height of the frustum, but will
// leave the frustum width intact.
typedef enum {
    PARCC_VERTICAL,
    PARCC_HORIZONTAL,
} parcc_fov;

// The controller can be configured in orbit mode or pan-and-zoom mode. The mode can be changed at
// run time, in which case the controller will smoothly animate to a common "home" position.
typedef enum {
    PARCC_ORBIT,  // aka tumble, trackball, or arcball
    PARCC_MAP,    // pan and zoom like Google Maps
} parcc_mode;

// Captured camera state (in world-space) used for Van Wijk animation in MAP mode.
typedef struct {
    parcc_float extent;  // This is either width or height, depending on fov_orientation.
    parcc_float center_x;
    parcc_float center_y;
} parcc_frame;

typedef struct {
    parcc_float min_corner[3];
    parcc_float max_corner[3];
} parcc_aabb;

typedef bool (*parcc_raycast_fn)(const parcc_float origin[3], const parcc_float dir[3],
                                 parcc_float* t, void* userdata);

typedef struct {
    parcc_mode mode;
    int viewport_width;
    int viewport_height;
    parcc_float near_plane;
    parcc_float far_plane;
    parcc_fov fov_orientation;
    parcc_float fov_degrees;
    parcc_aabb content_aabb;
    parcc_raycast_fn raycast_function;
    void* raycast_userdata;
} parcc_config;

// Opaque handle to a camera controller and its memory arena.
typedef struct parcc_context_s parcc_context;

// Context and configuration functions.
parcc_context* parcc_create_context(parcc_config config);
parcc_config parcc_get_config(const parcc_context* context);
void parcc_set_config(parcc_context* context, parcc_config config);
void parcc_destroy_context(parcc_context* ctx);

// The "tick" function takes the current time (expressed in seconds), computes the current look-at
// vectors and camera matrices, advances animation, and returns true if the camera has been dirtied
// since the previous tick.
bool parcc_tick(parcc_context* ctx, double time);

// Camera retrieval functions. These can be called only after calling tick() at least once.
void parcc_get_look_at(const parcc_context* ctx, parcc_float eyepos[3], parcc_float target[3],
                       parcc_float upward[3]);
void parcc_get_matrix_projection(const parcc_context* ctx, parcc_float projection[16]);
void parcc_get_matrix_view(const parcc_context* ctx, parcc_float view[16]);

// Screen-space functions for user interaction. Each of these functions take winx / winy coords.
// The winx coord should be in [0, viewport_width) where 0 is the left-most column.
// The winy coord should be in [0, viewport_height) where 0 is the top-most row.
void parcc_grab_begin(parcc_context* context, int winx, int winy);
void parcc_grab_update(parcc_context* context, int winx, int winy, parcc_float scrolldelta);
void parcc_grab_end(parcc_context* context);
bool parcc_do_raycast(parcc_context* context, int winx, int winy, parcc_float result[3]);

// Frames (captured controller states) and Van Wijk interpolation functions.
parcc_frame parcc_get_current_frame(const parcc_context* context);
parcc_frame parcc_get_home_frame(const parcc_context* context, parcc_float margin);
void parcc_set_frame(parcc_context* context, parcc_frame state);
parcc_frame parcc_interpolate_frames(parcc_frame a, parcc_frame b, double t);
double parcc_get_interpolation_duration(parcc_frame a, parcc_frame b);

#ifdef __cplusplus
}
#endif

// -----------------------------------------------------------------------------
// END PUBLIC API
// -----------------------------------------------------------------------------
#ifdef PAR_CAMERA_CONTROL_IMPLEMENTATION

#include <assert.h>
#include <math.h>
#include <memory.h>
#include <stdlib.h>

#include "../../src/ray_float.h"  // TODO: integrate
#include "../../src/vec_float.h"  // TODO: integrate

#ifndef PAR_PI
#define PAR_PI (3.14159265359)
#define PAR_MIN(a, b) (a > b ? b : a)
#define PAR_MAX(a, b) (a > b ? a : b)
#define PAR_CLAMP(v, lo, hi) PAR_MAX(lo, PAR_MIN(hi, v))
#define PAR_SWAP(T, A, B) \
    {                     \
        T tmp = B;        \
        B = A;            \
        A = tmp;          \
    }
#define PAR_SQR(a) ((a) * (a))
#endif

#ifndef PAR_MALLOC
#define PAR_MALLOC(T, N) ((T*)malloc(N * sizeof(T)))
#define PAR_CALLOC(T, N) ((T*)calloc(N * sizeof(T), 1))
#define PAR_REALLOC(T, BUF, N) ((T*)realloc(BUF, sizeof(T) * (N)))
#define PAR_FREE(BUF) free(BUF)
#endif

struct parcc_context_s {
    parcc_config config;
    parcc_frame current_frame;
    parcc_float projection[16];
    parcc_float viewmatrix[16];
    parcc_float eyepos[3];
    parcc_float target[3];
    parcc_float upward[3];

    bool grabbing;
    parcc_float grab_point_far[3];
    parcc_float grab_point_world[3];
    parcc_float grab_point_eyepos[3];
    parcc_float grab_point_target[3];
};

static void parcc_get_ray_far(parcc_context* context, int winx, int winy, parcc_float result[3]);

static bool parcc_raycast_aabb(const parcc_float origin[3], const parcc_float dir[3],
                               parcc_float* t, void* userdata);

parcc_context* parcc_create_context(parcc_config config) {
    parcc_context* context = PAR_CALLOC(parcc_context, 1);
    context->config = config;
    parcc_float center[3];
    float3_lerp(center, config.content_aabb.min_corner, config.content_aabb.max_corner, 0.5);
    float3_set(context->eyepos, center[0], center[1], 2);  // TODO: compute initial distance
    float3_set(context->target, center[0], center[1], 0);
    float3_set(context->upward, 0, 1, 0);
    return context;
}

parcc_config parcc_get_config(const parcc_context* context) { return context->config; }

void parcc_set_config(parcc_context* context, parcc_config config) { context->config = config; }

void parcc_destroy_context(parcc_context* context) { PAR_FREE(context); }

bool parcc_tick(parcc_context* context, double seconds) {
    float16_look_at(context->viewmatrix, context->eyepos, context->target, context->upward);
    const parcc_config cfg = context->config;
    const parcc_float aspect = (parcc_float)cfg.viewport_width / cfg.viewport_height;
    const parcc_float fov = cfg.fov_degrees;
    if (context->config.fov_orientation == PARCC_HORIZONTAL) {
        float16_perspective_x(context->projection, fov, aspect, cfg.near_plane, cfg.far_plane);
    } else {
        float16_perspective_y(context->projection, fov, aspect, cfg.near_plane, cfg.far_plane);
    }
    return true;  // TODO: dirty flag
}

void parcc_get_look_at(const parcc_context* ctx, parcc_float eyepos[3], parcc_float target[3],
                       parcc_float upward[3]) {
    float3_copy(eyepos, ctx->eyepos);
    float3_copy(target, ctx->target);
    float3_copy(upward, ctx->upward);
}

void parcc_get_matrix_projection(const parcc_context* ctx, parcc_float projection[16]) {
    float16_copy(projection, ctx->projection);
}

void parcc_get_matrix_view(const parcc_context* ctx, parcc_float view[16]) {
    float16_copy(view, ctx->viewmatrix);
}

void parcc_grab_begin(parcc_context* context, int winx, int winy) {
    if (!parcc_do_raycast(context, winx, winy, context->grab_point_world)) {
        puts("missed\n");
        return;
    }
    context->grabbing = true;
    parcc_get_ray_far(context, winx, winy, context->grab_point_far);
    float3_copy(context->grab_point_eyepos, context->eyepos);
    float3_copy(context->grab_point_target, context->target);
}

void parcc_grab_update(parcc_context* context, int winx, int winy, parcc_float scrolldelta) {
    parcc_float u_vec[3];
    float3_subtract(u_vec, context->grab_point_world, context->grab_point_eyepos);
    const parcc_float u_len = float3_length(u_vec);

    parcc_float v_vec[3];
    float3_subtract(v_vec, context->grab_point_far, context->grab_point_world);
    const parcc_float v_len = float3_length(v_vec);

    parcc_float far_point[3];
    parcc_get_ray_far(context, winx, winy, far_point);

    parcc_float translation[3];
    float3_subtract(translation, far_point, context->grab_point_far);
    float3_scale(translation, -u_len / v_len);

    if (context->grabbing) {
        float3_add(context->eyepos, context->grab_point_eyepos, translation);
        float3_add(context->target, context->grab_point_target, translation);
    }

    // Handle zoom.
    if (scrolldelta != 0.0) {
        // TODO
        context->eyepos[2] -= scrolldelta * context->eyepos[2] * 0.01;  // TODO: kZoomSpeed
    }
}

void parcc_grab_end(parcc_context* context) { context->grabbing = false; }

bool parcc_do_raycast(parcc_context* context, int winx, int winy, parcc_float result[3]) {
    const parcc_float width = context->config.viewport_width;
    const parcc_float height = context->config.viewport_height;
    const parcc_float fov = context->config.fov_degrees * VEC_PI / 180.0;
    const bool vertical_fov = context->config.fov_orientation == PARCC_VERTICAL;
    const parcc_float* origin = context->eyepos;

    parcc_float gaze[3];
    float3_subtract(gaze, context->target, origin);
    float3_normalize(gaze);

    parcc_float right[3];
    float3_cross(right, gaze, context->upward);
    float3_normalize(right);

    parcc_float upward[3];
    float3_cross(upward, right, gaze);
    float3_normalize(upward);

    // Remap the grid coordinate into [-1, +1] and shift it to the pixel center.
    const parcc_float u = 2.0 * (winx + 0.5) / width - 1.0;
    const parcc_float v = 2.0 * (winy + 0.5) / height - 1.0;

    // Compute the tangent of the field-of-view angle as well as the aspect ratio.
    const parcc_float tangent = tan(fov / 2.0);
    const parcc_float aspect = width / height;

    // Adjust the gaze so it goes through the pixel of interest rather than the grid center.
    if (vertical_fov) {
        float3_scale(right, tangent * u * aspect);
        float3_scale(upward, tangent * v);
    } else {
        float3_scale(right, tangent * u);
        float3_scale(upward, tangent * v / aspect);
    }
    float3_add(gaze, gaze, right);
    float3_add(gaze, gaze, upward);
    float3_normalize(gaze);

    // Invoke the user's callback or fallback to our AABB intersector.
    parcc_raycast_fn callback = context->config.raycast_function;
    void* userdata = context->config.raycast_userdata;
    if (!callback) {
        callback = parcc_raycast_aabb;
        userdata = context;
    }

    parcc_float t;
    if (!callback(origin, gaze, &t, userdata)) {
        return false;
    }

    float3_scale(gaze, t);
    float3_add(result, origin, gaze);
    return true;
}

parcc_frame parcc_get_current_frame(const parcc_context* context) { return context->current_frame; }

parcc_frame parcc_get_home_frame(const parcc_context* context, parcc_float margin) {
    return (parcc_frame){0};
}

void parcc_set_frame(parcc_context* context, parcc_frame frame) { context->current_frame = frame; }

parcc_frame parcc_interpolate_frames(parcc_frame a, parcc_frame b, double t) {
    return (parcc_frame){0};
}

double parcc_get_interpolation_duration(parcc_frame a, parcc_frame b) { return 1.0; }

static bool parcc_raycast_aabb(const parcc_float origin[3], const parcc_float dir[3],
                               parcc_float* t, void* userdata) {
    typedef struct parcc_vec3 {
        parcc_float x, y, z;
    } parcc_vec3;
    parcc_context* context = (parcc_context*)userdata;
    const parcc_vec3 minc = *((parcc_vec3*)context->config.content_aabb.min_corner);
    const parcc_vec3 maxc = *((parcc_vec3*)context->config.content_aabb.max_corner);

    // The front face is defined in CCW order and the back face is defined in CW order.
    // Both start at the lower-left corner.

    const parcc_float fr[4][3] = {
        {minc.x, minc.y, maxc.z},
        {maxc.x, minc.y, maxc.z},
        {maxc.x, maxc.y, maxc.z},
        {minc.x, maxc.y, maxc.z},
    };

    const parcc_float bk[4][3] = {
        {minc.x, minc.y, minc.z},
        {minc.x, maxc.y, minc.z},
        {maxc.x, maxc.y, minc.z},
        {maxc.x, minc.y, minc.z},
    };

    float u, v;

    // Front
    if (intersect_quad(origin, dir, fr[0], fr[1], fr[2], fr[3], t, &u, &v)) {
        return true;
    }

    // Back
    if (intersect_quad(origin, dir, bk[0], bk[1], bk[2], bk[3], t, &u, &v)) {
        return true;
    }

    // Right
    if (intersect_quad(origin, dir, fr[2], fr[1], bk[3], bk[2], t, &u, &v)) {
        return true;
    }

    // Left
    if (intersect_quad(origin, dir, fr[3], bk[1], bk[0], fr[0], t, &u, &v)) {
        return true;
    }

    // Bottom
    if (intersect_quad(origin, dir, bk[0], bk[3], fr[1], fr[0], t, &u, &v)) {
        return true;
    }

    // Top
    if (intersect_quad(origin, dir, bk[1], fr[3], fr[2], bk[2], t, &u, &v)) {
        return true;
    }

    return false;
}

// Finds the point on the frustum's far plane that a pick ray intersects.
static void parcc_get_ray_far(parcc_context* context, int winx, int winy, parcc_float result[3]) {
    const parcc_float width = context->config.viewport_width;
    const parcc_float height = context->config.viewport_height;
    const parcc_float fov = context->config.fov_degrees * VEC_PI / 180.0;
    const bool vertical_fov = context->config.fov_orientation == PARCC_VERTICAL;
    const parcc_float* origin = context->eyepos;

    parcc_float gaze[3];
    float3_subtract(gaze, context->target, origin);
    float3_normalize(gaze);

    parcc_float right[3];
    float3_cross(right, gaze, context->upward);
    float3_normalize(right);

    parcc_float upward[3];
    float3_cross(upward, right, gaze);
    float3_normalize(upward);

    // Remap the grid coordinate into [-1, +1] and shift it to the pixel center.
    const parcc_float u = 2.0 * (winx + 0.5) / width - 1.0;
    const parcc_float v = 2.0 * (winy + 0.5) / height - 1.0;

    // Compute the tangent of the field-of-view angle as well as the aspect ratio.
    const parcc_float tangent = tan(fov / 2.0);
    const parcc_float aspect = width / height;

    // Adjust the gaze so it goes through the pixel of interest rather than the grid center.
    if (vertical_fov) {
        float3_scale(right, tangent * u * aspect);
        float3_scale(upward, tangent * v);
    } else {
        float3_scale(right, tangent * u);
        float3_scale(upward, tangent * v / aspect);
    }
    float3_add(gaze, gaze, right);
    float3_add(gaze, gaze, upward);
    float3_scale(gaze, context->config.far_plane);
    float3_add(result, origin, gaze);
}

#endif  // PAR_CAMERA_CONTROL_IMPLEMENTATION
#endif  // PAR_CAMERA_CONTROL_H

// par_camera_control is distributed under the MIT license:
//
// Copyright (c) 2019 Philip Rideout
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
