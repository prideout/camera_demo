.release:
	cmake -H. -B.release -DCMAKE_BUILD_TYPE=Release
	cmake --build .release -- -j

.debug:
	cmake -H. -B.debug -DCMAKE_BUILD_TYPE=Debug
	cmake --build .debug

run: .release
	cmake --build .release -- -j
	.release/camera_demo

lldb: .debug
	cmake --build .debug -- -j
	lldb -o run .debug/camera_demo

run_debug: .debug
	cmake --build .debug -- -j
	.debug/camera_demo

clean:
	rm -rf .release .debug
