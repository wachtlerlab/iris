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

Usage - Tools
-------------

| Command            | Description                                                |
| ----------------   | ---------------------------------------------------------- |
| `pr655`            | interface with the PR655 spectrometer                      |
| `iris-measure`     | use calibration stimuli to measure spectra                 |
| `iris-calibrate`   | use measured spectra to generated calibration              |
| `iris-cgen`        | generate color (`-r, -g, -b`) or white (`-w`) stimuli, in  |
|                    | increments of N steps (`-N`) that repeat for (`-B`) blocks |
| `iris-info`        | Show information about known monitors                      |
| `iris-store`       | Show store information                                     |
| `iris-colorcircle` | Display a (calibrated) colorcircle                         |
| `iris-board`       | Display an animated (and color calibrated checkerboard     |
| `iris-isoslant`    | Measure iso-slant data for a single test subject           |
| `iris-fitiso`      | Use iso-slant data to generate per-subject calibration     |



[glm]: https://glm.g-truc.net/0.9.9/index.html
[glew]: http://glew.sourceforge.net/
[hdf5]: https://www.hdfgroup.org/solutions/hdf5/
[cminpack]: http://devernay.free.fr/hacks/cminpack/
[openblas]: https://www.openblas.net/
[lapack]: http://www.netlib.org/lapack/
[yaml]: https://github.com/jbeder/yaml-cpp

