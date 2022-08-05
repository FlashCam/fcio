# Installation

Run `make` to produce the C header file `include/fcio.h` and the static library `lib/fcio.a`.

# Contributing

This project is licensed under the Mozilla Public License 2.0, see [LICENSE](LICENSE) for the full terms of use. The MPL
2.0 is a free-software license and we encourage you to feed back any improvements by submitting patches to the upstream
maintainers (see Contact below).

The `Makefile` provided to build this project is licensed under the GNU General Public License 3.0.

# Development

The `tmio` and `bufio` dependencies are pulled into the repository as git subtrees, not submodules, to provide an
all-in-one repository to build fcio. Both dependencies are compiled directly into `libfcio.a`.

To update from their origins run `scripts/subtree_pull_origin.sh`; this should only be necessary for developers!

To push commits in the subdirectories upstream run `git subtree push --prefix externals/$repo $repo $branch`. In our
case `$repo` is either `tmio` or `bufio` and `$branch` probably `master`.

# Contact

Please send your questions, bug reports or patches via e-mail to fcio-maintainers@mpi-hd.mpg.de.
