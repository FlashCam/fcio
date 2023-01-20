VERSION := $(shell python3 -c 'import src.fcio; print(src.fcio.__version__)')
CSOURCES := ..
CTARGET  := src/fcio
.PHONY: dist clean install gather all
all: dist

gather:
	mkdir -p $(CTARGET)/include $(CTARGET)/src
	cp -a $(CSOURCES)/src/fcio.c $(CSOURCES)/src/time_utils.c $(CSOURCES)/externals/tmio/src/tmio.c $(CSOURCES)/externals/tmio/externals/bufio/src/bufio.c $(CTARGET)/src
	cp -a $(CSOURCES)/externals/tmio/externals/bufio/src/bufio.h  $(CSOURCES)/externals/tmio/src/tmio.h $(CSOURCES)/src/time_utils.h $(CSOURCES)/src/fcio.h $(CTARGET)/include

clean:
	rm -r $(CTARGET)/include $(CTARGET)/src dist src/fcio.egg-info

dist/fcio-$(VERSION).tar.gz: gather
	python3 -m build --sdist

dist: dist/fcio-$(VERSION).tar.gz

install: dist/fcio-$(VERSION).tar.gz
	python3 -m pip install dist/fcio-$(VERSION).tar.gz