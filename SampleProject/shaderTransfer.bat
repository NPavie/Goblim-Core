@echo off

set outputDir=%1
set outputProject=%2
set currentDir=%CD%
set globalDir=%CD%\..\Core

echo "Clean Shaders and Objects"

if exist "%outputDir%\%outputProject%\Materials\" RMDIR /S /Q "%outputDir%\%outputProject%\Materials\"
if exist "%outputDir%\%outputProject%\Effects\" RMDIR /S /Q "%outputDir%\%outputProject%\Effects\"
if exist "%outputDir%\%outputProject%\Objets\" RMDIR /S /Q "%outputDir%\%outputProject%\Objets\"

echo "Prepare to copy shaders from" 
echo "%currentDir% and %globalDir%"
echo "to %outputDir%%outputProject%"

if exist "%globalDir%\Materials" xcopy "%globalDir%\Materials\*.glsl" "%outputDir%\%outputProject%\Materials\" /S /I /y /C
if exist "%globalDir%\Effects" xcopy "%globalDir%\Effects\*.glsl" "%outputDir%\%outputProject%\Effects\" /S /I /y /C
if exist "%globalDir%\Objets" xcopy "%globalDir%\Objets" "%outputDir%\%outputProject%\Objets\" /S /I /y /C

:: partie pour les sous projets
if exist "%currentDir%\Materials" xcopy "%currentDir%\Materials\*.glsl" "%outputDir%\%outputProject%\Materials\" /S /I /y /C
if exist "%currentDir%\Effects" xcopy "%currentDir%\Effects\*.glsl" "%outputDir%\%outputProject%\Effects\" /S /I /y /C
if exist "%currentDir%\Objets" xcopy "%currentDir%\Objets" "%outputDir%\%outputProject%\Objets\" /S /I /y /C
