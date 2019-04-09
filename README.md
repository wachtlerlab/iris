IRIS
====

Toolbox for creating (color) vision experiments. It provides a library and
tools with the aim to create simple visual experiments on color-calibrated
monitors.

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

Usage - Store
-------------

The calibration and configuration data can be stored globally,
`/etc/iris`, or locally per user at `~/.config/iris`. The basic
layout is the following:

	[store root]
	├── cones/                              # cone sensitivies data
	│   └── sml_380@4.csv                   # SML data, starting 380nm, 4nm steps
	├── default.font -> fonts/OpenSans.ttf  # link to default font
	├── default.monitor -> monitors/eDP-1   # link to default monitor
	├── fonts/                              # all fonts
	├── links.cfg                           # output to monitor description
	├── monitors/                           # all known montors
	│   └── sony_gdmf520_77                 # example monitor
	│       ├── 20150602.settings           # description of monitor settings (brightness, …)
	│       ├── 20150602T1807.rgb2lms       # calibration data from spectra (`iris-calibrate`)
	│       ├── 20150608T1549.cac           # calibration data like above, in hdf5 format
	│       ├── sony_gdmf520_77.monitor     # description of the monitor (model, name, …)
	│       └── spectra-20150617T1737.h5    # measured monitor spectra (`iris-measure`)
	├── subjects/                           # registred subjects
	│   └── tsubject                        # subject, "Test Subject" (tsubject)
	│       ├── 20150827T1755.isodata       # measured isoslant data (`iris-isoslant`)
	│       ├── 20150827T1755.isoslant      # isoslant calibration form isodata (`iris-fitiso`)
	│       └── tsubject.subject            # subject description
	└── version                             # store format version, currently "1.0\n"

Most measured data (`*.cac`, `*.isodata`, `*.h5`) are stored in the HDF5
format. The rest of the data is in YAML.

Usage - Calibration
-------------------

We need to blacklist the `cdc-acm` kernel module that otherwise might be
loaded and bind earlier. At least the PR655 needs the `usbserial`. This
can be done by creatin a file called `/etc/modprobe.d/pr665.conf` with
the following contents (see `modprobe.conf(5)`):

	#PR655 should use usbserial
	blacklist cdc-acm

The following set of commands will use the PR655 connected to `/dev/ttyUSB0`
to calibrate the screen of `395 mm` width by `295 mm` height.

	iris-measure --device=/dev/ttyUSB0 calib.stim
	iris-calibrate -W 395 -H 295 spectra-20150814T1522.h5


[glm]: https://glm.g-truc.net/0.9.9/index.html
[glew]: http://glew.sourceforge.net/
[hdf5]: https://www.hdfgroup.org/solutions/hdf5/
[cminpack]: http://devernay.free.fr/hacks/cminpack/
[openblas]: https://www.openblas.net/
[lapack]: http://www.netlib.org/lapack/
[yaml]: https://github.com/jbeder/yaml-cpp

