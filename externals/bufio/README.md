# bufio

This library provides a thin buffering layer on top of the unified Unix I/O model. It connects a TCP stream (as server
or client), Unix standard streams, or files and buffers all I/O operations. An interface very similar to stdio has been
implemented with additional support for polling.

## Installation

Run `make` to produce the C header file `include/bufio.h` and the static library `lib/libbufio.a`.

Run `make check` to run unit tests.

## Contributing

This project is licensed under the Mozilla Public License 2.0, see [LICENSE](LICENSE) for the full terms of use. The MPL
2.0 is a free-software license and we encourage you to feed back any improvements by submitting patches to the upstream
maintainers (see Contact below).

The `Makefile` provided to build this project is licensed under the GNU General Public License 3.0.

## Contact

Please send your questions, bug reports or patches via e-mail to felix.werner@mpi-hd.mpg.de.
