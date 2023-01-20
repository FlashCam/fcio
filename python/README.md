# Installation

## Requirements for local installation from source

`pip install build`

Run `make` to build the source distribution package, located in `dist`.

Run `make install` to install the sdist using pip.

# Usage

The library provides scripts as examples, located in `src/fcio/cmds/cmds.py`.
Most useful as a quick entry is the script `fcio-plot-events` which plots the raw traces using matplotlib.


# Contributing

This project is licensed under the Mozilla Public License 2.0, see [LICENSE](LICENSE) for the full terms of use. The MPL
2.0 is a free-software license and we encourage you to feed back any improvements by submitting patches to the upstream
maintainers (see Contact below).

The `Makefile` provided to build this project is licensed under the GNU General Public License 3.0.

# Development

Development is best done in a local environment, e.g. using `venv`

```
# create local environment in env directory. venv need to be installed
python3 -m venv env

# activate the environment
source env/bin/activate

# this step is crucial. We copy the fcio source files to the local directory.
# This needs to be rerun, whenever changes in the parent library are made
make gather_source_files 

# install the library in editable mode
python3 -m pip install -e .
```


# Contact

Please send your questions, bug reports or patches via e-mail to fcio-maintainers@mpi-hd.mpg.de.
