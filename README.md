# Installation

Run `make` to produce the C header file `include/fcio.h` and the static library `lib/fcio.a`, and it's dependencies `lib/bufio.a` and `lib/tmio.a`.

Run `make prefix=<path-to-install> install` to install header (`$prefix/include`) and libraries (`$prefix/lib/`). `prefix` does not have a default and _needs_ to be set explicitely.


# Contributing

This project is licensed under the Mozilla Public License 2.0, see [LICENSE](LICENSE) for the full terms of use. The MPL
2.0 is a free-software license and we encourage you to feed back any improvements by submitting patches to the upstream
maintainers (see Contact below).

The `Makefile` provided to build this project is licensed under the GNU General Public License 3.0.

# Development

The `tmio` and `bufio` dependencies are pulled into the repository as git submodules.

# Contact

Please send your questions, bug reports or patches via e-mail to fcio-maintainers@mpi-hd.mpg.de.
