@echo off

IF NOT EXIST w:\build mkdir w:\build

pushd w:\build

cl -Zi w:\handmade\code\win32_handmade.cpp User32.lib Gdi32.lib

popd