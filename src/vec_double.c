#include "vec_double.h"

extern inline void double3_set(double dst[3], double x, double y, double z);
extern inline void double3_add(double dst[3], const double a[3], const double b[3]);
extern inline void double3_macc(double dst[3], const double src[3], double scale);
extern inline void double3_subtract(double dst[3], const double a[3], const double b[3]);
extern inline double double3_dot(const double a[3], const double b[3]);
extern inline void double3_cross(double dst[3], const double a[3], const double b[3]);
extern inline void double3_scale(double dst[3], double v);
extern inline double double3_length(const double dst[3]);
extern inline void double3_normalize(double dst[3]);
extern inline void double3_copy(double dst[3], const double src[3]);
extern inline void double3_copy_to_vec4(double dst[4], const double src[3], double w);
extern inline void double3_lerp(double dst[3], const double a[3], const double b[3], double t);
extern inline void double3_min(double dst[3], const double a[3], const double b[3]);
extern inline void double3_max(double dst[3], const double a[3], const double b[3]);

extern inline void double4_set(double dst[3], double x, double y, double z, double w);
extern inline void double4_copy(double dst[4], const double src[4]);
extern inline double double4_dot(const double a[4], const double b[4]);
extern inline void double4_add(double dst[4], const double a[4], const double b[4]);
extern inline void double4_scale(double dst[4], double v);
extern inline void double4_scaled(double dst[4], const double src[4], double v);
extern inline void double4_macc(double dst[4], const double src[4], double v);
extern inline void double4_lerp(double dst[4], const double a[4], const double b[4], double t);

extern inline void double16_copy(double dst[16], const double src[16]);
extern inline void double16_scale(double dst[16], double v);
extern inline void double16_identity(double dst[16]);
extern inline void double16_multiply(double dst[16], const double a[16], const double b[16]);
extern inline void double16_translation(double dst[16], const double src[3]);
extern inline void double16_transform(double dst[4], const double src[16]);
extern inline void double16_transpose(double dst[16]);
extern inline void double16_invert(double dst[16]);

extern inline void double16_look_at(double dst[16], const double eye[3], const double target[3],
                                    const double up[3]);

extern inline void double16_perspective_x(double dst[16], double fov_degrees, double aspect_ratio,
                                          double near, double far);

extern inline void double16_perspective_y(double dst[16], double fov_degrees, double aspect_ratio,
                                          double near, double far);

extern inline void double16_rotation_x(double dst[16], double radians);
extern inline void double16_rotation_y(double dst[16], double radians);
extern inline void double16_rotation_z(double dst[16], double radians);
extern inline void double16_rotation(double dst[16], double radians, const double axis[3]);

extern inline void double16_cast(double dst[16], const float src[16]);
extern inline void float16_cast(float dst[16], const double src[16]);

#ifdef TVECTOR_STDIO
extern inline void double3_print(FILE* out, const double src[3]);
extern inline void double4_print(FILE* out, const double src[4]);
extern inline void double16_print(FILE* out, const double src[16]);
#endif
