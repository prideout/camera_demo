// Vanilla C bindings to nanort by Philip Rideout.
//
// Maybe one day this could be "par_tracer.h" but for now the implementation
// uses nanort which is C++.

#ifndef NANORT_C_H
#define NANORT_C_H

#include <stdbool.h>
#include <stddef.h>
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
#if NANO_RT_C_IMPLEMENTATION

#include <nanort/nanort.h>

struct part_context_s {
  nanort::BVHBuildOptions<float> options;
  nanort::TriangleMesh<float>* mesh;
  nanort::TriangleSAHPred<float>* surfaceAreaHeuristic;
  nanort::BVHAccel<float> accel;    // TODO: fix leak
  std::vector<unsigned int> faces;  // TODO: fix leak
  nanort::BVHTraceOptions* trace_options;
};

part_context* part_create_context(part_config config, part_mesh mesh) {
  part_context* context = PAR_CALLOC(part_context, 1);
  if (config.bin_size) {
    context->options.bin_size = config.bin_size;
  }
  if (config.cost_t_aabb) {
    context->options.cost_t_aabb = config.cost_t_aabb;
  }
  if (config.min_leaf_primitives) {
    context->options.min_leaf_primitives = config.min_leaf_primitives;
  }
  if (config.max_tree_depth) {
    context->options.max_tree_depth = config.max_tree_depth;
  }
  if (config.bin_size) {
    context->options.bin_size = config.bin_size;
  }
  if (config.shallow_depth) {
    context->options.shallow_depth = config.shallow_depth;
  }
  if (config.cache_bbox) {
    context->options.cache_bbox = config.cache_bbox;
  }

  context->faces = std::vector<unsigned int>(
      mesh.triangles, mesh.triangles + mesh.num_triangles * 3);

  context->mesh = new nanort::TriangleMesh<float>(
      mesh.vertices, context->faces.data(), sizeof(float) * 3);

  context->surfaceAreaHeuristic = new nanort::TriangleSAHPred<float>(
      mesh.vertices, context->faces.data(), 12);

  context->trace_options = new nanort::BVHTraceOptions();
  context->trace_options->cull_back_face = config.cull_backfaces;

  bool ret =
      context->accel.Build(mesh.num_triangles, *context->mesh,
                           *context->surfaceAreaHeuristic, context->options);

  if (!ret) {
    // TODO: fix memory leak
    return nullptr;
  }

  return context;
}

void part_destroy_context(part_context* ctx) {
  // TODO: invoke destructors in other members
  delete ctx->trace_options;
  delete ctx->mesh;
  delete ctx->surfaceAreaHeuristic;
  PAR_FREE(ctx);
}

bool part_trace(const part_context* ctx, part_ray ray,
                part_intersection* intersection) {
  nanort::TriangleIntersector<float> intersector(
      ctx->mesh->vertices_, ctx->mesh->faces_, sizeof(float) * 3);
  nanort::TriangleIntersection<float> isect;

  nanort::Ray<float> nray;
  nray.org[0] = ray.org[0];
  nray.org[1] = ray.org[1];
  nray.org[2] = ray.org[2];
  nray.dir[0] = ray.dir[0];
  nray.dir[1] = ray.dir[1];
  nray.dir[2] = ray.dir[2];
  nray.min_t = ray.min_t;
  nray.max_t = ray.max_t;

  bool hit =
      ctx->accel.Traverse(nray, intersector, &isect, *ctx->trace_options);
  if (!hit) {
    return false;
  }
  intersection->u = isect.u;
  intersection->v = isect.v;
  intersection->t = isect.t;
  intersection->triangle_index = isect.prim_id;
  return true;
}

#endif  // NANO_RT_C_IMPLEMENTATION
#endif  // NANO_RT_C_H
