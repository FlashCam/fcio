TARGET      := lib/libtmio.a
SOURCES     := tmio.c
TGT_INCDIRS := ../${TARGET_DIR}/include
TGT_LDFLAGS := -L${TARGET_DIR}/lib
TGT_LDLIBS  := -lbufio
TGT_PREREQS := lib/libbufio.a

define MOVE_HEADER
	@mkdir -p $(TARGET_DIR)/include
	@cp -a $(DIR)/tmio.h $(TARGET_DIR)/include
endef

define REMOVE_HEADER
        @rm -f $(TARGET_DIR)/include/tmio.h
endef

TGT_POSTMAKE  := ${MOVE_HEADER}
TGT_POSTCLEAN := ${REMOVE_HEADER}
