# Installation

Building requires `meson` and `ninja`.

If these are not available (or already shipped) via your OS package manager,
it is possible to install them using `python3 -m pip install --user meson ninja`.

The `Makefile` is acting as a thin wrapper around the usual meson commands:

- `make build`: creates the local build directory, default `build`
- `make clean`: removes the build directory
- `make`,`make compile`: depends on `build` and compiles the library
- `make local`: reconfigures the build directory to install the library `pkg-config` definitions into `${HOME}/.local`
- `make install`: installs the library
- `make uninstall`: uninstalls the installed files defined in the `build` directory
