TARGET  := bin/tmio_benchmark
SOURCES := tmio_benchmark.c timer.c

define MOVE_SCRIPTS
	@cp -a $(DIR)/tmio_benchmark_*.sh $(TARGET_DIR)/bin
endef

define REMOVE_SCRIPTS
	@rm -f $(TARGET_DIR)/bin/tmio_benchmark_*.sh
endef

TGT_POSTMAKE  := ${MOVE_SCRIPTS}
TGT_POSTCLEAN := ${REMOVE_SCRIPTS}
