// Vanilla C bindings to nanort by Philip Rideout.
// Maybe one day this could be "par_tracer.h" but for now the impl uses C++.

#ifndef NANORT_C_H
#define NANORT_C_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  float org[3];
  float dir[3];
  float min_t;
  float max_t;
} part_ray;

typedef struct {
  float u;
  float v;
  float t;
  size_t triangle_index;
} part_intersection;

typedef struct {
  const float* vertices;
  size_t num_vertices;
  const uint16_t* triangles;
  size_t num_triangles;
} part_mesh;

typedef struct {
  float cost_t_aabb;
  uint32_t min_leaf_primitives;
  uint32_t max_tree_depth;
  uint32_t bin_size;
  uint32_t shallow_depth;
  bool cache_bbox;
  bool cull_backfaces;
} part_config;

typedef struct part_context_s part_context;

part_context* part_create_context(part_config config, part_mesh mesh);

void part_destroy_context(part_context* ctx);

bool part_trace(const part_context* ctx, part_ray ray,
                part_intersection* isect);

#ifdef __cplusplus
}
#endif

// -----------------------------------------------------------------------------
// END PUBLIC API
// -----------------------------------------------------------------------------
#ifdef NANO_RT_C_IMPLEMENTATION

#include <nanort/nanort.h>

struct part_context_s {
  nanort::BVHBuildOptions<float> options;
  nanort::TriangleMesh<float> mesh;
  nanort::TriangleSAHPred<float> surfaceAreaHeuristic;
  nanort::BVHAccel<float> accel;
};

part_context* part_create_context(part_config config, part_mesh mesh) {
  part_context* context = PAR_CALLOC(part_context, 1);
  // TODO
  return context;
}

void part_destroy_context(part_context* ctx) { PAR_FREE(ctx); }

bool part_trace(const part_context* ctx, part_ray ray,
                part_intersection* isect) {
  // TODO
  return true;
}

#if 0
void foo() {
  nanort::BVHBuildOptions<float> options;

  nanort::TriangleMesh<float> triangleMesh(
      &mesh.vertices.at(0), &mesh.faces.at(0), sizeof(float) * 3);

  nanort::TriangleSAHPred<float> surfaceAreaHeuristic(
      &mesh.vertices.at(0), &mesh.faces.at(0), sizeof(float) * 3);

  nanort::BVHAccel<float> accel;

  ret = accel.Build(mesh.faces.size() / 3, triangleMesh, trianglePred, options);

  nanort::BVHAccel<float> accel;
  nanort::TriangleIntersector<float> isecter(
      triangleMesh.vertices_, triangleMesh.faces_, sizeof(float) * 3);

  nanort::TriangleIntersection<float> isect;
  bool hit = accel.Traverse(ray, isecter, &isect);
  if (hit) {
    occlusion += 1.0f;
  }
}
#endif

#endif  // NANO_RT_C_IMPLEMENTATION
#endif  // NANO_RT_C_H
