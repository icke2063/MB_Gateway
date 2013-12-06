configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/build_options.h.in
	${CMAKE_CURRENT_BINARY_DIR}/build_options.h
)

### ADD INCLUDE DIRS ###
# add the binary tree to the search path for include files
include_directories(
${CMAKE_CURRENT_BINARY_DIR}
${CMAKE_CURRENT_BINARY_DIR}/base
${CMAKE_CURRENT_SOURCE_DIR}/base
${MB_Framework_SOURCE_DIR}/include
)

## find extern lib
find_library(MODBUS_LIB modbus)
find_library(LOG4CPP log4cpp)
FIND_PACKAGE(Boost COMPONENTS program_options REQUIRED)


### ADD SOURCES ###
#add common_sources
set (COMMON_SOURCES
	
)

#add default sources
set (DEF_SOURCES
	base/Connection.cpp
	base/Server.cpp
	base/SlaveList.cpp
	base/SummerySlave.cpp
	base/handler/DefaultHandler.cpp
	base/handler/MultiRegisterHandler.cpp
	base/DummyFunctor.cpp
)
	

#add webinterface	
set(ECPPC "/usr/bin/ecppc")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/webinterface)
FILE(GLOB ECPP_SRC "${CMAKE_CURRENT_SOURCE_DIR}/webinterface/pages/*.ecpp")

set(PAGES_CPP_SRC webinterface/WebInterface.cpp)
foreach(_file ${ECPP_SRC})
	
	#create outputfile absolute path
    string(REPLACE ".ecpp" "" file_cpp ${_file})
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}" file_cpp ${file_cpp})
    set(file_cpp "${file_cpp}.cpp")
    
    #createoutput directory
    get_filename_component(cur_path ${file_cpp} PATH)
    file(MAKE_DIRECTORY ${cur_path})
  
	#run tntnet compiler
    add_custom_command(
        OUTPUT ${file_cpp} 
        COMMAND ${ECPPC} -v 
            -I${CMAKE_CURRENT_SOURCE_DIR}/webinterface/pages
            -I${CMAKE_CURRENT_SOURCE_DIR}/base
            -o ${file_cpp} ${_file}
        DEPENDS ${_file}
    ) 
    list(APPEND PAGES_CPP_SRC ${file_cpp})
endforeach()	
		
#add I2C includes
if (I2C_SUPPORT)
	include_directories(
		${CMAKE_CURRENT_BINARY_DIR}/I2C
		${CMAKE_CURRENT_SOURCE_DIR}/I2C
		${CMAKE_CURRENT_SOURCE_DIR}/I2C/I2C-Comm
		${CMAKE_CURRENT_SOURCE_DIR}/I2C/Handler
		${CMAKE_CURRENT_SOURCE_DIR}/I2C/I2C_IO_BOARD.git/base/inc	
	)
	
	set (I2C_SOURCES
	  I2C/IOBoardSlave.cpp
	  I2C/I2C-Comm/I2CComm.cpp
	  I2C/Handler/MultiByteHandler.cpp
	  I2C/I2CScanner.cpp
	)
	
endif (I2C_SUPPORT)

### ADD TARGET ###
add_executable (${PROJECT_NAME} base/MB_Gateway.cpp
  ${DEF_SOURCES}
  ${PAGES_CPP_SRC}
  ${I2C_SOURCES}
)

#add_executable (testclient testclient/MBGateway_Testclient.cpp)


### ADD PREPROCESS MACROS ###

### LINK LIBRARIES ###
#default libraries
target_link_libraries(${PROJECT_NAME}
	${MODBUS_LIB}
	${Boost_LIBRARIES}
	${LOG4CPP}
	common_cpp-shared
	tntnet
	cxxtools
)

#target_link_libraries(testclient
#	${MODBUS_LIB}
#	${Boost_LIBRARIES}
#)
### INSTALL ###

INSTALL(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib

)