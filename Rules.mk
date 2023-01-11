CFLAGS       := -std=c99 -O2 -Wall -Wextra -W -pedantic -march=native -fPIC
INCDIRS      := src
SUBMAKEFILES := src/Rules.mk src/Rules-shared.mk

.PHONY: submodules
submodules: 
	git submodule update --init --recursive

# let's hope this redefinition never breaks. I think it's import to note have any
# lines for the recipe here, these would be overwritten
all: submodules

# prefix ?= /usr/local
.PHONY: install
install: all
ifeq ($(prefix),)
	@echo 'prefix variable not set. We require an explicity installation path.'
	@echo 'Rerun with e.g.: make prefix=$${HOME}/.local install'
	@exit 1
endif

	install -d $(DESTDIR)$(prefix)/lib/
	install -m 644 lib/libfcio.a lib/libtmio.a lib/libbufio.a $(DESTDIR)$(prefix)/lib/
	[ -e lib/libfcio.so ] && install -m 644 lib/libfcio.so $(DESTDIR)$(prefix)/lib/
	install -d $(DESTDIR)$(prefix)/include/
	install -m 644 include/fcio.h $(DESTDIR)$(prefix)/include/