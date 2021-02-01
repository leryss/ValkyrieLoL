:: Set VPath environment variable
pushd %~dp0
set VDir=%CD%
popd
setx VPath %VDir%

:: Move boost python dll to system32 so it can be picked up automatically
set BoostPyFile=boost_python39-vc141-mt-x32-1_75.dll
copy %VDIR%\%BoostPyFile% %WINDIR%\%BoostPyFile%