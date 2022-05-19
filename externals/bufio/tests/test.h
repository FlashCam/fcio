#include <stdio.h>

#define assert(e) if (!(e)) { fprintf(stderr, "Assertion failed: %s, function %s, file %s, line %d.\n", #e, __func__, __FILE__, __LINE__); _exit(1); }

// Three macros to simplify creation of forks and to check for a clean exit of the child process; use in this order
#define FORK_CHILD if (fork() == 0) {
#define FORK_PARENT _exit(0); } else {
#define FORK_JOIN int status; wait(&status); assert(WIFEXITED(status)); if (WEXITSTATUS(status) != 0) return 1; }
