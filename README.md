This repo contains demos and docs for the [par_camera_control.h](https://github.com/prideout/par),
a one-file C library that enables orbit controls (a.k.a. tumble, arcball, trackball) or pan-and-zoom
like Google Maps.

To view the documentation and play with WebAssembly demos, go [here]().

The native demo is easy to build on macOS. First make sure you have CMake and clang installed, then
do `make run`. For other platforms, simply invoke CMake in the way that you normally do.

# Tweet

I wrote a camera controller as a single-file C library. Supports smooth Van Wijk zoom & pan with
perspective correction. Also has an orbit mode for sketchfab-style control. Docs and wasm demos at
https://github.com/prideout/camera_demo

# Notes on Orbit mode

When the controller is in orbit mode, the orientation of the camera is defined by a Y-axis
rotation followed by an X-axis rotation.

φ :: X axis rotation :: constrained   :: applied first  :: how far from equator
θ :: Y axis rotation :: unconstrained :: applied second :: how far from prime meridian

If home_vector is (0,0,1) then eyepos works like this:

PLEASE VERIFY WITH PYTHON THAT THE FOLLOWING IS ALL TRUE

x = sin(θ) * cos(φ)
y = sin(φ)          // the "axis" of planet is the Y axis
z = cos(θ) * cos(φ) // because we want Z=1 when θ=φ=0

θ = atan2(x,z)
φ = asin(y)

# TODO for Orbit mode

- get_current, goto_frame

- no sphere, just simple rotation about the center point
    - dragging both buttons "strafes"; this translates the rotation center as well
    - scrolling moves fwd / backwd along the view axes, and does not affect the rotation center

# Demo ideas

- change mouse cursor during pan / zoom
    - CSS cursors: grab default, grabbing for pan, all-scroll for strafe, crosshair for zoom

- two emscripten demos (no microui stuff, probably)

# Docs branch

- gifs
    - modes...shows two viewports: orbit and map
    - raycasting...shows two viewports: orbit and map
    - FOV orientation...shows two viewports resizing
    - animation: two viewports: Van Wijk interpolation, slerp
- two inline web demos
- one fullscreen demo
- pandoc (maybe don't bother with pypandoc)
- api reference

# Longer term ideas

- Multi-touch support, especially for zoom.
- First person mode (sketchfab has this)
- Demonstrate precision crosshairs for deep zoom, similar to my old blog post.
- Make a demo that has a "view cube" as seen in tinkercad. See `ImGuizmo` for inspiration.

# Notes

```
JuliaComputing/llvm-cbe

BLINN, J. 1988. Where am I? What am I looking at? In IEEE Computer Graphics and Applications. Vol. 22.
179–188.

GLEICHER, M. AND WITKIN, A. 1992. Through-the-lens camera control. In Siggraph, E. E. Catmull, Ed.
Vol. 26. 331–340. ISBN 0-201-51585-7. Held in Chicago, Illinois.

https://github.com/dougbinks/enkiTS
```
