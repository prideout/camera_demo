#pragma once

#include <math.h>

#ifndef VEC_PI
#define VEC_PI (3.14159265359)
#endif

inline void float3_set(float dst[3], float x, float y, float z) {
    dst[0] = x;
    dst[1] = y;
    dst[2] = z;
}

inline void float3_add(float dst[3], const float a[3], const float b[3]) {
    dst[0] = a[0] + b[0];
    dst[1] = a[1] + b[1];
    dst[2] = a[2] + b[2];
}

inline void float3_macc(float dst[3], const float src[3], float scale) {
    dst[0] += src[0] * scale;
    dst[1] += src[1] * scale;
    dst[2] += src[2] * scale;
}

inline void float3_subtract(float dst[3], const float a[3], const float b[3]) {
    dst[0] = a[0] - b[0];
    dst[1] = a[1] - b[1];
    dst[2] = a[2] - b[2];
}

inline float float3_dot(const float a[3], const float b[3]) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

inline void float3_cross(float dst[3], const float a[3], const float b[3]) {
    dst[0] = a[1] * b[2] - a[2] * b[1];
    dst[1] = a[2] * b[0] - a[0] * b[2];
    dst[2] = a[0] * b[1] - a[1] * b[0];
}

inline void float3_scale(float dst[3], float v) {
    dst[0] *= v;
    dst[1] *= v;
    dst[2] *= v;
}

inline float float3_length(float dst[3]) { return sqrtf(float3_dot(dst, dst)); }

inline void float3_normalize(float dst[3]) { float3_scale(dst, 1.0f / float3_length(dst)); }

inline void float3_copy(float dst[3], const float src[3]) {
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
}

inline void float3_copy_to_vec4(float dst[4], const float src[3], float w) {
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
    dst[3] = w;
}

inline void float3_lerp(float dst[3], const float a[3], const float b[3], float t) {
    dst[0] = a[0] * (1 - t) + b[0] * t;
    dst[1] = a[1] * (1 - t) + b[1] * t;
    dst[2] = a[2] * (1 - t) + b[2] * t;
}

inline void float3_min(float dst[3], const float a[3], const float b[3]) {
    dst[0] = fminf(a[0], b[0]);
    dst[1] = fminf(a[1], b[1]);
    dst[2] = fminf(a[2], b[2]);
}

inline void float3_max(float dst[3], const float a[3], const float b[3]) {
    dst[0] = fmaxf(a[0], b[0]);
    dst[1] = fmaxf(a[1], b[1]);
    dst[2] = fmaxf(a[2], b[2]);
}

inline void float4_set(float dst[4], float x, float y, float z, float w) {
    dst[0] = x;
    dst[1] = y;
    dst[2] = z;
    dst[3] = w;
}

inline void float4_copy(float dst[4], const float src[4]) {
    dst[0] = src[0];
    dst[1] = src[1];
    dst[2] = src[2];
    dst[3] = src[3];
}

inline float float4_dot(const float a[4], const float b[4]) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3];
}

inline void float4_add(float dst[4], const float a[4], const float b[4]) {
    dst[0] = a[0] + b[0];
    dst[1] = a[1] + b[1];
    dst[2] = a[2] + b[2];
    dst[3] = a[3] + b[3];
}

inline void float4_scale(float dst[4], float v) {
    dst[0] *= v;
    dst[1] *= v;
    dst[2] *= v;
    dst[3] *= v;
}

inline void float4_scaled(float dst[4], const float src[4], float v) {
    dst[0] = src[0] * v;
    dst[1] = src[1] * v;
    dst[2] = src[2] * v;
    dst[3] = src[3] * v;
}

inline void float4_macc(float dst[4], const float src[4], float v) {
    dst[0] = fma(src[0], v, dst[0]);
    dst[1] = fma(src[1], v, dst[1]);
    dst[2] = fma(src[2], v, dst[2]);
    dst[3] = fma(src[3], v, dst[3]);
}

inline void float4_lerp(float dst[4], const float a[4], const float b[4], float t) {
    dst[0] = a[0] * (1 - t) + b[0] * t;
    dst[1] = a[1] * (1 - t) + b[1] * t;
    dst[2] = a[2] * (1 - t) + b[2] * t;
    dst[3] = a[3] * (1 - t) + b[3] * t;
}

inline void float16_copy(float dst[16], const float src[16]) {
    for (int i = 0; i < 16; ++i) {
        dst[i] = src[i];
    }
}

inline void float16_scale(float dst[16], float v) {
    for (int i = 0; i < 16; i++) {
        dst[i] *= v;
    }
}

inline void float16_identity(float dst[16]) {
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

inline void float16_multiply(float dst[16], const float a[16], const float b[16]) {
    dst[0] = float4_dot(a + 0, (float[4]){b[0], b[4], b[8], b[12]});
    dst[1] = float4_dot(a + 0, (float[4]){b[1], b[5], b[9], b[13]});
    dst[2] = float4_dot(a + 0, (float[4]){b[2], b[6], b[10], b[14]});
    dst[3] = float4_dot(a + 0, (float[4]){b[3], b[7], b[11], b[15]});

    dst[0 + 4] = float4_dot(a + 4, (float[4]){b[0], b[4], b[8], b[12]});
    dst[1 + 4] = float4_dot(a + 4, (float[4]){b[1], b[5], b[9], b[13]});
    dst[2 + 4] = float4_dot(a + 4, (float[4]){b[2], b[6], b[10], b[14]});
    dst[3 + 4] = float4_dot(a + 4, (float[4]){b[3], b[7], b[11], b[15]});

    dst[0 + 8] = float4_dot(a + 8, (float[4]){b[0], b[4], b[8], b[12]});
    dst[1 + 8] = float4_dot(a + 8, (float[4]){b[1], b[5], b[9], b[13]});
    dst[2 + 8] = float4_dot(a + 8, (float[4]){b[2], b[6], b[10], b[14]});
    dst[3 + 8] = float4_dot(a + 8, (float[4]){b[3], b[7], b[11], b[15]});

    dst[0 + 12] = float4_dot(a + 12, (float[4]){b[0], b[4], b[8], b[12]});
    dst[1 + 12] = float4_dot(a + 12, (float[4]){b[1], b[5], b[9], b[13]});
    dst[2 + 12] = float4_dot(a + 12, (float[4]){b[2], b[6], b[10], b[14]});
    dst[3 + 12] = float4_dot(a + 12, (float[4]){b[3], b[7], b[11], b[15]});
}

inline void float16_translation(float dst[16], const float src[3]) {
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

inline void float16_transform(float dst[4], const float src[16]) {
    float x = float4_dot(dst, (float[4]){src[0], src[4], src[8], src[12]});
    float y = float4_dot(dst, (float[4]){src[1], src[5], src[9], src[13]});
    float z = float4_dot(dst, (float[4]){src[2], src[6], src[10], src[14]});
    float w = float4_dot(dst, (float[4]){src[3], src[7], src[11], src[15]});
    dst[0] = x;
    dst[1] = y;
    dst[2] = z;
    dst[3] = w;
}

inline void float16_look_at(float dst[16], const float eye[3], const float target[3],
                            const float up[3]) {
    float v3X[3];
    float v3Y[3];
    float v3Z[3];

    float3_copy(v3Y, up);
    float3_normalize(v3Y);

    float3_subtract(v3Z, eye, target);
    float3_normalize(v3Z);

    float3_cross(v3X, v3Y, v3Z);
    float3_normalize(v3X);

    float3_cross(v3Y, v3Z, v3X);

    float4_set(dst + 0, v3X[0], v3Y[0], v3Z[0], 0);
    float4_set(dst + 4, v3X[1], v3Y[1], v3Z[1], 0);
    float4_set(dst + 8, v3X[2], v3Y[2], v3Z[2], 0);
    float4_set(dst + 12,               //
               -float3_dot(v3X, eye),  //
               -float3_dot(v3Y, eye),  //
               -float3_dot(v3Z, eye), 1.0);
}

inline void float16_perspective_y(float dst[16], float fovy_degrees, float aspect_ratio, float near,
                                  float far) {
    const float fovy_radians = fovy_degrees * VEC_PI / 180;
    const float f = tanf(VEC_PI / 2.0f - 0.5f * fovy_radians);
    const float rangeinv = 1.0f / (near - far);
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

inline void float16_perspective_x(float dst[16], float fovy_degrees, float aspect_ratio, float near,
                                  float far) {
    const float fovy_radians = fovy_degrees * VEC_PI / 180;
    const float f = tanf(VEC_PI / 2.0f - 0.5f * fovy_radians);
    const float rangeinv = 1.0f / (near - far);
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

inline void float16_transpose(float dst[16]) {
    float m[16];
    float4_set(m + 0, dst[0], dst[4], dst[8], dst[12]);
    float4_set(m + 4, dst[1], dst[5], dst[9], dst[13]);
    float4_set(m + 8, dst[2], dst[6], dst[10], dst[14]);
    float4_set(m + 12, dst[3], dst[7], dst[11], dst[15]);
    float16_copy(dst, m);
}

inline void float16_invert(float dst[16]) {
    float a00 = dst[0], a01 = dst[1], a02 = dst[2], a03 = dst[3];
    float a10 = dst[4], a11 = dst[5], a12 = dst[6], a13 = dst[7];
    float a20 = dst[8], a21 = dst[9], a22 = dst[10], a23 = dst[11];
    float a30 = dst[12], a31 = dst[13], a32 = dst[14], a33 = dst[15];
    float b00 = a00 * a11 - a01 * a10;
    float b01 = a00 * a12 - a02 * a10;
    float b02 = a00 * a13 - a03 * a10;
    float b03 = a01 * a12 - a02 * a11;
    float b04 = a01 * a13 - a03 * a11;
    float b05 = a02 * a13 - a03 * a12;
    float b06 = a20 * a31 - a21 * a30;
    float b07 = a20 * a32 - a22 * a30;
    float b08 = a20 * a33 - a23 * a30;
    float b09 = a21 * a32 - a22 * a31;
    float b10 = a21 * a33 - a23 * a31;
    float b11 = a22 * a33 - a23 * a32;
    float det = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;
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

inline void float16_rotation_x(float dst[16], float radians) {
    const float s = sinf(radians);
    const float c = cosf(radians);
    float4_set(dst + 0, 1, 0, 0, 0);
    float4_set(dst + 4, 0, c, s, 0);
    float4_set(dst + 8, 0, -s, c, 0);
    float4_set(dst + 12, 0, 0, 0, 1);
}

inline void float16_rotation_y(float dst[16], float radians) {
    const float s = sinf(radians);
    const float c = cosf(radians);
    float4_set(dst + 0, c, 0, -s, 0);
    float4_set(dst + 4, 0, 1, 0, 0);
    float4_set(dst + 8, s, 0, c, 0);
    float4_set(dst + 12, 0, 0, 0, 1);
}

inline void float16_rotation_z(float dst[16], float radians) {
    const float s = sinf(radians);
    const float c = cosf(radians);
    float4_set(dst + 0, c, s, 0, 0);
    float4_set(dst + 4, -s, c, 0, 0);
    float4_set(dst + 8, 0, 0, 1, 0);
    float4_set(dst + 12, 0, 0, 0, 1);
}

inline void float16_rotation(float dst[16], float radians, const float axis[3]) {
    const float s = sinf(radians);
    const float c = cosf(radians);
    const float x = axis[0];
    const float y = axis[1];
    const float z = axis[2];
    const float xy = x * y;
    const float yz = y * z;
    const float zx = z * x;
    const float d = 1.0f - c;
    float4_set(dst + 0, (((x * x) * d) + c), ((xy * d) + (z * s)), ((zx * d) - (y * s)), 0.0f);
    float4_set(dst + 4, ((xy * d) - (z * s)), (((y * y) * d) + c), ((yz * d) + (x * s)), 0.0f);
    float4_set(dst + 8, ((zx * d) + (y * s)), ((yz * d) - (x * s)), (((z * z) * d) + c), 0.0f);
    float4_set(dst + 12, 0, 0, 0, 1);
}

#ifdef TVECTOR_STDIO

#include <stdio.h>

inline void float3_print(FILE* out, const float src[3]) {
    fprintf(out, "[%g %g %g]", src[0], src[1], src[2]);
}

inline void float4_print(FILE* out, const float src[4]) {
    fprintf(out, "[%g %g %g %g]", src[0], src[1], src[2], src[3]);
}

inline void float16_print(FILE* out, const float src[16]) {
    fprintf(out, "[[%g %g %g %g]\n", src[0], src[1], src[2], src[3]);
    fprintf(out, " [%g %g %g %g]\n", src[4], src[5], src[6], src[7]);
    fprintf(out, " [%g %g %g %g]\n", src[8], src[9], src[10], src[11]);
    fprintf(out, " [%g %g %g %g]]\n", src[12], src[13], src[14], src[15]);
}

#endif  // TVECTOR_STDIO
