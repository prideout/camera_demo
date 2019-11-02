#include "camera.h"
#include "vec_double.h"

#include <stdlib.h>

struct CameraImpl {
    double aspect;
    double node_transform[16];
    double projection[16];
};

Camera* camera_create() {
    Camera* retval = malloc(sizeof(struct CameraImpl));
    retval->aspect = 1.0;
    double16_identity(retval->node_transform);
    double16_identity(retval->projection);
    return retval;
}

void camera_destroy(Camera* cam) { free(cam); }

void camera_set_aspect(Camera* cam, double aspect) { cam->aspect = aspect; }
double camera_get_aspect(const Camera* cam) { return cam->aspect; }

void camera_look_atf(Camera* cam, const float* eye3, const float* target3, const float* up3) {
    double eyepos[3] = {eye3[0], eye3[1], eye3[2]};
    double target[3] = {target3[0], target3[1], target3[2]};
    double upward[3] = {up3[0], up3[1], up3[2]};
    camera_look_atd(cam, eyepos, target, upward);
}

void camera_look_atd(Camera* cam, const double* eye3, const double* target3, const double* up3) {
    double16_look_at(cam->node_transform, eye3, target3, up3);
    double16_invert(cam->node_transform);
}

void camera_perspective(Camera* cam, CameraFov fov, double degrees, double near, double far) {
    if (fov == CAMERA_FOV_VERTICAL) {
        double16_perspective_y(cam->projection, degrees, cam->aspect, near, far);
    } else {
        double16_perspective_x(cam->projection, degrees, cam->aspect, near, far);
    }
}

void camera_get_view_matrixf(const Camera* cam, float* matrix16) {
    double retval[16];
    camera_get_view_matrixd(cam, retval);
    float16_cast(matrix16, retval);
}

void camera_get_view_matrixd(const Camera* cam, double* matrix16) {
    double16_copy(matrix16, cam->node_transform);
    double16_invert(matrix16);
}

void camera_get_projection_matrixf(const Camera* cam, float* matrix16) {
    float16_cast(matrix16, cam->projection);
}

void camera_get_projection_matrixd(const Camera* cam, double* matrix16) {
    double16_copy(matrix16, cam->projection);
}

void camera_set_node_transformf(Camera* cam, const float* transform16) {
    double16_cast(cam->node_transform, transform16);
}

void camera_set_node_transformd(Camera* cam, const double* transform16) {
    double16_copy(cam->node_transform, transform16);
}

void camera_get_node_transformf(const Camera* cam, float* transform16) {
    float16_cast(transform16, cam->node_transform);
}

void camera_get_node_transformd(const Camera* cam, double* transform16) {
    double16_copy(transform16, cam->node_transform);
}

void camera_get_positionf(const Camera* cam, float* eye3) {
    eye3[0] = cam->node_transform[12];
    eye3[1] = cam->node_transform[13];
    eye3[2] = cam->node_transform[14];
}

void camera_get_positiond(const Camera* cam, double* eye3) {
    eye3[0] = cam->node_transform[12];
    eye3[1] = cam->node_transform[13];
    eye3[2] = cam->node_transform[14];
}

double camera_get_fovy_radians(const Camera* cam) {
    const float f = cam->projection[5];
    return 2.0 * (VEC_PI / 2.0f - atan(f));
}

double camera_get_fovy_degrees(const Camera* cam) {
    return camera_get_fovy_radians(cam) * 180.0 / VEC_PI;
}
