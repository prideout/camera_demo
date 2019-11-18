// CAMERA CONTROL :: https://prideout.net/blog/par_camera_control/
// Enables orbit controls (a.k.a. tumble, arcball, trackball) or pan-and-zoom like Google Maps.
//
// This simple library controls a camera that orbits or pans over a 3D object or terrain. Users can
// control their viewing position by grabbing and dragging locations in the scene. Sometimes this is
// known as "through-the-lens" camera control. No assumptions are made about the renderer or
// platform. In a sense, this is just a math library.
//
// When the controller is in ORBIT mode, it allows users to move the camera longitudinally or
// latitudinally, but does not allow for tilting the "up" vector. The rationale is that people
// rarely crook their heads sideways when examining 3D objects. Quaternions are great, but they are
// avoided here! Instead the orientation of the camera is defined by a Y-axis rotation followed by
// an X-axis rotation.
//
// The library takes a raycast callback to support precise grabbing behavior. If this is not
// required for your use case (e.g. a top-down terrain with an orthgraphic projection), provide NULL
// for the callback and the library will simply raycast against the ground plane.
//
// The following is an abbreviated usage example, for a more complete example see:
// https://github.com/prideout/camera_demo
//
//     #define PAR_CAMERA_CONTROL_IMPLEMENTATION
//     #include "par_camera_control.h"
//
//     static bool raycast(float origin[3], float dir[3], float* t, void* my_mesh) {
//        ...
//     }
//
//     parcc_context* controller = parc_create_context((parcc_config) {
//         .mode = PARCC_MAP,
//         .viewport_width = 1024,
//         .viewport_height = 768,
//         .near_plane = 0.01,
//         .far_plane = 100.0,
//         .map_plane = {0, 0, 1, 0},      // the map lies a Z=0
//         .map_extent = {2000.0, 1000.0}, // width = 2000, height = 1000
//         .raycast_function = raycast,    // or use NULL to intersect plane
//         .raycast_userdata = my_mesh,
//     });
//
//    while (game_loop) {
//
//        parcc_float eyepos[3], target[3], upward[3];
//        parcc_get_look_at(controller, eyepos, target, upward);
//        // ^ or use "parcc_get_matrices" to compute 4x4 matrices
//
//        my_camera->set_look_at(eyepos, target, upward);
//        ...
//     }
//
//     parcc_destroy_context(controller);
//
// Distributed under the MIT License, see bottom of file.

#ifndef PAR_CAMERA_CONTROL_H
#define PAR_CAMERA_CONTROL_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef PARCC_USE_DOUBLE
typedef double parcc_float;
#else
typedef float parcc_float;
#endif

// The camera controller can be configured using either a VERTICAL or HORIZONTAL field of view.
// This specifies which of the two FOV angles should be held constant. For example, if you use a
// horizontal FOV, shrinking the viewport width will change the height of the frustum, but will
// leave the frustum width intact.
typedef enum {
    PARCC_VERTICAL,
    PARCC_HORIZONTAL,
} parcc_fov;

// The controller can be configured in orbit mode or pan-and-zoom mode.
typedef enum {
    PARCC_ORBIT,  // aka tumble, trackball, or arcball
    PARCC_MAP,    // pan and zoom like Google Maps
} parcc_mode;

// Captured camera state used for animation and bookmarks.
typedef struct {
    // Van Wijk parameters for MAP mode.
    parcc_float extent;     // world-space width or height, depending on fov_orientation.
    parcc_float center[2];  // vector from home_target that gets projected to map_plane.

    // Radius and euler angles for ORBIT mode. Nope, does not use quaternions.
    parcc_float radius;  // world-space distance between home_target and camera
    parcc_float theta;   // Y axis rotation (applied first)
    parcc_float phi;     // X axis rotation (applied second)
} parcc_frame;

typedef struct {
    parcc_float min_corner[3];
    parcc_float max_corner[3];
} parcc_aabb;

typedef struct {
    parcc_float min_value;
    parcc_float max_value;
} parcc_range;

typedef bool (*parcc_raycast_fn)(const parcc_float origin[3], const parcc_float dir[3],
                                 parcc_float* t, void* userdata);

// The config structure represents all user-controlled state in the library.
// The first few fields must be provided, but all remaining fields fall back to reasonable default
// values when they are zero-filled.
typedef struct {
    parcc_mode mode;         // must be PARCC_ORBIT or PARCC_MAP
    int viewport_width;      // horizontal extent in pixels
    int viewport_height;     // vertical extent in pixels
    parcc_float near_plane;  // distance between camera and near clipping plane
    parcc_float far_plane;   // distance between camera and far clipping plane

    parcc_float map_extent[2];  // constraints for map_plane (centered at home_target)
    parcc_float map_plane[4];   // plane equation with normalized XYZ, defaults to (0,0,1,0)

    parcc_fov fov_orientation;  // defaults to PARCC_VERTICAL
    parcc_float fov_degrees;    // full field-of-view angle (not half-angle), defaults to 33.
    parcc_float zoom_speed;     // defaults to 0.01

    parcc_raycast_fn raycast_function;  // defaults to plane & sphere intersectors
    void* raycast_userdata;             // arbitrary data for the raycast callback

    parcc_float home_target[3];  // world-space coordinate, defaults to (0,0,0)
    parcc_float home_upward[3];  // unit-length vector, defaults to (0,1,0)

    parcc_aabb orbit_aabb;                       // TODO: replace with sphere
    parcc_range orbit_constraint_theta_degrees;  // Y axis rotation, defaults to [-inf,+inf]
    parcc_range orbit_constraint_phi_degrees;    // X axis rotation, defaults to [-89, +89]
} parcc_config;

// Opaque handle to a camera controller and its memory arena.
typedef struct parcc_context_s parcc_context;

// Context and configuration functions.
parcc_context* parcc_create_context(parcc_config config);
parcc_config parcc_get_config(const parcc_context* context);
void parcc_set_config(parcc_context* context, parcc_config config);
void parcc_destroy_context(parcc_context* ctx);

// Camera retrieval functions.
void parcc_get_look_at(const parcc_context* ctx,  //
                       parcc_float eyepos[3],     //
                       parcc_float target[3],     //
                       parcc_float upward[3]);

void parcc_get_matrices(const parcc_context* ctx,    //
                        parcc_float projection[16],  //
                        parcc_float view[16]);

// Screen-space functions for user interaction. Each of these functions take winx / winy coords.
// The winx coord should be in [0, viewport_width) where 0 is the left-most column.
// The winy coord should be in [0, viewport_height) where 0 is the top-most row.
void parcc_grab_begin(parcc_context* context, int winx, int winy);
void parcc_grab_update(parcc_context* context, int winx, int winy, parcc_float scrolldelta);
void parcc_grab_end(parcc_context* context);
bool parcc_do_raycast(parcc_context* context, int winx, int winy, parcc_float result[3]);

// Frames (captured controller states) and Van Wijk interpolation functions.
parcc_frame parcc_get_current_frame(const parcc_context* context);
parcc_frame parcc_get_home_frame(const parcc_context* context);
void parcc_goto_frame(parcc_context* context, parcc_frame state);
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

#define PARCC_MIN(a, b) (a > b ? b : a)
#define PARCC_MAX(a, b) (a > b ? a : b)
#define PARCC_CLAMP(v, lo, hi) PARCC_MAX(lo, PARCC_MIN(hi, v))
#define PARCC_CALLOC(T, N) ((T*)calloc(N * sizeof(T), 1))
#define PARCC_FREE(BUF) free(BUF)

struct parcc_context_s {
    parcc_config config;
    parcc_float eyepos[3];
    parcc_float target[3];
    parcc_float upward[3];
    bool grabbing;
    parcc_float grab_point_far[3];
    parcc_float grab_point_world[3];
    parcc_float grab_point_eyepos[3];
    parcc_float grab_point_target[3];
};

static bool parcc_raycast_aabb(const parcc_float origin[3], const parcc_float dir[3],
                               parcc_float* t, void* userdata);

static bool parcc_raycast_plane(const parcc_float origin[3], const parcc_float dir[3],
                                parcc_float* t, void* userdata);

static void parcc_get_ray_far(parcc_context* context, int winx, int winy, parcc_float result[3]);

static void parcc_pan_with_constraints(parcc_context* context, const parcc_float vec[3]);

static void parcc_zoom_with_constraints(parcc_context* context, const parcc_float vec[3]);

parcc_context* parcc_create_context(parcc_config config) {
    parcc_context* context = PARCC_CALLOC(parcc_context, 1);
    parcc_set_config(context, config);
    parcc_goto_frame(context, parcc_get_home_frame(context));
    return context;
}

parcc_config parcc_get_config(const parcc_context* context) { return context->config; }

void parcc_set_config(parcc_context* context, parcc_config config) {
    if (config.fov_degrees == 0) {
        config.fov_degrees = 33;
    }
    if (config.zoom_speed == 0) {
        config.zoom_speed = 0.01;
    }
    if (float3_dot(config.home_upward, config.home_upward) == 0) {
        config.home_upward[1] = 1;
    }
    if (float4_dot(config.map_plane, config.map_plane) == 0) {
        config.map_plane[2] = 1;
    }

    parcc_range* theta = &config.orbit_constraint_theta_degrees;
    if (theta->min_value == 0 && theta->max_value == 0) {
        *theta = (parcc_range){-INFINITY, INFINITY};
    }

    parcc_range* phi = &config.orbit_constraint_phi_degrees;
    if (phi->min_value == 0 && phi->max_value == 0) {
        *phi = (parcc_range){-89, +89};
    }

    context->config = config;
}

void parcc_destroy_context(parcc_context* context) { PARCC_FREE(context); }

void parcc_get_matrices(const parcc_context* context, parcc_float projection[16],
                        parcc_float view[16]) {
    float16_look_at(view, context->eyepos, context->target, context->upward);
    const parcc_config cfg = context->config;
    const parcc_float aspect = (parcc_float)cfg.viewport_width / cfg.viewport_height;
    const parcc_float fov = cfg.fov_degrees;
    if (context->config.fov_orientation == PARCC_HORIZONTAL) {
        float16_perspective_x(projection, fov, aspect, cfg.near_plane, cfg.far_plane);
    } else {
        float16_perspective_y(projection, fov, aspect, cfg.near_plane, cfg.far_plane);
    }
}

void parcc_get_look_at(const parcc_context* ctx, parcc_float eyepos[3], parcc_float target[3],
                       parcc_float upward[3]) {
    float3_copy(eyepos, ctx->eyepos);
    float3_copy(target, ctx->target);
    float3_copy(upward, ctx->upward);
}

void parcc_grab_begin(parcc_context* context, int winx, int winy) {
    if (!parcc_do_raycast(context, winx, winy, context->grab_point_world)) {
        return;
    }
    context->grabbing = true;
    parcc_get_ray_far(context, winx, winy, context->grab_point_far);
    float3_copy(context->grab_point_eyepos, context->eyepos);
    float3_copy(context->grab_point_target, context->target);
}

void parcc_grab_update(parcc_context* context, int winx, int winy, parcc_float scrolldelta) {
    // Handle pan.
    if (context->grabbing) {
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

        parcc_pan_with_constraints(context, translation);
    }

    // Handle zoom.
    if (scrolldelta != 0.0) {
        parcc_float grab_point_world[3];
        if (!parcc_do_raycast(context, winx, winy, grab_point_world)) {
            return;
        }

        parcc_float grab_point_far[3];
        parcc_get_ray_far(context, winx, winy, grab_point_far);

        // We intentionally avoid normalizing this vector since you usually
        // want to slow down when approaching the surface.
        parcc_float u_vec[3];
        float3_subtract(u_vec, grab_point_world, context->eyepos);

        // Do not zoom in too far (this prevents getting stuck).
        const parcc_float zoom_speed = context->config.zoom_speed;
        const parcc_float u_len = float3_length(u_vec);
        if (u_len < zoom_speed && scrolldelta > 0.0) {
            return;
        }

        float3_scale(u_vec, scrolldelta * zoom_speed);
        parcc_zoom_with_constraints(context, u_vec);
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

    // Invoke the user's callback or fallback function.
    parcc_raycast_fn callback = context->config.raycast_function;
    parcc_raycast_fn fallback =
        context->config.mode == PARCC_ORBIT ? parcc_raycast_aabb : parcc_raycast_plane;
    void* userdata = context->config.raycast_userdata;
    if (!callback) {
        callback = fallback;
        userdata = context;
    }

    // If the ray misses, then try the fallback function.
    parcc_float t;
    if (!callback(origin, gaze, &t, userdata)) {
        if (callback == fallback) {
            return false;
        }
        if (!fallback(origin, gaze, &t, context)) {
            return false;
        }
    }

    float3_scale(gaze, t);
    float3_add(result, origin, gaze);
    return true;
}

parcc_frame parcc_get_current_frame(const parcc_context* context) {
    parcc_frame result = {0};
    if (context->config.mode == PARCC_MAP) {
        const parcc_float* origin = context->eyepos;
        const parcc_float* upward = context->config.home_upward;

        parcc_float direction[3];
        float3_subtract(direction, context->target, origin);
        float3_normalize(direction);

        parcc_float distance;
        parcc_raycast_plane(origin, direction, &distance, (void*)context);

        const parcc_float fov = context->config.fov_degrees * VEC_PI / 180.0;
        const parcc_float half_extent = distance * tan(fov / 2);

        parcc_float target[3];
        float3_scale(direction, distance);
        float3_add(target, origin, direction);

        // Compute the tangent frame defined by the map_plane normal and the home_upward vector.
        parcc_float uvec[3];
        parcc_float vvec[3];
        parcc_float target_to_eye[3];

        float3_copy(target_to_eye, context->config.map_plane);
        float3_cross(uvec, upward, target_to_eye);
        float3_cross(vvec, target_to_eye, uvec);

        float3_subtract(target, target, context->config.home_target);

        result.extent = half_extent * 2;
        result.center[0] = float3_dot(uvec, target);
        result.center[1] = float3_dot(vvec, target);
    }
    return result;
}

parcc_frame parcc_get_home_frame(const parcc_context* context) {
    parcc_frame result;
    if (context->config.mode == PARCC_MAP) {
        const bool horiz = context->config.fov_orientation == PARCC_HORIZONTAL;
        result.extent = horiz ? context->config.map_extent[0] : context->config.map_extent[1];
        result.center[0] = 0;
        result.center[1] = 0;
    }
    return result;
}

void parcc_goto_frame(parcc_context* context, parcc_frame frame) {
    if (context->config.mode == PARCC_MAP) {
        const parcc_float* upward = context->config.home_upward;
        const parcc_float half_extent = frame.extent / 2.0;
        const parcc_float fov = context->config.fov_degrees * VEC_PI / 180.0;
        const parcc_float distance = half_extent / tan(fov / 2);

        // Compute the tangent frame defined by the map_plane normal and the home_upward vector.
        parcc_float uvec[3];
        parcc_float vvec[3];
        parcc_float target_to_eye[3];

        float3_copy(target_to_eye, context->config.map_plane);
        float3_cross(uvec, upward, target_to_eye);
        float3_cross(vvec, target_to_eye, uvec);

        // Scale the U and V components by the frame coordinate.
        float3_scale(uvec, frame.center[0]);
        float3_scale(vvec, frame.center[1]);

        // Obtain the new target position by adding U and V to home_target.
        float3_copy(context->target, context->config.home_target);
        float3_add(context->target, context->target, uvec);
        float3_add(context->target, context->target, vvec);

        // Obtain the new eye position by adding the scaled plane normal to the new target position.
        float3_scale(target_to_eye, distance);
        float3_add(context->eyepos, context->target, target_to_eye);

        // The up vector should never change, but just for completeness go ahead and set it.
        float3_copy(context->upward, upward);
    }
}

parcc_frame parcc_interpolate_frames(parcc_frame a, parcc_frame b, double t) {
    const double rho = sqrt(2.0);
    const double rho2 = 2, rho4 = 4;
    const double ux0 = a.center[0], uy0 = a.center[1], w0 = a.extent;
    const double ux1 = b.center[0], uy1 = b.center[1], w1 = b.extent;
    const double dx = ux1 - ux0, dy = uy1 - uy0, d2 = dx * dx + dy * dy, d1 = sqrt(d2);
    const double b0 = (w1 * w1 - w0 * w0 + rho4 * d2) / (2.0 * w0 * rho2 * d1);
    const double b1 = (w1 * w1 - w0 * w0 - rho4 * d2) / (2.0 * w1 * rho2 * d1);
    const double r0 = log(sqrt(b0 * b0 + 1.0) - b0);
    const double r1 = log(sqrt(b1 * b1 + 1) - b1);
    const double dr = r1 - r0;
    const int valid_dr = (dr == dr) && dr != 0;
    const double S = (valid_dr ? dr : log(w1 / w0)) / rho;
    parcc_frame result;
    const double s = t * S;
    if (valid_dr) {
        const double coshr0 = cosh(r0);
        const double u = w0 / (rho2 * d1) * (coshr0 * tanh(rho * s + r0) - sinh(r0));
        result.center[0] = ux0 + u * dx;
        result.center[1] = uy0 + u * dy;
        result.extent = w0 * coshr0 / cosh(rho * s + r0);
        return result;
    }
    result.center[0] = ux0 + t * dx;
    result.center[1] = uy0 + t * dy;
    result.extent = w0 * exp(rho * s);
    return result;
}

double parcc_get_interpolation_duration(parcc_frame a, parcc_frame b) {
    const double rho = sqrt(2.0);
    const double rho2 = 2, rho4 = 4;
    const double ux0 = a.center[0], uy0 = a.center[1], w0 = a.extent;
    const double ux1 = b.center[0], uy1 = b.center[1], w1 = b.extent;
    const double dx = ux1 - ux0, dy = uy1 - uy0, d2 = dx * dx + dy * dy, d1 = sqrt(d2);
    const double b0 = (w1 * w1 - w0 * w0 + rho4 * d2) / (2.0 * w0 * rho2 * d1);
    const double b1 = (w1 * w1 - w0 * w0 - rho4 * d2) / (2.0 * w1 * rho2 * d1);
    const double r0 = log(sqrt(b0 * b0 + 1.0) - b0);
    const double r1 = log(sqrt(b1 * b1 + 1) - b1);
    const double dr = r1 - r0;
    const int valid_dr = (dr == dr) && dr != 0;
    const double S = (valid_dr ? dr : log(w1 / w0)) / rho;
    return fabs(S);
}

static bool parcc_raycast_aabb(const parcc_float origin[3], const parcc_float dir[3],
                               parcc_float* t, void* userdata) {
    typedef struct parcc_vec3 {
        parcc_float x, y, z;
    } parcc_vec3;
    parcc_context* context = (parcc_context*)userdata;
    const parcc_vec3 minc = *((parcc_vec3*)context->config.orbit_aabb.min_corner);
    const parcc_vec3 maxc = *((parcc_vec3*)context->config.orbit_aabb.max_corner);

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

static bool parcc_raycast_plane(const parcc_float origin[3], const parcc_float dir[3],
                                parcc_float* t, void* userdata) {
    typedef struct parcc_vec3 {
        parcc_float x, y, z;
    } parcc_vec3;
    parcc_context* context = (parcc_context*)userdata;
    const parcc_float* plane = context->config.map_plane;
    parcc_vec3 n = {plane[0], plane[1], plane[2]};
    parcc_vec3 p0 = n;
    float3_scale(&p0.x, plane[3]);
    const parcc_float denom = -float3_dot(&n.x, dir);
    if (denom > 1e-6) {
        parcc_vec3 p0l0;
        float3_subtract(&p0l0.x, &p0.x, origin);
        *t = float3_dot(&p0l0.x, &n.x) / -denom;
        return *t >= 0;
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

static void parcc_pan_with_constraints(parcc_context* context, const parcc_float vec[3]) {
    const bool horizontal = context->config.fov_orientation == PARCC_HORIZONTAL;
    const parcc_float width = context->config.viewport_width;
    const parcc_float height = context->config.viewport_height;
    const parcc_float aspect = width / height;
    const parcc_float map_width = context->config.map_extent[0] / 2;
    const parcc_float map_height = context->config.map_extent[1] / 2;

    float3_add(context->eyepos, context->grab_point_eyepos, vec);
    float3_add(context->target, context->grab_point_target, vec);

    parcc_frame frame = parcc_get_current_frame(context);
    parcc_float x = frame.center[0];
    parcc_float y = frame.center[1];

    if (context->config.fov_orientation == PARCC_HORIZONTAL) {
        const parcc_float vp_width = frame.extent / 2;
        const parcc_float vp_height = vp_width / aspect;
        assert(map_width >= vp_width);
        x = PARCC_CLAMP(x, -map_width + vp_width, map_width - vp_width);
        if (map_height < vp_height) {
            y = PARCC_CLAMP(y, -vp_height + map_height, vp_height - map_height);
        } else {
            y = PARCC_CLAMP(y, -map_height + vp_height, map_height - vp_height);
        }
    } else {
        const parcc_float vp_height = frame.extent / 2;
        const parcc_float vp_width = vp_height * aspect;
        assert(map_height >= vp_height);
        y = PARCC_CLAMP(y, -map_height + vp_height, map_height - vp_height);
        if (map_width < vp_width) {
            x = PARCC_CLAMP(x, -vp_width + map_width, vp_width - map_width);
        } else {
            x = PARCC_CLAMP(x, -map_width + vp_width, map_width - vp_width);
        }
    }

    frame.center[0] = x;
    frame.center[1] = y;

    parcc_goto_frame(context, frame);
}

static void parcc_zoom_with_constraints(parcc_context* context, const parcc_float vec[3]) {
    const bool horizontal = context->config.fov_orientation == PARCC_HORIZONTAL;
    const parcc_float* map_extent = context->config.map_extent;
    const parcc_float max_extent = horizontal ? map_extent[0] : map_extent[1];
    const parcc_float* upward = context->config.home_upward;
    const parcc_float fov = context->config.fov_degrees * VEC_PI / 180.0;

    parcc_float eyepos[3];
    parcc_float target[3];
    float3_copy(eyepos, context->eyepos);
    float3_copy(target, context->target);

    parcc_float direction[3];
    float3_subtract(direction, target, eyepos);
    float3_normalize(direction);

    // Apply unconstrained zoom.
    float3_add(context->eyepos, eyepos, vec);
    float3_add(context->target, target, vec);

    // Check if exceeds the max zoom.
    parcc_float distance;
    parcc_raycast_plane(context->eyepos, direction, &distance, (void*)context);
    const parcc_float extent = 2.0 * distance * tan(fov / 2.0);
    if (extent > max_extent) {
        const parcc_float ideal_distance = max_extent / (2.0 * tan(fov / 2.0));

        parcc_float hit_point[3];
        float3_scale(direction, distance);
        float3_add(hit_point, context->eyepos, direction);

        float3_normalize(direction);
        float3_scale(direction, -ideal_distance);

        // Determine the new camera position and target.
        float3_add(context->eyepos, hit_point, direction);
        parcc_float new_vec[3];
        float3_subtract(new_vec, context->eyepos, eyepos);
        float3_add(context->target, target, new_vec);
    }
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
