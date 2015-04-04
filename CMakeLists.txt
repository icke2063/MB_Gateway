cmake_minimum_required (VERSION 2.6)
project (crumby)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")

#support switches
option(CRUMBY_USE_C++11_SUPPORT "use c++11 if available" ON)
option(CRUMBY_WEBINTERFACE "enable webinterface" ON)

## I2C
option(CRUMBY_I2C_SUPPORT "compile with I2C support" ON)
option(CRUMBY_I2C_IOBOARD " compile with I2C IOBoard support" ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CXX11-CMake-Macros")

include(CXX11)

check_for_cxx11_compiler(C++11)

if(C++11 AND CRUMBY_USE_C++11_SUPPORT)
ADD_DEFINITIONS(-std=c++0x)
  #enable c++11
  enable_cxx11()
  message(STATUS "use C++11")
  FIND_PACKAGE(Boost COMPONENTS program_options REQUIRED)
else()
  #use boost fallback
  FIND_PACKAGE(Boost COMPONENTS program_options thread system serialization chrono REQUIRED)
  message(STATUS "use boost")
  SET(ICKE2063_CRUMBY_NO_CPP11 1)
endif()

add_subdirectory (common_cpp)

#default includes
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/inc
	${CMAKE_CURRENT_SOURCE_DIR}/MB_Framework/inc
	${common_cpp_INC}
)

add_subdirectory(MB_Framework)

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
