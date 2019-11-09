#pragma once

#include <math.h>

#ifndef VEC_PI
#define VEC_PI (3.14159265359)
#endif

inline void double3_set(double dst[3], double x, double y, double z) {
    dst[0] = x;
    dst[1] = y;
    dst[2] = z;
}

inline void double3_add(double dst[3], const double a[3], const double b[3]) {
    dst[0] = a[0] + b[0];
    dst[1] = a[1] + b[1];
    dst[2] = a[2] + b[2];
}

inline void double3_macc(double dst[3], const double src[3], double scale) {
    dst[0] += src[0] * scale;
    dst[1] += src[1] * scale;
    dst[2] += src[2] * scale;
}

inline void double3_subtract(double dst[3], const double a[3], const double b[3]) {
    dst[0] = a[0] - b[0];
    dst[1] = a[1] - b[1];
    dst[2] = a[2] - b[2];
}

inline double double3_dot(const double a[3], const double b[3]) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

inline void double3_cross(double dst[3], const double a[3], const double b[3]) {
    dst[0] = a[1] * b[2] - a[2] * b[1];
    dst[1] = a[2] * b[0] - a[0] * b[2];
    dst[2] = a[0] * b[1] - a[1] * b[0];
}

inline void double3_scale(double dst[3], double v) {
    dst[0] *= v;
    dst[1] *= v;
    dst[2] *= v;
}

inline double double3_length(double dst[3]) { return sqrtf(double3_dot(dst, dst)); }

inline void double3_normalize(double dst[3]) { double3_scale(dst, 1.0f / double3_length(dst)); }

inline void double3_copy(double dst[3], const double src[3]) {
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
}

inline void double3_copy_to_vec4(double dst[4], const double src[3], double w) {
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
    dst[3] = w;
}

inline void double3_lerp(double dst[3], const double a[3], const double b[3], double t) {
    dst[0] = a[0] * (1 - t) + b[0] * t;
    dst[1] = a[1] * (1 - t) + b[1] * t;
    dst[2] = a[2] * (1 - t) + b[2] * t;
}

inline void double3_min(double dst[3], const double a[3], const double b[3]) {
    dst[0] = fmin(a[0], b[0]);
    dst[1] = fmin(a[1], b[1]);
    dst[2] = fmin(a[2], b[2]);
}

inline void double3_max(double dst[3], const double a[3], const double b[3]) {
    dst[0] = fmax(a[0], b[0]);
    dst[1] = fmax(a[1], b[1]);
    dst[2] = fmax(a[2], b[2]);
}

inline void double4_set(double dst[4], double x, double y, double z, double w) {
    dst[0] = x;
    dst[1] = y;
    dst[2] = z;
    dst[3] = w;
}

inline void double4_copy(double dst[4], const double src[4]) {
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
    dst[3] = src[3];
}

inline double double4_dot(const double a[4], const double b[4]) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3];
}

inline void double4_add(double dst[4], const double a[4], const double b[4]) {
    dst[0] = a[0] + b[0];
    dst[1] = a[1] + b[1];
    dst[2] = a[2] + b[2];
    dst[3] = a[3] + b[3];
}

inline void double4_scale(double dst[4], double v) {
    dst[0] *= v;
    dst[1] *= v;
    dst[2] *= v;
    dst[3] *= v;
}

inline void double4_scaled(double dst[4], const double src[4], double v) {
    dst[0] = src[0] * v;
    dst[1] = src[1] * v;
    dst[2] = src[2] * v;
    dst[3] = src[3] * v;
}

inline void double4_macc(double dst[4], const double src[4], double v) {
    dst[0] = fma(src[0], v, dst[0]);
    dst[1] = fma(src[1], v, dst[1]);
    dst[2] = fma(src[2], v, dst[2]);
    dst[3] = fma(src[3], v, dst[3]);
}

inline void double4_lerp(double dst[4], const double a[4], const double b[4], double t) {
    dst[0] = a[0] * (1 - t) + b[0] * t;
    dst[1] = a[1] * (1 - t) + b[1] * t;
    dst[2] = a[2] * (1 - t) + b[2] * t;
    dst[3] = a[3] * (1 - t) + b[3] * t;
}

inline void double16_copy(double dst[16], const double src[16]) {
    for (int i = 0; i < 16; ++i) {
        dst[i] = src[i];
    }
}

inline void double16_scale(double dst[16], double v) {
    for (int i = 0; i < 16; i++) {
        dst[i] *= v;
    }
}

inline void double16_identity(double dst[16]) {
    dst[0] = 1;
    dst[1] = 0;
    dst[2] = 0;
    dst[3] = 0;
    dst[4] = 0;
    dst[5] = 1;
    dst[6] = 0;
    dst[7] = 0;
    dst[8] = 0;
    dst[9] = 0;
    dst[10] = 1;
    dst[11] = 0;
    dst[12] = 0;
    dst[13] = 0;
    dst[14] = 0;
    dst[15] = 1;
}

inline void double16_multiply(double dst[16], const double a[16], const double b[16]) {
    dst[0] = double4_dot(a + 0, (double[4]){b[0], b[4], b[8], b[12]});
    dst[1] = double4_dot(a + 0, (double[4]){b[1], b[5], b[9], b[13]});
    dst[2] = double4_dot(a + 0, (double[4]){b[2], b[6], b[10], b[14]});
    dst[3] = double4_dot(a + 0, (double[4]){b[3], b[7], b[11], b[15]});

    dst[0 + 4] = double4_dot(a + 4, (double[4]){b[0], b[4], b[8], b[12]});
    dst[1 + 4] = double4_dot(a + 4, (double[4]){b[1], b[5], b[9], b[13]});
    dst[2 + 4] = double4_dot(a + 4, (double[4]){b[2], b[6], b[10], b[14]});
    dst[3 + 4] = double4_dot(a + 4, (double[4]){b[3], b[7], b[11], b[15]});

    dst[0 + 8] = double4_dot(a + 8, (double[4]){b[0], b[4], b[8], b[12]});
    dst[1 + 8] = double4_dot(a + 8, (double[4]){b[1], b[5], b[9], b[13]});
    dst[2 + 8] = double4_dot(a + 8, (double[4]){b[2], b[6], b[10], b[14]});
    dst[3 + 8] = double4_dot(a + 8, (double[4]){b[3], b[7], b[11], b[15]});

    dst[0 + 12] = double4_dot(a + 12, (double[4]){b[0], b[4], b[8], b[12]});
    dst[1 + 12] = double4_dot(a + 12, (double[4]){b[1], b[5], b[9], b[13]});
    dst[2 + 12] = double4_dot(a + 12, (double[4]){b[2], b[6], b[10], b[14]});
    dst[3 + 12] = double4_dot(a + 12, (double[4]){b[3], b[7], b[11], b[15]});
}

inline void double16_translation(double dst[16], const double src[3]) {
    dst[0] = 1;
    dst[1] = 0;
    dst[2] = 0;
    dst[3] = 0;
    dst[4] = 0;
    dst[5] = 1;
    dst[6] = 0;
    dst[7] = 0;
    dst[8] = 0;
    dst[9] = 0;
    dst[10] = 1;
    dst[11] = 0;
    dst[12] = src[0];
    dst[13] = src[1];
    dst[14] = src[2];
    dst[15] = 1;
}

inline void double16_transform(double dst[4], const double src[16]) {
    double x = double4_dot(dst, (double[4]){src[0], src[4], src[8], src[12]});
    double y = double4_dot(dst, (double[4]){src[1], src[5], src[9], src[13]});
    double z = double4_dot(dst, (double[4]){src[2], src[6], src[10], src[14]});
    double w = double4_dot(dst, (double[4]){src[3], src[7], src[11], src[15]});
    dst[0] = x;
    dst[1] = y;
    dst[2] = z;
    dst[3] = w;
}

inline void double16_look_at(double dst[16], const double eye[3], const double target[3],
                             const double up[3]) {
    double v3X[3];
    double v3Y[3];
    double v3Z[3];

    double3_copy(v3Y, up);
    double3_normalize(v3Y);

    double3_subtract(v3Z, eye, target);
    double3_normalize(v3Z);

    double3_cross(v3X, v3Y, v3Z);
    double3_normalize(v3X);

    double3_cross(v3Y, v3Z, v3X);

    double4_set(dst + 0, v3X[0], v3Y[0], v3Z[0], 0);
    double4_set(dst + 4, v3X[1], v3Y[1], v3Z[1], 0);
    double4_set(dst + 8, v3X[2], v3Y[2], v3Z[2], 0);
    double4_set(dst + 12,                //
                -double3_dot(v3X, eye),  //
                -double3_dot(v3Y, eye),  //
                -double3_dot(v3Z, eye), 1.0);
}

inline void double16_perspective_y(double dst[16], double fovy_degrees, double aspect_ratio,
                                   double near, double far) {
    const double fovy_radians = fovy_degrees * VEC_PI / 180;
    const double f = tanf(VEC_PI / 2.0f - 0.5f * fovy_radians);
    const double rangeinv = 1.0f / (near - far);
    dst[0] = f / aspect_ratio;
    dst[1] = 0;
    dst[2] = 0;
    dst[3] = 0;
    dst[4] = 0;
    dst[5] = f;
    dst[6] = 0;
    dst[7] = 0;
    dst[8] = 0;
    dst[9] = 0;
    dst[10] = (near + far) * rangeinv;
    dst[11] = -1;
    dst[12] = 0;
    dst[13] = 0;
    dst[14] = ((near * far) * rangeinv) * 2.0f;
    dst[15] = 0;
}

inline void double16_perspective_x(double dst[16], double fovy_degrees, double aspect_ratio,
                                   double near, double far) {
    const double fovy_radians = fovy_degrees * VEC_PI / 180;
    const double f = tanf(VEC_PI / 2.0f - 0.5f * fovy_radians);
    const double rangeinv = 1.0f / (near - far);
    dst[0] = f;
    dst[1] = 0;
    dst[2] = 0;
    dst[3] = 0;
    dst[4] = 0;
    dst[5] = f * aspect_ratio;
    dst[6] = 0;
    dst[7] = 0;
    dst[8] = 0;
    dst[9] = 0;
    dst[10] = (near + far) * rangeinv;
    dst[11] = -1;
    dst[12] = 0;
    dst[13] = 0;
    dst[14] = ((near * far) * rangeinv) * 2.0f;
    dst[15] = 0;
}

inline void double16_transpose(double dst[16]) {
    double m[16];
    double4_set(m + 0, dst[0], dst[4], dst[8], dst[12]);
    double4_set(m + 4, dst[1], dst[5], dst[9], dst[13]);
    double4_set(m + 8, dst[2], dst[6], dst[10], dst[14]);
    double4_set(m + 12, dst[3], dst[7], dst[11], dst[15]);
    double16_copy(dst, m);
}

inline void double16_invert(double dst[16]) {
    double a00 = dst[0], a01 = dst[1], a02 = dst[2], a03 = dst[3];
    double a10 = dst[4], a11 = dst[5], a12 = dst[6], a13 = dst[7];
    double a20 = dst[8], a21 = dst[9], a22 = dst[10], a23 = dst[11];
    double a30 = dst[12], a31 = dst[13], a32 = dst[14], a33 = dst[15];
    double b00 = a00 * a11 - a01 * a10;
    double b01 = a00 * a12 - a02 * a10;
    double b02 = a00 * a13 - a03 * a10;
    double b03 = a01 * a12 - a02 * a11;
    double b04 = a01 * a13 - a03 * a11;
    double b05 = a02 * a13 - a03 * a12;
    double b06 = a20 * a31 - a21 * a30;
    double b07 = a20 * a32 - a22 * a30;
    double b08 = a20 * a33 - a23 * a30;
    double b09 = a21 * a32 - a22 * a31;
    double b10 = a21 * a33 - a23 * a31;
    double b11 = a22 * a33 - a23 * a32;
    double det = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;
    if (det == 0.0) {
        return;
    }
    det = 1.0 / det;
    dst[0] = (a11 * b11 - a12 * b10 + a13 * b09) * det;
    dst[1] = (a02 * b10 - a01 * b11 - a03 * b09) * det;
    dst[2] = (a31 * b05 - a32 * b04 + a33 * b03) * det;
    dst[3] = (a22 * b04 - a21 * b05 - a23 * b03) * det;
    dst[4] = (a12 * b08 - a10 * b11 - a13 * b07) * det;
    dst[5] = (a00 * b11 - a02 * b08 + a03 * b07) * det;
    dst[6] = (a32 * b02 - a30 * b05 - a33 * b01) * det;
    dst[7] = (a20 * b05 - a22 * b02 + a23 * b01) * det;
    dst[8] = (a10 * b10 - a11 * b08 + a13 * b06) * det;
    dst[9] = (a01 * b08 - a00 * b10 - a03 * b06) * det;
    dst[10] = (a30 * b04 - a31 * b02 + a33 * b00) * det;
    dst[11] = (a21 * b02 - a20 * b04 - a23 * b00) * det;
    dst[12] = (a11 * b07 - a10 * b09 - a12 * b06) * det;
    dst[13] = (a00 * b09 - a01 * b07 + a02 * b06) * det;
    dst[14] = (a31 * b01 - a30 * b03 - a32 * b00) * det;
    dst[15] = (a20 * b03 - a21 * b01 + a22 * b00) * det;
}

inline void double16_rotation_x(double dst[16], double radians) {
    const double s = sinf(radians);
    const double c = cosf(radians);
    double4_set(dst + 0, 1, 0, 0, 0);
    double4_set(dst + 4, 0, c, s, 0);
    double4_set(dst + 8, 0, -s, c, 0);
    double4_set(dst + 12, 0, 0, 0, 1);
}

inline void double16_rotation_y(double dst[16], double radians) {
    const double s = sinf(radians);
    const double c = cosf(radians);
    double4_set(dst + 0, c, 0, -s, 0);
    double4_set(dst + 4, 0, 1, 0, 0);
    double4_set(dst + 8, s, 0, c, 0);
    double4_set(dst + 12, 0, 0, 0, 1);
}

inline void double16_rotation_z(double dst[16], double radians) {
    const double s = sinf(radians);
    const double c = cosf(radians);
    double4_set(dst + 0, c, s, 0, 0);
    double4_set(dst + 4, -s, c, 0, 0);
    double4_set(dst + 8, 0, 0, 1, 0);
    double4_set(dst + 12, 0, 0, 0, 1);
}

inline void double16_rotation(double dst[16], double radians, const double axis[3]) {
    const double s = sinf(radians);
    const double c = cosf(radians);
    const double x = axis[0];
    const double y = axis[1];
    const double z = axis[2];
    const double xy = x * y;
    const double yz = y * z;
    const double zx = z * x;
    const double d = 1.0f - c;
    double4_set(dst + 0, (((x * x) * d) + c), ((xy * d) + (z * s)), ((zx * d) - (y * s)), 0.0f);
    double4_set(dst + 4, ((xy * d) - (z * s)), (((y * y) * d) + c), ((yz * d) + (x * s)), 0.0f);
    double4_set(dst + 8, ((zx * d) + (y * s)), ((yz * d) - (x * s)), (((z * z) * d) + c), 0.0f);
    double4_set(dst + 12, 0, 0, 0, 1);
}

inline void double16_cast(double dst[16], const float src[16]) {
    for (int i = 0; i < 16; ++i) {
        dst[i] = src[i];
    }
}

inline void float16_cast(float dst[16], const double src[16]) {
    for (int i = 0; i < 16; ++i) {
        dst[i] = src[i];
    }
}

#ifdef TVECTOR_STDIO

#include <stdio.h>

inline void double3_print(FILE* out, const double src[3]) {
    fprintf(out, "[%g %g %g]", src[0], src[1], src[2]);
}

inline void double4_print(FILE* out, const double src[4]) {
    fprintf(out, "[%g %g %g %g]", src[0], src[1], src[2], src[3]);
}

inline void double16_print(FILE* out, const double src[16]) {
    fprintf(out, "[[%g %g %g %g]\n", src[0], src[1], src[2], src[3]);
    fprintf(out, " [%g %g %g %g]\n", src[4], src[5], src[6], src[7]);
    fprintf(out, " [%g %g %g %g]\n", src[8], src[9], src[10], src[11]);
    fprintf(out, " [%g %g %g %g]]\n", src[12], src[13], src[14], src[15]);
}

#endif  // TVECTOR_STDIO
