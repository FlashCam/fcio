# tmio

This library provides a tagged message I/O model on top of TCP streams, Unix
standard streams, or files with and without locking.

A message is composed of a tag, specifying the type of the message, and one or
more data payloads of known size. A writer issues `tmio_write_tag` followed by
any number of calls to `tmio_write_data`. A reader typically uses `tmio_read_tag`
to skip to the next tag, followed by any number of calls to `tmio_read_data`.
For fine-grained polling, `tmio_wait` can be used to wait for the next tag with
a specified timeout.

Any stream errors or protocol timeouts will make the stream inoperable. A
protocol timeout occurs in the following conditions:

 1. a write operation (`tmio_write_tag`, `tmio_write_data`, `tmio_flush`,
    `tmio_sync`) timed out
 2. `tmio_read_data` timed out

To make sure that a reader does not run into condition (2), a writer must
issue `tmio_flush` within the protocol timeout after writing one or more tags
and data frames. The protocol timeout is initially set when creating the
context with `tmio_init` and can be changed at any time with `tmio_timeout`.

The stream is made inoperable to (a) force the application to actively handle
stream errors and (b) to simplify application code, as the user is able to
group a number of associated operations and check for errors afterwards. It is
the application's responsibility to resynchronise with the other end after a
stream error or protocol timeout.

The error handling of tmio is generally POSIX-like: if an error occurs, a
function returns either `NULL` (when returning a pointer) or -1. A status code
and description can then be retrieved with `tmio_status` and `tmio_status_str`,
similar to `errno` and `strerror(3)` but permanently associated with the stream
context. Success is indicated with a valid pointer or an integer >=0 (e.g.,
`tmio_write_data` and `tmio_read_data` return the number of bytes written/read).
