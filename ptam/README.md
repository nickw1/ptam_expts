# Log for PTAM web

- 2021-07-08: Started putting together beginnings of PTAM code for the web based on `slam_system.cc` from original PTAM Plus repo. Encountering linker errors at the moment relating to `dgesvd()` and `s_cat()` - it seems that there are inconsistent return type declarations (i.e. void in declaration vs int in implementation or vice-versa). Will try rebuildng ptam with these fixed.

- Rebuilt ptam. Those issues are fixed (in fact they weren't causing the fatal error in the end). This was:

```
[parse exception: attempted pop from empty stack / beyond block start boundary at 122351 (at 0:122351)]
Fatal: error in parsing input
em++: error: '/home/nick/src/emsdk/upstream/bin/wasm-emscripten-finalize --minimize-wasm-changes --dyncalls-i64 ptam_wasm.wasm -o ptam_wasm.wasm --detect-features' failed (returned 1)
```

Not sure why this is happening...
