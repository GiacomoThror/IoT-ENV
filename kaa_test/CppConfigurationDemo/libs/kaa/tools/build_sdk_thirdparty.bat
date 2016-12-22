@REM
@REM Copyright 2014-2016 CyberVision, Inc.
@REM
@REM Licensed under the Apache License, Version 2.0 (the "License");
@REM you may not use this file except in compliance with the License.
@REM You may obtain a copy of the License at
@REM
@REM      http://www.apache.org/licenses/LICENSE-2.0
@REM
@REM Unless required by applicable law or agreed to in writing, software
@REM distributed under the License is distributed on an "AS IS" BASIS,
@REM WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
@REM See the License for the specific language governing permissions and
@REM limitations under the License.
@REM

@echo off

setlocal

call env.bat
SET BUILD_HOME=%CD%
SET BUILD_TYPE=debug

if not exist %ZLIB_ROOT% (
    echo "ZLIB_ROOT=%ZLIB_ROOT% does not exist"
    echo "Please set this variable to valid value in env.bat and run this script again"
    goto :eof
)

if not exist %AVRO_ROOT% (
    echo "AVRO_ROOT=%AVRO_ROOT% does not exist"
    echo "Please set this variable to valid value in env.bat and run this script again"
    goto :eof
)


if not exist %BOTAN_ROOT% (
    echo "BOTAN_ROOT=%BOTAN_ROOT% does not exist"
    echo "Please set this variable to valid value in env.bat and run this script again"
    goto :eof
)

if not exist %SQLITE_ROOT% (
    echo "SQLITE_ROOT=%SQLITE_ROOT% does not exist"
    echo "Please set this variable to valid value in env.bat and run this script again"
    goto :eof
)


if "%1" == "" goto startBuild
if /i %1 == release call :setRelease

:startBuild

call :buildKaaThirdparty
goto :eof

:setRelease
  SET BUILD_TYPE=release
goto :eof

:buildKaaThirdparty
  echo Building Kaa thirdparty components...
  call :buildZlib
  call :buildAvro
  call :buildBotan
  call :buildSqlite
goto :eof

:buildAvro
  echo Building Avro...

  call :deleteDir %AVRO_SRC%

  call :download %AVRO_SRC%.tar.gz %AVRO_URL%

  md %AVRO_SRC%\lang\c++\build.win
  cd %AVRO_SRC%\lang\c++\build.win

  if %BUILD_PLATFORM% == x86 (
    cmake -DCMAKE_INSTALL_PREFIX:PATH=%AVRO_ROOT% -G "Visual Studio %MSVC_VERSION%"  ..
  ) else (
    cmake -DCMAKE_INSTALL_PREFIX:PATH=%AVRO_ROOT% -G "Visual Studio %MSVC_VERSION% Win64"  ..
  )

  del buffertest.vcxproj
  del SchemaTests.vcxproj

  msbuild INSTALL.vcxproj /property:Configuration=%BUILD_TYPE%  /property:Platform=%BUILD_PLATFORM%

  cd %BUILD_HOME%

goto :eof

:buildZlib

  echo Building zlib...

  call :deleteDir %ZLIB_SRC%

  call :download %ZLIB_SRC%.tar.gz %ZLIB_URL%

  md %ZLIB_SRC%\build.win
  cd %ZLIB_SRC%\build.win

  if %BUILD_PLATFORM% == x86 (
    cmake -DCMAKE_INSTALL_PREFIX:PATH=%ZLIB_ROOT% -G "Visual Studio %MSVC_VERSION%"  ..
  ) else (
    cmake -DCMAKE_INSTALL_PREFIX:PATH=%ZLIB_ROOT% -G "Visual Studio %MSVC_VERSION% Win64"  ..
  )
  msbuild INSTALL.vcxproj /property:Configuration=%BUILD_TYPE% /property:Platform=%BUILD_PLATFORM%

  cd %BUILD_HOME%

goto :eof

:buildBotan

  echo Building Botan...

  call :deleteDir %BOTAN_SRC%

  call :download %BOTAN_SRC%.tar.gz %BOTAN_URL%

  cd %BOTAN_SRC%

  if %BUILD_PLATFORM% == x86 (
    set ARCH=i386
  ) else (
    set ARCH=amd64
  )
  if %BUILD_TYPE%==debug (
    python configure.py --cc=msvc --cpu=%ARCH% --prefix=%BOTAN_ROOT% --with-debug-info --no-optimizations
  ) else (
    python configure.py --cc=msvc --cpu=%ARCH% --prefix=%BOTAN_ROOT%
  ) 

  nmake install

  move %BOTAN_ROOT%/include/botan-1.11/botan %BOTAN_ROOT%/include

  cd %BUILD_HOME%

goto :eof

:buildSqlite

  echo Building Sqlite...
  call :download %SQLITE_SRC%.tar.gz %SQLITE_URL%
  cd %SQLITE_SRC%
  nmake /f Makefile.msc sqlite3.c
  nmake /f Makefile.msc
  mkdir %SQLITE_ROOT%\include
  copy sqlite3.h %SQLITE_ROOT%\include
  copy sqlite3ext.h %SQLITE_ROOT%\include
  mkdir %SQLITE_ROOT%\lib
  copy sqlite3.dll %SQLITE_ROOT%\lib
  copy sqlite3.lib %SQLITE_ROOT%\lib

goto :eof

:download

  IF not EXIST %1 (
    wget --no-check-certificate --content-disposition -c %2
  )
  bsdtar -xf %1

goto :eof

:deleteDir

  IF EXIST %1\NUL (
    del /s /f /q %1\*.*
    for /f %%f in ('dir /ad /b %1\') do rd /s /q %1\%%f
    rd /s /q %1
  )

goto :eof

endlocal

