This is a demo for the `par_camera_control.h` library.

Easy to build on macOS. First make sure you have CMake and clang installed, then do `make run`.

# TODO

- map mode
    - use ray_float to write raycaster against the Z=0 plane
    - pan and zoom
    - implement the nanort C interface (nanort_c.h)
    - navigation and animation (frames)
    - bounds
    - proper home position
    - proper raycasting
- orbit mode
- "show frame"
- multi-touch to zoom

# Notes

JuliaComputing/llvm-cbe

Screen-space Camera Constaints

BLINN, J. 1988. Where am I? What am I looking at? In IEEE Computer Graphics and Applications. Vol. 22.
179–188.

GLEICHER, M. AND WITKIN, A. 1992. Through-the-lens camera control. In Siggraph, E. E. Catmull, Ed.
Vol. 26. 331–340. ISBN 0-201-51585-7. Held in Chicago, Illinois.

