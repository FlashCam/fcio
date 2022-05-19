TARGET  := lib/libbufio.a
SOURCES := bufio.c

define MOVE_HEADER
	@mkdir -p $(TARGET_DIR)/include
	@cp -a $(DIR)/bufio.h $(TARGET_DIR)/include
endef

define REMOVE_HEADER
	@rm -f $(TARGET_DIR)/include/bufio.h
endef

TGT_POSTMAKE  := ${MOVE_HEADER}
TGT_POSTCLEAN := ${REMOVE_HEADER}
