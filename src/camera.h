#pragma once

typedef enum { CAMERA_FOV_VERTICAL, CAMERA_FOV_HORIZONTAL } CameraFov;

typedef struct CameraImpl Camera;

Camera* camera_create();
void camera_destroy(Camera*);

void camera_set_aspect(Camera*, double aspect);
double camera_get_aspect(const Camera*);

void camera_look_atf(Camera*, const float* eye3, const float* target3, const float* up3);
void camera_look_atd(Camera*, const double* eye3, const double* target3, const double* up3);

void camera_perspective(Camera*, CameraFov, double degrees, double near, double far);

void camera_get_view_matrixf(const Camera*, float* matrix16);
void camera_get_view_matrixd(const Camera*, double* matrix16);

void camera_get_projection_matrixf(const Camera*, float* matrix16);
void camera_get_projection_matrixd(const Camera*, double* matrix16);

void camera_set_node_transformf(Camera*, const float* transform16);
void camera_set_node_transformd(Camera*, const double* transform16);

void camera_get_node_transformf(const Camera*, float* transform16);
void camera_get_node_transformd(const Camera*, double* transform16);

void camera_get_positionf(const Camera*, float* eye3);
void camera_get_positiond(const Camera*, double* eye3);

double camera_get_fovy_degrees(const Camera*);
double camera_get_fovy_radians(const Camera*);
