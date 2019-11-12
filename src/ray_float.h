#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool intersect_triangle(const float orig[3], const float dir[3], const float vert0[3],
                        const float vert1[3], const float vert2[3], float* t, float* u, float* v);

// Vertices must be in ccw order
bool intersect_quad(const float orig[3], const float dir[3], const float sw[3], const float se[3],
                    const float ne[3], const float nw[3], float* t, float* u, float* v);

#ifdef __cplusplus
}
#endif
