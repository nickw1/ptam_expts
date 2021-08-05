# Building this demo

This is a demo to test PTAM functionality compiled to WASM. It works with the front-end in this directory's parent directory.

## Emscripten

You will need emscripten installed, see [emscripten.org](https://emscripten.org). You can change the emscripten installation directory in `CMakeLists.txt`.

## Which PTAM version?

You will need the specific version of PTAM [here](https://github.com/nickw1/ptam_plus/tree/ptam_emscripten_fix), i.e. the `ptam_emscripten_fix` branch. This was forked from Thorsten Bux's [fork](https://github.com/ThorstenBux/ptam_plus) of [PTAM Plus](https://github.com/williammc/ptam_plus). It's been fixed so it compiles on Emscripten and also has some additional functionality added to work better with a web front end.

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
