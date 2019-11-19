// Ray-Triangle Intersection Test Routine
// Tomas Moller and Ben Trumbore

#include "ray_float.h"
#include "vec_float.h"

#include <math.h>

#define EPSILON 0.000001
#define CROSS(dest, v1, v2)                  \
    dest[0] = v1[1] * v2[2] - v1[2] * v2[1]; \
    dest[1] = v1[2] * v2[0] - v1[0] * v2[2]; \
    dest[2] = v1[0] * v2[1] - v1[1] * v2[0];
#define DOT(v1, v2) (v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2])
#define SUB(dest, v1, v2)    \
    dest[0] = v1[0] - v2[0]; \
    dest[1] = v1[1] - v2[1]; \
    dest[2] = v1[2] - v2[2];

bool intersect_triangle(const float orig[3], const float dir[3], const float vert0[3],
                        const float vert1[3], const float vert2[3], float* t, float* u, float* v) {
    float edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
    float det, inv_det;

    /* find vectors for two edges sharing vert0 */
    SUB(edge1, vert1, vert0);
    SUB(edge2, vert2, vert0);

    /* begin calculating determinant - also used to calculate U parameter */
    CROSS(pvec, dir, edge2);

    /* if determinant is near zero, ray lies in plane of triangle */
    det = DOT(edge1, pvec);

    if (det > -EPSILON && det < EPSILON) return false;
    inv_det = 1.0 / det;

    /* calculate distance from vert0 to ray origin */
    SUB(tvec, orig, vert0);

    /* calculate U parameter and test bounds */
    *u = DOT(tvec, pvec) * inv_det;
    if (*u < 0.0 || *u > 1.0) return false;

    /* prepare to test V parameter */
    CROSS(qvec, tvec, edge1);

    /* calculate V parameter and test bounds */
    *v = DOT(dir, qvec) * inv_det;
    if (*v < 0.0 || *u + *v > 1.0) return false;

    /* calculate t, ray intersects triangle */
    *t = DOT(edge2, qvec) * inv_det;

    return true;
}

static void compute_bilinear_coord(const float orig[3], const float dir[3], float t,
                                   const float sw[3], const float se[3], const float ne[3],
                                   const float nw[3], float* u, float* v) {
    float p[3];
    float3_copy(p, dir);
    float3_scale(p, t);
    float3_add(p, p, orig);

    float pvec[3];
    float3_subtract(pvec, p, sw);

    float udir[3];
    float3_subtract(udir, se, sw);

    float vdir[3];
    float3_subtract(vdir, nw, sw);

    const float ulen2 = float3_dot(udir, udir);
    const float vlen2 = float3_dot(vdir, vdir);

    *u = float3_dot(pvec, udir) / ulen2;
    *v = float3_dot(pvec, vdir) / vlen2;
}

// TODO: use the Lagae and Dutré method instead
bool intersect_quad(const float orig[3], const float dir[3], const float sw[3], const float se[3],
                    const float ne[3], const float nw[3], float* t, float* u, float* v) {
    bool t0 = intersect_triangle(orig, dir, sw, se, ne, t, u, v);
    if (t0) {
        compute_bilinear_coord(orig, dir, *t, sw, se, ne, nw, u, v);
        return true;
    }
    bool t1 = intersect_triangle(orig, dir, ne, nw, sw, t, u, v);
    if (t1) {
        compute_bilinear_coord(orig, dir, *t, sw, se, ne, nw, u, v);
        return true;
    }
    return false;
}

#if 0
static bool parcc_raycast_aabb(const parcc_float origin[3], const parcc_float dir[3],
                               parcc_float* t, void* userdata) {
    typedef struct parcc_vec3 {
        parcc_float x, y, z;
    } parcc_vec3;
    parcc_context* context = (parcc_context*)userdata;
    const parcc_vec3 minc = *((parcc_vec3*)context->props.orbit_aabb.min_corner);
    const parcc_vec3 maxc = *((parcc_vec3*)context->props.orbit_aabb.max_corner);

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
#endif
