# Log for PTAM web

- 2021-07-08: Started putting together beginnings of PTAM code for the web based on `slam_system.cc` from original PTAM Plus repo. Encountering linker errors at the moment relating to `dgesvd()` and `s_cat()` - it seems that there are inconsistent return type declarations (i.e. void in declaration vs int in implementation or vice-versa). Will try rebuildng ptam with these fixed.
