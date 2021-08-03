# Building this demo

This is a demo to test PTAM functionality compiled to WASM. It works with the front-end in this directory's parent directory.

## Emscripten

You will need emscripten installed, see [emscripten.org](https://emscripten.org). You can change the emscripten installation directory in `CMakeLists.txt`.

## OpenCV

You also need opencv.js installed. It has been tested on OpenCV direct clones from the [GitHub repository](https://github.com/opencv/opencv) from early July 2021, including OpenCV 4.5.3.

Note that there are some issues with the installer for the opencv.js build.

- You need to set an `EMSCRIPTEN` environment variable. Set this to the directory containing your Emscripten binaries.

- You also need to edit the Python script used to build OpenCV, see [here](https://github.com/opencv/opencv/issues/19493) as the script is trying to parse CMake arguments as its own arguments.

You need to ensure the correct OpenCV installation directory is present in `CMakeLists.txt`.

CMake 3.12 or higher is needed.

Then to build:
```
emcmake cmake .
emmake make
```
