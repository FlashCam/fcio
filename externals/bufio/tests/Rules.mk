TGT_LDFLAGS  := -L${TARGET_DIR}/lib
TGT_LDLIBS   := -lbufio
TGT_PREREQS  := lib/libbufio.a
SUBMAKEFILES := $(shell ls ${DIR}/bufio_test_*.mk | sed s,^${DIR}/,,g)

TESTS := $(shell ls ${DIR}/bufio_test_*.mk | sed s,^${DIR}/,bin/,g | sed s,\.mk,,g)
SKIPPED_TESTS := bufio_test_follow_chunk  # needs manual Ctrl+C

check: ${TESTS}
	@echo "Running tests:"
	@for TEST in $(TESTS); do \
		[[ $$TEST =~ $(SKIPPED_TESTS) ]] && continue; \
		printf -- "- $$TEST... "; \
		$$TEST && echo "pass" || exit 1; \
	done \
