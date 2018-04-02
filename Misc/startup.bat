@echo off
set pc=%1%
if not exist w:\ (
	if %pc%==0 (
		subst w: "d:\CppGames"
		set "PATH=%PATH%;w:\misc;"
	) else if %pc%==1 (
		subst w: "c:\Users\kylep\CppGames"
		set "PATH=%PATH%;w:\handmade\Misc;"
	)
)

if not defined DevEnvDir (
pushd "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build"
call vcvarsall.bat x64
popd
)

if %pc%==0 (
	set "PATH=%PATH%;D:\Program Files (x86)\Notepad++"
) else if %pc%==1 (
	set "PATH=%PATH%;C:\Program Files\Notepad++"
)
cd w:\handmade\Misc