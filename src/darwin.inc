export MACOSX_DEPLOYMENT_TARGET="10.5"
XR_SDK_PATH_PRE = ../../sdk/$(FF_REL)
DI_XR_SDK_PATH = $(XR_SDK_PATH_PRE)/maci386/xulrunner-sdk
XPIDL = $(DI_XR_SDK_PATH)/bin/xpidl
IDL_INCLUDES = -I .. -I $(DI_XR_SDK_PATH)/sdk/idl -I $(DI_XR_SDK_PATH)/idl
IMPL_OBJS = CBFDNSSDService-darwin-ppc.o CBFDNSSDService-darwin-i386.o
MOD_OBJS = CBFDNSSDServiceModule-darwin-ppc.o CBFDNSSDServiceModule-darwin-i386.o
LIB_OBJS = BFDNSSDService-darwin-ppc.dylib BFDNSSDService-darwin-universal.dylib
ALL_OBJS = $(HEADERS) $(TYPELIB) $(IMPL_OBJS) $(MOD_OBJS) $(LIB_OBJS)

all: $(ALL_OBJS)

%.xpt: %.idl
	$(XPIDL) $(IDL_INCLUDES) -m typelib $<

%.h: %.idl
	$(XPIDL) $(IDL_INCLUDES) -m header $<

CBFDNSSDService-darwin-%.o: CBFDNSSDService.cpp
	g++ -arch $* -mmacosx-version-min=10.5 -isysroot /Developer/SDKs/MacOSX10.5.sdk \
    -fshort-wchar -w -c -o $@ -I . \
    -I $(XR_SDK_PATH_PRE)/mac$(*)/xulrunner-sdk/include \
    -I $(XR_SDK_PATH_PRE)/mac$(*)/xulrunner-sdk/sdk/include \
    -I $(XR_SDK_PATH_PRE)/mac$(*)/xulrunner-sdk/include/xpcom \
    -DXP_UNIX -DXP_MACOSX $<

CBFDNSSDServiceModule-darwin-%.o: CBFDNSSDServiceModule.cpp
	g++ -arch $*  -mmacosx-version-min=10.5 -isysroot /Developer/SDKs/MacOSX10.5.sdk \
    -fshort-wchar -w -c -o $@ -I . \
    -I $(XR_SDK_PATH_PRE)/mac$(*)/xulrunner-sdk/include \
    -I $(XR_SDK_PATH_PRE)/mac$(*)/xulrunner-sdk/sdk/include \
    -I $(XR_SDK_PATH_PRE)/mac$(*)/xulrunner-sdk/include/xpcom \
    -DXP_UNIX -DXP_MACOSX $<

BFDNSSDService-darwin-%.dylib: CBFDNSSDService-darwin-%.o CBFDNSSDServiceModule-darwin-%.o
	g++ -arch $*  -mmacosx-version-min=10.5 -isysroot /Developer/SDKs/MacOSX10.5.sdk \
    -L$(XR_SDK_PATH_PRE)/mac$(*)/xulrunner-sdk/sdk/lib \
    -L$(XR_SDK_PATH_PRE)/mac$(*)/xulrunner-sdk/bin \
    -Wl,-executable_path, -fshort-wchar -dynamiclib -o $@ $^ \
    -lxpcomglue_s -lxpcom -lnspr4

BFDNSSDService-darwin-universal.dylib: BFDNSSDService-darwin-ppc.dylib  BFDNSSDService-darwin-i386.dylib
	lipo -create BFDNSSDService-darwin-i386.dylib BFDNSSDService-darwin-ppc.dylib -output BFDNSSDService-darwin-universal.dylib
