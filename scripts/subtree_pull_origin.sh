#!/bin/sh
echo "Pulling changes from bufio/master."
git subtree pull -P externals/bufio bufio master --squash
echo "Pulling changes from tmio/master."
git subtree pull -P externals/tmio tmio master --squash
