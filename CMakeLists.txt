cmake_minimum_required (VERSION 2.6)
project (mb_gateway)

#support switches
option(I2C_SUPPORT "compile with I2C support" ON)

include_directories(/usr/include)

#libmodbus git subdirectory
include_directories(libmodbus)
include_directories(libmodbus/src)

#default includes
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/inc
	${CMAKE_CURRENT_SOURCE_DIR}/MB_Framework/inc
)

#add sources(own CMakeList.txt)
add_subdirectory (src)