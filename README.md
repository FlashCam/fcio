# Install

Run `make` to build the fcio library. This will produce header files in `include` and libraries in `lib`.

# Development

The `tmio` and `bufio` dependencies are pulled into the repository as git subtrees, not submodules, to provide an all-in-one repository to build fcio.
Both dependencies are not built separately added directly to the compilation of fcio.

To update from their origins run `scripts/subtree_pull_origin.sh`; this should only be necessary for developers!

To push commits in the subdirectories upstream run `git subtree push --prefix externals/$repo $repo $branch`. In our case `$repo` is either `tmio` or `bufio` and `$branch` probably `master`.
