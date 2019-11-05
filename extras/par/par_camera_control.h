// CAMERA CONTROL :: https://prideout.net/blog/par_camera_control/
// Enables orbit controls (a.k.a. tumble, arcball, trackball) or pan-and-zoom like Google Maps.
//
// This simple library is designed around the idea of a camera orbiting (or panning) around (or
// over) a 3D object or swath of terrain, and that users would like to control their viewing
// location by grabbing and dragging locations in the scene. It makes no assumptions about
// your renderer or platform. In a sense, this is just a math library.
//
// If desired, clients could also use this library to help with "spin the object" functionality
// rather than "orbit the camera", but the latter is what we've designed it for.
//
// The library takes a raycast callback in order to support precise grabbing behavior. If this is
// not necessary for your use case (e.g. a top-down terrain with an orthgraphic projection), simply
// provide a plane intersection function, as shown below.
//
//   #define PAR_CAMERA_CONTROL_IMPLEMENTATION
//   #include "par_camera_control.h"
//
//   static bool raycast(float origin[3], float dir[3], float* t, void* userdata) {
//      ... intersect with ground plane at z = 0
//   }
//
//   parcc_context* controller = parc_create_context((parcc_config) {
//      ...
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

#if PARCC_USE_DOUBLE
typedef double parcc_float;
#else
typedef float parcc_float;
#endif

typedef enum {
    PARCC_VERTICAL,
    PARCC_HORIZONTAL,
} parcc_fov;

typedef enum {
    PARCC_ORBIT,  // aka tumble or trackball
    PARCC_MAP,    // pan and zoom like Google Maps
} parcc_mode;

typedef struct {
    parcc_float viewport_width;
    parcc_float x;
    parcc_float y;
} parcc_frame;

typedef struct {
    parcc_float min_corner[3];
    parcc_float max_corner[3];
} parcc_aabb;

typedef bool (*parcc_raycast_fn)(float origin[3], float dir[3], float* t, void* userdata);

typedef struct {
    parcc_mode mode;
    int viewport_width;
    int viewport_height;
    parcc_fov fov_direction;
    parcc_float fov_radians;
    parcc_aabb content_aabb;
    parcc_raycast_fn raycast;
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

// Camera retrieval functions.
void parcc_get_look_at(const parcc_context* ctx, parcc_float eyepos[3], parcc_float target[3],
                       parcc_float upward[3]);
void parcc_get_matrix_projection(const parcc_context* ctx, parcc_float projection[16]);
void parcc_get_matrix_view(const parcc_context* ctx, parcc_float view[16]);

// Window functions, which are useful for user interaction.
// Window coordinates are normalized into [0, +1] where (0, 0) is the top left.
void parcc_grab_begin(parcc_context* context, parcc_float winx, parcc_float winy);
void parcc_grab_update(parcc_context* context, parcc_float winx, parcc_float winy,
                       parcc_float scrolldelta);
void parcc_grab_end(parcc_context* context);

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
#include <limits.h>
#include <math.h>
#include <memory.h>
#include <stdlib.h>

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
};

parcc_context* parcc_create_context(parcc_config config) {
    parcc_context* context = PAR_CALLOC(parcc_context, 1);
    context->config = config;
    return context;
}

parcc_config parcc_get_config(const parcc_context* context) { return context->config; }

void parcc_set_config(parcc_context* context, parcc_config config) { context->config = config; }

void parsl_destroy_context(parcc_context* context) { PAR_FREE(context); }

void parcc_grab_begin(parcc_context* context, parcc_float winx, parcc_float winy) {}

void parcc_grab_update(parcc_context* context, parcc_float winx, parcc_float winy,
                       parcc_float scrolldelta) {}

void parcc_grab_end(parcc_context* context) {}

parcc_frame parcc_get_current_frame(const parcc_context* context) { return context->current_frame; }

parcc_frame parcc_get_home_frame(const parcc_context* context, parcc_float margin) {
    return (parcc_frame){0};
}

void parcc_set_frame(parcc_context* context, parcc_frame frame) { context->current_frame = frame; }

parcc_frame parcc_interpolate_frames(parcc_frame a, parcc_frame b, double t) {
    return (parcc_frame){0};
}

double parcc_get_interpolation_duration(parcc_frame a, parcc_frame b) { return 1.0; }

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
