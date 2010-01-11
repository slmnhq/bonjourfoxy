XR_SDK_PATH = ../../sdk/$(FF_REL)/win32/xulrunner-sdk
XR_SDK_PATH_W = $(shell cygpath -w $(XR_SDK_PATH))
XPIDL = $(XR_SDK_PATH)/bin/xpidl
IDL_INCLUDES = -I .. -I $(XR_SDK_PATH)/sdk/idl -I $(XR_SDK_PATH)/idl
ALL_OBJS = $(HEADERS) $(TYPELIB) CBFDNSSDService.obj CBFDNSSDServiceModule.obj BFDNSSDService.dll

all: $(ALL_OBJS)

%.xpt: %.idl
	$(XPIDL) $(IDL_INCLUDES) -m typelib $<

%.h: %.idl
	$(XPIDL) $(IDL_INCLUDES) -m header $<

%.obj: %.cpp
	cl ../$*.cpp /Od \
    /I "." \
    /I "$(XR_SDK_PATH_W)\include" \
    /I "$(XR_SDK_PATH_W)\sdk\include" \
    /I "$(XR_SDK_PATH_W)\include\xpcom" \
    /I "C:\Program Files\Bonjour SDK\Include" \
    /D "WIN32" /D "_WINDOWS" /D "_USRDLL" \
    /D "DNSSDSERVICE_EXPORTS" /D "XP_WIN" /D "XP_WIN32" \
    /D "_WINDLL" /D "_MBCS" /Gm /EHsc /RTC1 /MT /Zc:wchar_t- \
    /W3 /c /Zi /TP

BFDNSSDService.dll: CBFDNSSDService.obj CBFDNSSDServiceModule.obj
	link /OUT:"BFDNSSDService.dll" /INCREMENTAL \
    /LIBPATH:"$(XR_SDK_PATH_W)\sdk\lib" \
    /LIBPATH:"$(XR_SDK_PATH_W)\lib" \
    /LIBPATH:"c:\Program Files\Bonjour SDK\lib\win32" \
    /DLL /MANIFEST /MANIFESTFILE:"BFDNSSDService.dll.intermediate.manifest" \
    /MANIFESTUAC:"level='asInvoker' uiAccess='false'" \
    /DYNAMICBASE:NO /IMPLIB:"BFDNSSDService.lib" /MACHINE:X86 \
    nspr4.lib xpcom.lib xpcomglue_s.lib ws2_32.lib dnssd.lib  kernel32.lib \
    user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib \
    ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib \
    $^