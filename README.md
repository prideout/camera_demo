This is a demo for [par_camera_control.h](https://github.com/prideout/par),
a one-file C library that enables orbit controls (a.k.a. tumble, arcball, trackball) or pan-and-zoom
like Google Maps.

The native demo is easy to build on macOS. First make sure you have CMake and clang installed, then
do `make run`. For other platforms, simply invoke CMake in the way that you normally do.

<img src='https://github.com/prideout/camera_demo/blob/master/extras/screenshot.png'>

## Notes

- Ideas for par_camera_controller
    - Multi-touch support, especially for zoom.
    - Double-tap to focus.
    - First person mode (sketchfab has this).

- Create par_camera_animator.h
    - Time-aware utility class.
    - Provides momentum for panning.
    - Sudden movements should be smoothed.
    - parca_grab_begin, parca_grab_end, parca_grab_begin, parca_zoom, parca_tick.
    - does not depend on par_camera_control, uses callbacks instead.

- Demo ideas
    - Demonstrate precision crosshairs for deep zoom, similar to my old blog post.
    - Implement a view cube as seen in tinkercad. See `ImGuizmo` for inspiration.
