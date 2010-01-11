ifeq (src,$(notdir $(CURDIR)))
FF_RELS = 3.0 3.5 3.6
all:
	mkdir -pv $(FF_RELS)
	for release in $(FF_RELS); do \
        cd $$release && make -f ../Makefile FF_REL=$$release; cd ..; \
    done
clean:
	rm -fr $(FF_RELS)
else
ifeq ($(strip $(FF_REL)),)
    all:
	@echo FF_REL should be set when not in 'src' directory
	@false
else
VPATH = ..
UNAME = $(shell (uname -o 2>/dev/null||uname -s 2>/dev/null)|tr 'A-Z' 'a-z')
HEADERS = IBFDNSSDService.h CBFDNSSDService.h
TYPELIB = IBFDNSSDService.xpt IBFServiceTracker.xpt
ifeq ($(UNAME),darwin)
    include ../$(UNAME).inc
else
ifeq ($(UNAME),cygwin)
    include ../$(UNAME).inc
else
    @echo This script won't build on this platform
    @false
endif
endif
endif
endif