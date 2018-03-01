@echo off
IF NOT EXIST ..\build mkdir ..\build
pushd ..\build
cl /MT /Zi /Od ../code/main.cpp /I D:\SDL2\SDL2-2.0.3\include\ /I D:\SDL2\SDL2_image-2.0.1\include\ /link /ENTRY:wmainCRTStartup /SUBSYSTEM:WINDOWS /LIBPATH:D:\SDL2\SDL2-2.0.3\lib\x86\ SDL2.lib SDL2main.lib /LIBPATH:D:\SDL2\SDL2_image-2.0.1\lib\x86\ SDL2_image.lib
popd