TARGET      := lib/libfcio.a
SOURCES     := fcio.c time_utils.c
SRC_INCDIRS := ../externals/tmio/src

define BUILD_DEPENDENCIES

	@echo Building dependency tmio
	@cd $(DIR)/../externals/tmio && $(MAKE)
	@cp -a $(DIR)/../externals/tmio/lib/libtmio.a $(TARGET_DIR)/lib
	@cp -a $(DIR)/../externals/tmio/lib/libbufio.a $(TARGET_DIR)/lib
endef

define MOVE_HEADER

	@mkdir -p $(TARGET_DIR)/include
	@cp -a $(DIR)/fcio.h $(TARGET_DIR)/include
endef

define REMOVE_HEADER

  @rm -f $(TARGET_DIR)/include/fcio.h
	@rm -f $(TARGET_DIR)/lib/libtmio.a
	@rm -f $(TARGET_DIR)/lib/libbufio.a
endef

TGT_POSTMAKE  := ${MOVE_HEADER}
TGT_POSTMAKE  += ${BUILD_DEPENDENCIES}

TGT_POSTCLEAN := ${REMOVE_HEADER}