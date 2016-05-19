ECHO OFF
SET DEBUG_VALUE=ON
SET DIR_SOURCE=source\
SET DIR_BUILD=w32_vs2013_build

IF NOT EXIST %DIR_BUILD% MKDIR %DIR_BUILD%

cd %DIR_BUILD%
cmake -G "Visual Studio 12" ..\%DIR_SOURCE% -DRENGINE_DEBUG=%DEBUG_VALUE%
cd ..