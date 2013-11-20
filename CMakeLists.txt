cmake_minimum_required (VERSION 2.6)
project (crumby)

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
	${CMAKE_CURRENT_SOURCE_DIR}/common_cpp/include
)

add_subdirectory (common_cpp)
add_subdirectory (MB_Framework)

#get_cmake_property(_variableNames VARIABLES)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()


#add sources(own CMakeList.txt)
add_subdirectory (src)

configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/scripts/startup/startup_script.sh.in"
	${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.sh
)

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.sh
  DESTINATION /etc/init.d/)