TARGET      := lib/libfcio.so
SOURCES     := fcio.c time_utils.c
SRC_INCDIRS := ../externals/tmio/src
TGT_LDFLAGS := -shared -Llib
TGT_LDLIBS  := -ltmio -lbufio
TGT_PREREQS := lib/libfcio.a