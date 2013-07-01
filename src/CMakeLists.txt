### ADD INCLUDE DIRS ###
# add the binary tree to the search path for include files
include_directories(
${CMAKE_CURRENT_BINARY_DIR}/base
${CMAKE_CURRENT_SOURCE_DIR}/base
)

## find extern lib
find_library(MODBUS_LIB modbus)
find_library(LOG4CPP log4cpp)
#find_library(BOOST_LIB thread)
FIND_PACKAGE(Boost COMPONENTS thread serialization REQUIRED)


### ADD SOURCES ###
#add default sources
set (DEF_SOURCES
	base/MB_Gateway.cpp
	base/Logger.cpp
	base/ThreadPool.cpp
	base/WorkerThread.cpp
    base/Connection.cpp
    base/Server.cpp
    base/DefaultHandler.cpp
	)

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
	I2C/I2C-Comm//I2CComm.cpp
	I2C/Handler/MultiByteHandler.cpp
	)
	
endif (I2C_SUPPORT)

### ADD TARGET ###
add_executable (${PROJECT_NAME}
				${DEF_SOURCES}
				${I2C_SOURCES}
)

### ADD PREPROCESS MACROS ###

### LINK LIBRARIES ###
#default libraries
target_link_libraries(${PROJECT_NAME}
	${MODBUS_LIB}
	${Boost_LIBRARIES}
	${LOG4CPP}
)

### INSTALL ###

INSTALL(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
