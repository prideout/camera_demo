This repo contains demos and docs for the [par_camera_control.h](https://github.com/prideout/par),
a one-file C library that enables orbit controls (a.k.a. tumble, arcball, trackball) or pan-and-zoom
like Google Maps.

The native demo is easy to build on macOS. First make sure you have CMake and clang installed, then
do `make run`. For other platforms, simply invoke CMake in the way that you normally do.

# TODO for Orbit mode

- implement strafing (translates the orbit_pivot too)
- interpolate_frame for orbit mode

# Longer term ideas

- Multi-touch support, especially for zoom.
- Momentum for panning
    - Also, sudden movements should be smoothed.
    - This could be a time-aware utility class (par_camera_animator)
        - parca_grab_begin, parca_grab_end, parca_grab_begin, parca_zoom, parca_tick)
- Double-tap to refocus (might be a parca feature)
- First person mode (sketchfab has this)
- Demonstrate precision crosshairs for deep zoom, similar to my old blog post.
- Make a demo that has a "view cube" as seen in tinkercad. See `ImGuizmo` for inspiration.
