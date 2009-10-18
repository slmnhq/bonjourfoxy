@echo off
vcbuild >NUL 2>NUL
if %ERRORLEVEL% == -1 GOTO BUILD
echo Looks like vcvarsall.bat has not been called...
call "%VS90COMNTOOLS%vsvars32.bat"
:BUILD
IF "%1" == "clean" (
    @vcbuild bfdnssd.vcproj /clean /nologo /platform=Win32 Debug
) ELSE (
    @vcbuild bfdnssd.vcproj /nologo /platform=Win32 Debug
)