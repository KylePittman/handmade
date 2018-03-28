@echo off
if not exist w:\ (
	subst w: "d:\CppGames"
)
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
set path=w:\misc;%path%
set path=%path%;D:\Program Files (x86)\Notepad++