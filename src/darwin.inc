export MACOSX_DEPLOYMENT_TARGET="10.5"
XR_SDK_PATH_PRE = ../../sdk/$(FF_REL)
DI_XR_SDK_PATH = $(XR_SDK_PATH_PRE)/maci386/xulrunner-sdk
XPIDL = $(DI_XR_SDK_PATH)/bin/xpidl
IDL_INCLUDES = -I .. -I $(DI_XR_SDK_PATH)/sdk/idl -I $(DI_XR_SDK_PATH)/idl
IMPL_OBJS = CDNSSDService-darwin-ppc.o CDNSSDService-darwin-i386.o
MOD_OBJS = CDNSSDServiceModule-darwin-ppc.o CDNSSDServiceModule-darwin-i386.o
LIB_OBJS = DNSSDService-darwin-ppc.dylib DNSSDService-darwin-universal.dylib
ALL_OBJS = $(HEADERS) $(TYPELIB) $(IMPL_OBJS) $(MOD_OBJS) $(LIB_OBJS)

all: $(ALL_OBJS)

%.xpt: %.idl
	$(XPIDL) $(IDL_INCLUDES) -m typelib $<

%.h: %.idl
	$(XPIDL) $(IDL_INCLUDES) -m header $<

CDNSSDService-darwin-%.o: CDNSSDService.cpp
	g++ -arch $* -mmacosx-version-min=10.5 -isysroot /Developer/SDKs/MacOSX10.5.sdk \
    -fshort-wchar -w -c -o $@ -I . \
    -I $(XR_SDK_PATH_PRE)/mac$(*)/xulrunner-sdk/include \
    -I $(XR_SDK_PATH_PRE)/mac$(*)/xulrunner-sdk/sdk/include \
    -I $(XR_SDK_PATH_PRE)/mac$(*)/xulrunner-sdk/include/xpcom \
    -DXP_UNIX -DXP_MACOSX $<

CDNSSDServiceModule-darwin-%.o: CDNSSDServiceModule.cpp
	g++ -arch $*  -mmacosx-version-min=10.5 -isysroot /Developer/SDKs/MacOSX10.5.sdk \
    -fshort-wchar -w -c -o $@ -I . \
    -I $(XR_SDK_PATH_PRE)/mac$(*)/xulrunner-sdk/include \
    -I $(XR_SDK_PATH_PRE)/mac$(*)/xulrunner-sdk/sdk/include \
    -I $(XR_SDK_PATH_PRE)/mac$(*)/xulrunner-sdk/include/xpcom \
    -DXP_UNIX -DXP_MACOSX $<

DNSSDService-darwin-%.dylib: CDNSSDService-darwin-%.o CDNSSDServiceModule-darwin-%.o
	g++ -arch $*  -mmacosx-version-min=10.5 -isysroot /Developer/SDKs/MacOSX10.5.sdk \
    -L$(XR_SDK_PATH_PRE)/mac$(*)/xulrunner-sdk/sdk/lib \
    -L$(XR_SDK_PATH_PRE)/mac$(*)/xulrunner-sdk/bin \
    -Wl,-executable_path, -fshort-wchar -dynamiclib -o $@ $^ \
    -lxpcomglue_s -lxpcom -lnspr4

DNSSDService-darwin-universal.dylib: DNSSDService-darwin-ppc.dylib  DNSSDService-darwin-i386.dylib
	lipo -create DNSSDService-darwin-i386.dylib DNSSDService-darwin-ppc.dylib -output DNSSDService-darwin-universal.dylib
