This repo contains demos and docs for the [par_camera_control.h](https://github.com/prideout/par),
a one-file C library that enables orbit controls (a.k.a. tumble, arcball, trackball) or pan-and-zoom
like Google Maps.

The native demo is easy to build on macOS. First make sure you have CMake and clang installed, then
do `make run`. For other platforms, simply invoke CMake in the way that you normally do.

# TODO for Orbit mode

- phi constraint is not enforced
- scrolling moves fwd / backwd along the view axes, and does not affect the rotation center
- dragging both buttons "strafes"; this translates the rotation center as well
- interpolate_frame for orbit mode
- refactor magic numbers 100.0 into property: orbit_speed[2]

# Longer term ideas

- Multi-touch support, especially for zoom.
- Momentum for panning (this might be implemented in the demo rather than the library)
- First person mode (sketchfab has this)
- Demonstrate precision crosshairs for deep zoom, similar to my old blog post.
- Make a demo that has a "view cube" as seen in tinkercad. See `ImGuizmo` for inspiration.
