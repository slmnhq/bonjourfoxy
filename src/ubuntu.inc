UNAMEM = $(shell uname -m || echo not)
ifeq ($(UNAMEM),x86_64)
	LIB_PATH = /usr/lib64
	MOD_OBJS = bfdnssdmodule-linux64.o
	IMPL_OBJS = bfdnssdimpl-linux64.o
	LIB_OBJS = bfdnssd-linux64.so
else
	LIB_PATH = /usr/lib
	MOD_OBJS = bfdnssdmodule-linux.o
	IMPL_OBJS = bfdnssdimpl-linux.o
	LIB_OBJS = bfdnssd-linux.so
endif

XR_VERSION = $(shell xulrunner --gre-version)
XR_LIB_PATH = $(LIB_PATH)/xulrunner-devel-$(XR_VERSION)
XR_INC_PATH = /usr/include/xulrunner-$(XR_VERSION)
XR_IDL_PATH = /usr/share/idl/xulrunner-$(XR_VERSION)
XR_RUN_PATH = /usr/lib/xulrunner-$(XR_VERSION)
XPIDL                 = $(XR_RUN_PATH)/xpidl
IDL_INCLUDES          = -I $(XR_LIB_PATH)/sdk/idl
STABLE_IDL_INCLUDES   = -I $(XR_IDL_PATH)/stable
UNSTABLE_IDL_INCLUDES = -I $(XR_IDL_PATH)/unstable
STABLE_IDL_INCLUDES   = -I $(XR_IDL_PATH)/stable
UNSTABLE_IDL_INCLUDES = -I $(XR_IDL_PATH)/unstable
NSPR_INCLUDES         = -I /usr/include/nspr
AVAHI_INCLUDES        = -I /usr/include/avahi-compat-libdns_sd
CFLAGS                = -fPIC -fshort-wchar
LDFLAGS               = -Wl,-rpath-link,$(XR_LIB_PATH)/sdk -fshort-wchar
LIBS                  = -lxpcomglue_s -lxpcom -lnspr4 -ldns_sd
