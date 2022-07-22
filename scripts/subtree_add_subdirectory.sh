#!/bin/sh
echo "Adding bufio repository as subdirectory 'externals/bufio'"
git subtree add --prefix externals/bufio bufio master --squash
echo "Adding tmio repository as subdirectory 'externals/tmio'"
git subtree add --prefix externals/tmio bufio master --squash
