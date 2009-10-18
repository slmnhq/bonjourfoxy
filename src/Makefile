CC = g++

UNAME  = $(shell uname -s 2>/dev/null | tr 'A-Z' 'a-z' || echo not)
DISTID = $(shell lsb_release -is 2>/dev/null | tr 'A-Z' 'a-z' | sed 's/\ /_/g' || echo not)

ifeq ($(UNAME),darwin)
    include $(UNAME).inc
else
    include $(DISTID).inc
endif

HEADERS = bfdnssdbrowsecallback.h bfdnssdenumeratecallback.h \
          bfdnssdregistercallback.h bfdnssdresolvecallback.h bfdnssd.h

TYPELIB = bfdnssdbrowsecallback.xpt bfdnssdenumeratecallback.xpt \
          bfdnssdregistercallback.xpt bfdnssdresolvecallback.xpt \
          bfdnssd.xpt bfnotifier.xpt

ALL_OBJS = $(HEADERS) $(TYPELIB) $(IMPL_OBJS) $(MOD_OBJS) $(LIB_OBJS)

all: $(ALL_OBJS)

%.xpt: %.idl
	$(XPIDL) $(IDL_INCLUDES) $(STABLE_IDL_INCLUDES) $(UNSTABLE_IDL_INCLUDES) -m typelib $<

%.h: %.idl
	$(XPIDL) $(IDL_INCLUDES) $(STABLE_IDL_INCLUDES) $(UNSTABLE_IDL_INCLUDES) -m header $<

bfdnssdimpl-darwin-intel.o: bfdnssdimpl.cpp
	$(CC) -arch i386 $(CFLAGS) -w -c -o $@ -I $(DI_XR_SDK_PATH)/sdk/include -I $(DI_XR_SDK_PATH)/include/xpcom -I $(DI_XR_SDK_PATH)/idl -DXP_UNIX -DXP_MACOSX -I . $<

bfdnssdimpl-darwin-ppc.o: bfdnssdimpl.cpp
	$(CC) -arch ppc $(CFLAGS) -w -c -o $@ -I $(DP_XR_SDK_PATH)/sdk/include -I $(DP_XR_SDK_PATH)/include/xpcom -I $(DP_XR_SDK_PATH)/idl -DXP_UNIX -DXP_MACOSX  $<

bfdnssdimpl-%.o: bfdnssdimpl.cpp
	$(CC) $(CFLAGS) -w -c -o $@ -I $(XR_INC_PATH)/stable -I $(XR_INC_PATH)/unstable -DXP_UNIX  $(NSPR_INCLUDES) $(AVAHI_INCLUDES) $<

bfdnssdmodule-darwin-intel.o: bfdnssdmodule.cpp
	$(CC) -arch i386 $(CFLAGS) -w -c -o $@ -I $(DI_XR_SDK_PATH)/sdk/include -I $(DI_XR_SDK_PATH)/include/xpcom -I $(DI_XR_SDK_PATH)/idl -DXP_UNIX -DXP_MACOSX $<

bfdnssdmodule-darwin-ppc.o: bfdnssdmodule.cpp
	$(CC) -arch ppc $(CFLAGS) -w -c -o $@ -I $(DP_XR_SDK_PATH)/sdk/include -I $(DP_XR_SDK_PATH)/include/xpcom -I $(DP_XR_SDK_PATH)/idl -DXP_UNIX -DXP_MACOSX $<

bfdnssdmodule-%.o: bfdnssdmodule.cpp
	$(CC) $(CFLAGS) -w -c -o $@ -I $(XR_INC_PATH)/stable -I $(XR_INC_PATH)/unstable -DXP_UNIX $(NSPR_INCLUDES) $(AVAHI_INCLUDES) $<

bfdnssd-darwin-intel.dylib: bfdnssdimpl-darwin-intel.o bfdnssdmodule-darwin-intel.o
	$(CC) -arch i386 -L$(DI_XR_SDK_PATH)/sdk/lib -L$(DI_XR_SDK_PATH)/bin $(LDFLAGS) -dynamiclib -o $@ $^ $(LIBS)

bfdnssd-darwin-ppc.dylib: bfdnssdimpl-darwin-ppc.o bfdnssdmodule-darwin-ppc.o
	$(CC) -arch ppc -L$(DP_XR_SDK_PATH)/sdk/lib -L$(DP_XR_SDK_PATH)/bin $(LDFLAGS) -dynamiclib -o $@ $^ $(LIBS)

bfdnssd-%.so: bfdnssdimpl-%.o bfdnssdmodule-%.o
	$(CC) -shared -Wl,-z,defs -L$(XR_LIB_PATH)/sdk/lib -L$(XR_LIB_PATH)/bin $(LDFLAGS) -dynamiclib -o $@ $^ $(LIBS)

bfdnssd-darwin-universal.dylib: bfdnssd-darwin-intel.dylib bfdnssd-darwin-ppc.dylib 
	lipo -create bfdnssd-darwin-intel.dylib bfdnssd-darwin-ppc.dylib -output bfdnssd-darwin-universal.dylib

clean:
	rm -fv $(ALL_OBJS)