export MACOSX_DEPLOYMENT_TARGET="10.5"
XR_SDK_PATH_PRE = ../../sdk/$(FF_REL)
DI_XR_SDK_PATH = $(XR_SDK_PATH_PRE)/maci386/xulrunner-sdk
XPIDL = $(DI_XR_SDK_PATH)/bin/xpidl
IDL_INCLUDES = -I .. -I $(DI_XR_SDK_PATH)/sdk/idl -I $(DI_XR_SDK_PATH)/idl
IMPL_OBJS = bfdnssdimpl-darwin-i386.o bfdnssdimpl-darwin-ppc.o
MOD_OBJS = bfdnssdmodule-darwin-i386.o bfdnssdmodule-darwin-ppc.o
LIB_OBJS = bfdnssd-darwin-i386.dylib bfdnssd-darwin-ppc.dylib bfdnssd-darwin-universal.dylib
ALL_OBJS = $(HEADERS) $(TYPELIB) $(IMPL_OBJS) $(MOD_OBJS) $(LIB_OBJS)

all: $(ALL_OBJS)

%.xpt: %.idl
	$(XPIDL) $(IDL_INCLUDES) -m typelib $<

%.h: %.idl
	$(XPIDL) $(IDL_INCLUDES) -m header $<

bfdnssdimpl-darwin-%.o: bfdnssdimpl.cpp
	g++ -arch $* -mmacosx-version-min=10.5 -isysroot /Developer/SDKs/MacOSX10.5.sdk \
    -fshort-wchar -w -c -o $@ -I . \
    -I $(XR_SDK_PATH_PRE)/mac$(*)/xulrunner-sdk/include \
    -I $(XR_SDK_PATH_PRE)/mac$(*)/xulrunner-sdk/sdk/include \
    -I $(XR_SDK_PATH_PRE)/mac$(*)/xulrunner-sdk/include/xpcom \
    -DXP_UNIX -DXP_MACOSX $<

bfdnssdmodule-darwin-%.o: bfdnssdmodule.cpp
	g++ -arch $*  -mmacosx-version-min=10.5 -isysroot /Developer/SDKs/MacOSX10.5.sdk \
    -fshort-wchar -w -c -o $@ -I . \
    -I $(XR_SDK_PATH_PRE)/mac$(*)/xulrunner-sdk/include \
    -I $(XR_SDK_PATH_PRE)/mac$(*)/xulrunner-sdk/sdk/include \
    -I $(XR_SDK_PATH_PRE)/mac$(*)/xulrunner-sdk/include/xpcom \
    -DXP_UNIX -DXP_MACOSX $<

bfdnssd-darwin-%.dylib: bfdnssdimpl-darwin-%.o bfdnssdmodule-darwin-%.o
	g++ -arch $*  -mmacosx-version-min=10.5 -isysroot /Developer/SDKs/MacOSX10.5.sdk \
    -L$(XR_SDK_PATH_PRE)/mac$(*)/xulrunner-sdk/sdk/lib \
    -L$(XR_SDK_PATH_PRE)/mac$(*)/xulrunner-sdk/bin \
    -Wl,-executable_path, -fshort-wchar -dynamiclib -o $@ $^ \
    -lxpcomglue_s -lxpcom -lnspr4

bfdnssd-darwin-universal.dylib: bfdnssd-darwin-i386.dylib bfdnssd-darwin-ppc.dylib 
	lipo -create bfdnssd-darwin-i386.dylib bfdnssd-darwin-ppc.dylib -output bfdnssd-darwin-universal.dylib
