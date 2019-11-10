This is a demo for the `par_camera_control.h` library.

Easy to build on macOS. First make sure you have CMake and clang installed, then do `make run`.

# TODO

- map mode
    - the "aabb vs mesh" checkbox can be implemented
        - implement the nanort C interface (nanort_c.h)
    - pan and zoom
    - grabbing mountain tops should work
    - Ensure that maps need not be on the Z plane. (working_plane)
    - Provide a stock plane intersection function. (or aabb)
    - navigation and animation (frames)
    - zoom and pan constraints (make it optional, add a checkbox)
    - proper home position

- orbit mode
- "show frame"
- two emscripten demos (no microui stuff, probably)
- multi-touch to zoom

# Longer term ideas

- Consider adding FREE mode that changes the look-at vectors directly (forward / backward etc)
- Demonstrate precision crosshairs for deep zoom, similar to my old blog post.
- Tinkercad style cube the style of ImGuizmo.

# Notes

```
JuliaComputing/llvm-cbe

BLINN, J. 1988. Where am I? What am I looking at? In IEEE Computer Graphics and Applications. Vol. 22.
179–188.

GLEICHER, M. AND WITKIN, A. 1992. Through-the-lens camera control. In Siggraph, E. E. Catmull, Ed.
Vol. 26. 331–340. ISBN 0-201-51585-7. Held in Chicago, Illinois.
```
