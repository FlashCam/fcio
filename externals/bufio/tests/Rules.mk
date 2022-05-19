TGT_LDFLAGS  := -L${TARGET_DIR}/lib
TGT_LDLIBS   := -lbufio
TGT_PREREQS  := lib/libbufio.a
SUBMAKEFILES := $(shell ls ${DIR}/bufio_test_*.mk | sed s,^${DIR}/,,g)
