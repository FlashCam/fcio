TARGET      := lib/libfcio.a
SOURCES     := fcio.c time_utils.c ../${TARGET_DIR}/externals/bufio/src/bufio.c ../${TARGET_DIR}/externals/tmio/src/tmio.c
TGT_INCDIRS := ../${TARGET_DIR}/include ../${TARGET_DIR}/externals/bufio/src ../${TARGET_DIR}/externals/tmio/src
#TGT_LDFLAGS := -L${TARGET_DIR}/lib
#TGT_LDLIBS  := -ltmio -lbufio
#TGT_PREREQS := lib/libbufio.a lib/libtmio.a

define MOVE_HEADER
	@mkdir -p $(TARGET_DIR)/include
	@cp -a $(DIR)/fcio.h $(TARGET_DIR)/include
endef

define REMOVE_HEADER
        @rm -f $(TARGET_DIR)/include/fcio.h
endef

TGT_POSTMAKE  := ${MOVE_HEADER}
TGT_POSTCLEAN := ${REMOVE_HEADER}
