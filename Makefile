BUILDDIR:=build

.PHONY: setup compile install uninstall clean

all: compile

$(BUILDDIR):
	meson setup $(BUILDDIR)

clean:
	@rm -r $(BUILDDIR)

compile: $(BUILDDIR)
	meson compile -C $(BUILDDIR)

local: $(BUILDDIR)
	meson configure -Dprefix=${HOME}/.local $(BUILDDIR)

install: compile
	meson install -C $(BUILDDIR)

uninstall:
	cd $(BUILDDIR) && meson --internal uninstall
