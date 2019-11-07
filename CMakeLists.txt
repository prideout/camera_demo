cmake_minimum_required(VERSION 3.9)

project(camera_demo)

set(NAME camera_demo)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")
endif()

add_definitions(-DTVECTOR_STDIO)

set(SRCS
        extras/nanort/nanort.cc
        extras/nanort/nanort.h
        extras/microui/microui.c
        extras/microui/microui.h
        src/app.h
        src/app.c
        src/gui.h
        src/gui.c
        src/platform.m
        src/ray_float.c
        src/ray_float.h
        src/demo.c
        src/vec_double.c
        src/vec_double.h
        src/vec_float.c
        src/vec_float.h)

add_executable(${NAME} ${SRCS})

target_include_directories(${NAME} PRIVATE "extras")

# Uncomment the following lines to enable address sanitizer.
#set(ASAN_CFLAGS -fsanitize=undefined -fsanitize=address -fstack-protector)
#set(ASAN_LINKFLAGS ${ASAN_CFLAGS} -lstdc++)

target_compile_options(${NAME} PRIVATE -fobjc-arc -Wpedantic ${ASAN_CFLAGS})
target_link_options(${NAME} PRIVATE ${ASAN_LINKFLAGS})

set_target_properties(${NAME} PROPERTIES LINK_FLAGS "-Wl,-F/Library/Frameworks")

target_link_libraries(${NAME} PRIVATE
    "-framework Foundation"
    "-framework Cocoa"
    "-framework AppKit"
    "-framework OpenGL"
    objc
    m)
