#include "vec_float.h"

extern inline void float3_set(float dst[3], float x, float y, float z);
extern inline void float3_add(float dst[3], const float a[3], const float b[3]);
extern inline void float3_macc(float dst[3], const float src[3], float scale);
extern inline void float3_subtract(float dst[3], const float a[3], const float b[3]);
extern inline float float3_dot(const float a[3], const float b[3]);
extern inline void float3_cross(float dst[3], const float a[3], const float b[3]);
extern inline void float3_scale(float dst[3], float v);
extern inline float float3_length(float dst[3]);
extern inline void float3_normalize(float dst[3]);
extern inline void float3_copy(float dst[3], const float src[3]);
extern inline void float3_copy_to_vec4(float dst[4], const float src[3], float w);
extern inline void float3_lerp(float dst[3], const float a[3], const float b[3], float t);

extern inline void float4_set(float dst[3], float x, float y, float z, float w);
extern inline void float4_copy(float dst[4], const float src[4]);
extern inline float float4_dot(const float a[4], const float b[4]);
extern inline void float4_add(float dst[4], const float a[4], const float b[4]);
extern inline void float4_scale(float dst[4], float v);
extern inline void float4_scaled(float dst[4], const float src[4], float v);
extern inline void float4_macc(float dst[4], const float src[4], float v);
extern inline void float4_lerp(float dst[4], const float a[4], const float b[4], float t);

extern inline void float16_copy(float dst[16], const float src[16]);
extern inline void float16_scale(float dst[16], float v);
extern inline void float16_identity(float dst[16]);
extern inline void float16_multiply(float dst[16], const float a[16], const float b[16]);
extern inline void float16_translation(float dst[16], const float src[3]);
extern inline void float16_transform(float dst[4], const float src[16]);
extern inline void float16_transpose(float dst[16]);
extern inline void float16_invert(float dst[16]);

extern inline void float16_look_at(float dst[16], const float eye[3], const float target[3],
                                   const float up[3]);

extern inline void float16_perspective_x(float dst[16], float fov_degrees, float aspect_ratio,
                                         float near, float far);

extern inline void float16_perspective_y(float dst[16], float fov_degrees, float aspect_ratio,
                                         float near, float far);

extern inline void float16_rotation_x(float dst[16], float radians);
extern inline void float16_rotation_y(float dst[16], float radians);
extern inline void float16_rotation_z(float dst[16], float radians);
extern inline void float16_rotation(float dst[16], float radians, const float axis[3]);

#ifdef TVECTOR_STDIO
extern inline void float3_print(FILE* out, const float src[3]);
extern inline void float4_print(FILE* out, const float src[4]);
extern inline void float16_print(FILE* out, const float src[16]);
#endif
