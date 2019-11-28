CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# Name of project, learningnet library and executable:
SET(PROJECT_NAME "LEARNINGNET")
SET(LEARNINGNET_LIBRARIES "LN")
SET(EXECUTABLE "learningnet-pathfinder")
option(LN_TESTS "Whether to compile tests as well." ON)

PROJECT(${PROJECT_NAME} CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Instructions for finding installed dependencies are in ./cmake
SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)


## LEMON
# Try to use LEMON as a CMAKE subproject by looking for it in the 'lemon' or
# 'deps/lemon' subdirectories or in the directory given by the
# LEMON_SOURCE_ROOT_DIR variable.
# If LEMON is not there, try to find an installed version of LEMON.

FIND_PATH(LEMON_SOURCE_ROOT_DIR CMakeLists.txt
  PATHS ${CMAKE_SOURCE_DIR}/lemon ${CMAKE_SOURCE_DIR}/deps/lemon
  NO_DEFAULT_PATH
  DOC "Location of LEMON source as a CMAKE subproject")

IF(EXISTS ${LEMON_SOURCE_ROOT_DIR})
  ADD_SUBDIRECTORY(${LEMON_SOURCE_ROOT_DIR} deps/lemon)
  SET(LEMON_INCLUDE_DIRS
    ${LEMON_SOURCE_ROOT_DIR}
    ${CMAKE_BINARY_DIR}/deps/lemon
  )
  SET(LEMON_LIBRARIES lemon)
  UNSET(LEMON_ROOT_DIR CACHE)
  UNSET(LEMON_DIR CACHE)
  UNSET(LEMON_INCLUDE_DIR CACHE)
  UNSET(LEMON_LIBRARY CACHE)
ELSE()
  FIND_PACKAGE(LEMON QUIET NO_MODULE)
  FIND_PACKAGE(LEMON REQUIRED)
ENDIF()

INCLUDE_DIRECTORIES(${LEMON_INCLUDE_DIRS})


## rapidjson
# Search for rapidjson under rapidjson and deps/rapidjson.
# If it is not there, search for an installed version.
# If found, add it as a header-only library.
FIND_PATH(RAPIDJSON_SOURCE_ROOT_DIR CMakeLists.txt
  PATHS ${CMAKE_SOURCE_DIR}/rapidjson ${CMAKE_SOURCE_DIR}/deps/rapidjson
  NO_DEFAULT_PATH
  DOC "Location of rapidjson source as a CMAKE subproject")

IF(EXISTS ${RAPIDJSON_SOURCE_ROOT_DIR})
  SET(RAPIDJSON_INCLUDE_DIRS
    ${RAPIDJSON_SOURCE_ROOT_DIR}/include
    ${CMAKE_BINARY_DIR}/deps/rapidjson/include
  )
  SET(RAPIDJSON_LIBRARIES rapidjson)

  FILE(GLOB RAPIDJSON_HEADERS ${RAPIDJSON_INCLUDE_DIRS}/rapidjson/*.h)
  ADD_LIBRARY(${RAPIDJSON_LIBRARIES} "${RAPIDJSON_HEADERS}")
  SET_TARGET_PROPERTIES(${RAPIDJSON_LIBRARIES} PROPERTIES LINKER_LANGUAGE CXX)
  TARGET_INCLUDE_DIRECTORIES(${RAPIDJSON_LIBRARIES} INTERFACE ${RAPIDJSON_INCLUDE_DIR})
ELSE()
  FIND_PACKAGE(RAPIDJSON QUIET NO_MODULE)
  FIND_PACKAGE(RAPIDJSON REQUIRED)
ENDIF()


## LEARNINGNET_LIBRARIES
# Create the header-only library, set its include-dir as ./include.
ADD_LIBRARY(${LEARNINGNET_LIBRARIES} INTERFACE)
TARGET_INCLUDE_DIRECTORIES(${LEARNINGNET_LIBRARIES} INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include)


### EXECUTABLE
# Compile main.cpp to a binary with name ${EXECUTABLE}.
ADD_EXECUTABLE(${EXECUTABLE} main.cpp)

# Link ${EXECUTABLE} with lemon and learningnet.
TARGET_LINK_LIBRARIES(${EXECUTABLE} ${LEMON_LIBRARIES})
TARGET_LINK_LIBRARIES(${EXECUTABLE} ${LEARNINGNET_LIBRARIES})
TARGET_LINK_LIBRARIES(${EXECUTABLE} ${RAPIDJSON_LIBRARIES})


### TESTS VIA CATCH
IF(LN_TESTS)
    # Prepare "Catch" library for other executables.
    SET(CATCH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test/catch)
    ADD_LIBRARY(Catch INTERFACE)
    TARGET_INCLUDE_DIRECTORIES(Catch INTERFACE ${CATCH_INCLUDE_DIR})

    # Make main test with CMAKE_CONFIG_MAIN-define executable.
    ADD_LIBRARY(test-main ${CMAKE_CURRENT_SOURCE_DIR}/test-main.cpp)
    TARGET_LINK_LIBRARIES(test-main Catch)

    # This is the list of source files, specified explicitly by name.
    # If files were specified via globbing, adding new source-files would not update
    # the build!
    SET(APP_SOURCES)
    LIST(APPEND APP_SOURCES "check_test.cpp")
    LIST(APPEND APP_SOURCES "compress_test.cpp")
    LIST(APPEND APP_SOURCES "recommend_test.cpp")
    LIST(APPEND APP_SOURCES "experiments.cpp")

    # Put tests binaries into separate test dir.
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "test")

    # For each file in ${APP_SOURCES}:
    FOREACH(sourcefile ${APP_SOURCES})
        # Cut off the file extension to get ${output}.
        GET_FILENAME_COMPONENT(output ${sourcefile} NAME_WE)
        # Compile the file to a binary with name ${output}.
        ADD_EXECUTABLE(${output} "test/${sourcefile}")

        # Link ${output} to required libraries.
        TARGET_LINK_LIBRARIES(${output} lemon)
        TARGET_LINK_LIBRARIES(${output} LN)
        TARGET_LINK_LIBRARIES(${output} Catch)
        TARGET_LINK_LIBRARIES(${output} test-main)
    ENDFOREACH(sourcefile ${APP_SOURCES})
ENDIF()


## Documentation
# Create new target 'make doc' to create doxygen documentation in ./doc/html/
FIND_PACKAGE(Doxygen)
IF(DOXYGEN_FOUND)
    SET(DOC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/doc")
    ADD_CUSTOM_TARGET(doc ${DOXYGEN_EXECUTABLE} "${DOC_DIR}/doxygen.cfg"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
ELSE()
    MESSAGE(WARNING "Doxygen not found. Documentation cannot be built.")
ENDIF()


## Installation
# Not tested!
# Create new target 'make install' to install the binary to $PREFIX/bin.
# Set install directories:
IF(UNIX)
  SET(INSTALL_BIN_DIR "bin" CACHE STRING
    "Subdirectory for installing the binaries")
  SET(INSTALL_DOC_DIR "share/doc/${PROJECT_NAME}" CACHE STRING
    "Subdirectory for installing the doc")
  SET(INSTALL_DATA_DIR "share/data/${PROJECT_NAME}" CACHE STRING
    "Subdirectory for installing the map data")
  SET(INSTALL_EXAMPLES_DIR "share/doc/${PROJECT_NAME}/examples" CACHE STRING
    "Subdirectory for installing the examples")
ELSE()
  SET(INSTALL_BIN_DIR "." CACHE STRING "Subdir for installing the binaries")
  SET(INSTALL_DOC_DIR "doc" CACHE STRING "Subdir for installing the doc")
  SET(INSTALL_DATA_DIR "data" CACHE STRING "Subdir for installing the map data")
  SET(INSTALL_EXAMPLES_DIR "examples" CACHE STRING "Subdir for installing the examples")
ENDIF()

INSTALL(
  TARGETS learningnet-pathfinder
  RUNTIME DESTINATION ${INSTALL_BIN_DIR}
  COMPONENT bin
)


# Set warning flags
IF(CMAKE_COMPILER_IS_GNUCXX)
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
ENDIF()
