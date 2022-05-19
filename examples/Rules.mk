TGT_PREREQS  := lib/libtmio.a lib/libbufio.a
TGT_LDFLAGS  := -L${TARGET_DIR}/lib
TGT_LDLIBS   := -ltmio -lbufio
SUBMAKEFILES := $(shell ls ${DIR}/tmio_*.mk | sed s,^${DIR}/,,g)
