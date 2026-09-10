@echo off
rem Creates a directory junction %1 -> %2 if %1 doesn't already exist.
rem Used by CMakeLists.txt's post-build step to put Data/ and Images/ next to
rem the built executable. Kept as its own script because cmd.exe's "if"/
rem "mklink" builtins misparse unquoted forward-slash CMake paths (they read
rem "/Users" in "C:/Users/..." as a switch), and getting real quote
rem characters through CMake's multi-command post-build chain into a builtin
rem is unreliable; a plain argv into a script does not have that problem.
if not exist "%~1" mklink /J "%~1" "%~2"
