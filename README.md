IRIS
====

Building
--------

Dependencies:

 - OpenGL (e.g. mesa)
 - [OpenGL Mathematics][glm]
 - [glew][glew]
 - [HDF5][hdf5]
 - [C/C++ Minpack][cminpack]
 - [OpenBLAS][openblas]
 - [LAPACK][lapack]
 - [yaml cpp][yaml]
 - freetype
 - cmake
 - ninja (if you wanna build with ninja)

Install dependencies in Fedora:

	dnf install glm-devel hdf5-devel cminpack-devel openblas-devel \
	            yaml-cpp-devel hdf5-devel libglvnd-devel \
				freetype-devel cmake ninja-build

Configuring and building:

	mkdir build
	cd build
	cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX:PATH=/usr -G Ninja ..
	ninja



[glm]: https://glm.g-truc.net/0.9.9/index.html
[glew]: http://glew.sourceforge.net/
[hdf5]: https://www.hdfgroup.org/solutions/hdf5/
[cminpack]: http://devernay.free.fr/hacks/cminpack/
[openblas]: https://www.openblas.net/
[lapack]: http://www.netlib.org/lapack/
[yaml]: https://github.com/jbeder/yaml-cpp

