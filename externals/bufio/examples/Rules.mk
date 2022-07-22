TARGET      := bin/bufio_benchmark
SOURCES     := bufio_benchmark.c
TGT_PREREQS := lib/libbufio.a
TGT_LDFLAGS := -L${TARGET_DIR}/lib
TGT_LDLIBS  := -lbufio
