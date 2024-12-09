# my-acg-project

## Creating demos

Use the following commands to initialize.
```
git clone <link-to-this-repo>
git submodule init
mkdir build
cd build
cmake --build . --config release
```

Produce `bouncing_spheres.ppm` by `.\src\Release\render.exe -outfile output.ppm -demo 1`, if too slow, modify the parameters in `main.cpp/bouncing_spheres()`.

Produce `earth.ppm` by `.\src\Release\render.exe -outfile output.ppm -demo 3`.
