#define SOKOL_GLCORE33
#define SOKOL_IMPL
#define PARSH_ENABLE_STDIO
#define PAR_CAMERA_CONTROL_IMPLEMENTATION
#define PAR_SHADERS_IMPLEMENTATION
#define PAR_MSQUARES_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define NANO_RT_C_IMPLEMENTATION

#include <par/par_shaders.h>
#include <par/par_msquares.h>
#include <par/par_camera_control.h>

#include <sokol/sokol_app.h>
#include <sokol/sokol_gfx.h>
#include <sokol/sokol_time.h>

#include <stb/stb_image.h>
#include <stb/stb_image_resize.h>

#include <nanort/nanort_c.h>
