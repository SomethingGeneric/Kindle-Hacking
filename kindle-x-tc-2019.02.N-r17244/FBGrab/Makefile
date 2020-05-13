###
### I think it's not worth it to make such a small project
### modular. So this is a simple gnu Makefile...
###
### $Id: Makefile 16102 2019-07-08 04:45:15Z NiLuJe $
###

# Pickup our TCs
ifdef CROSS_TC
	CC:=$(CROSS_TC)-gcc
	CXX:=$(CROSS_TC)-g++
	STRIP:=$(CROSS_TC)-strip
	# NOTE: This relies on GCC plugins! Enforce AR & RANLIB to point to their real binary and not the GCC wrappers if your TC doesn't support that!
	AR:=$(CROSS_TC)-gcc-ar
	RANLIB:=$(CROSS_TC)-gcc-ranlib
else
	CC?=gcc
	CXX?=g++
	STRIP?=strip
	AR?=gcc-ar
	RANLIB?=gcc-ranlib
endif

GZIP:=gzip
GZIPFLAGS:=--best --to-stdout

# Build version string w/ the SVN revision of the file...
SVNREV:=-DVERSION='"1.3.$(shell sed -ne '/Revision:/p' fbgrab.c | sed -re 's/(.*?)(Revision: )([[:digit:]]*)(.*?)/\3/').N"'

# All the warnings!
EXTRA_CFLAGS+=-Wall
EXTRA_CFLAGS+=-Wextra -Wunused
EXTRA_CFLAGS+=-Wformat=2
EXTRA_CFLAGS+=-Wformat-signedness
EXTRA_CFLAGS+=-Wformat-truncation=2
EXTRA_CFLAGS+=-Wnull-dereference
EXTRA_CFLAGS+=-Wuninitialized
EXTRA_CFLAGS+=-Wduplicated-branches -Wduplicated-cond
EXTRA_CFLAGS+=-Wundef
EXTRA_CFLAGS+=-Wbad-function-cast
EXTRA_CFLAGS+=-Wwrite-strings
EXTRA_CFLAGS+=-Wjump-misses-init
EXTRA_CFLAGS+=-Wlogical-op
EXTRA_CFLAGS+=-Wstrict-prototypes -Wold-style-definition
EXTRA_CFLAGS+=-Wshadow
EXTRA_CFLAGS+=-Wmissing-prototypes -Wmissing-declarations
EXTRA_CFLAGS+=-Wnested-externs
EXTRA_CFLAGS+=-Winline
EXTRA_CFLAGS+=-Wcast-qual
# NOTE: GCC 8 introduces -Wcast-align=strict to warn regardless of the target architecture (i.e., like clang)
EXTRA_CFLAGS+=-Wcast-align
EXTRA_CFLAGS+=-Wconversion

# Detect the K3 TC to disable CLOEXEC...
ifdef CROSS_TC
	ifeq "$(CROSS_TC)" "arm-kindle-linux-gnueabi"
		EXTRA_CPPFLAGS+=-DKINDLE_LEGACY
	endif
endif

all: fbgrab fbgrab.1.gz

fbgrab: fbgrab.c
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(SVNREV) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) $< -lpng -lz -o $@

static: fbgrab.c
	$(CC) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(SVNREV) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) $< -l:libpng.a -l:libz.a -o fbgrab
	$(STRIP) --strip-unneeded fbgrab

fbgrab.1.gz: fbgrab.1.man
	$(GZIP) $(GZIPFLAGS) $< > $@

install: fbgrab fbgrab.1.gz
	install -D -m 0755 fbgrab $(DESTDIR)/usr/bin/fbgrab
	install -D -m 0644 fbgrab.1.gz $(DESTDIR)/usr/share/man/man1/fbgrab.1.gz

clean:
	-$(RM) fbgrab fbgrab.1.gz *~ \#*\#

.DELETE_ON_ERROR:
.PHONY: install clean all static
