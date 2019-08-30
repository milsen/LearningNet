CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# Name of project, learningnet library and executable:
SET(PROJECT_NAME "LEARNINGNET")
set(LEARNINGNET_LIBRARIES "LN")
set(EXECUTABLE "learningnet-pathfinder")

PROJECT(${PROJECT_NAME} CXX)

SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# Set install directories
IF(UNIX)
  SET(INSTALL_BIN_DIR "bin" CACHE STRING
    "Subdir for installing the binaries")
  SET(INSTALL_DOC_DIR "share/doc/${PROJECT_NAME}"
    CACHE STRING
    "Subdir for installing the doc")
  SET(INSTALL_DATA_DIR "share/data/${PROJECT_NAME}"
    CACHE STRING
    "Subdir for installing the map data")
  SET(INSTALL_EXAMPLES_DIR "share/doc/${PROJECT_NAME}/examples"
    CACHE STRING
    "Subdir for installing the examples")
ELSE(UNIX)
  SET(INSTALL_BIN_DIR "." CACHE STRING
                  "Subdir for installing the binaries")
  SET(INSTALL_DOC_DIR "doc" CACHE STRING
                  "Subdir for installing the doc")
  SET(INSTALL_DATA_DIR "data" CACHE STRING
                  "Subdir for installing the map data")
  SET(INSTALL_EXAMPLES_DIR "examples" CACHE STRING
                       "Subdir for installing the examples")
ENDIF(UNIX)


## LEMON
# First, it tries to use LEMON as a CMAKE subproject by looking for
# it in the 'lemon' or 'deps/lemon' subdirectories or in directory
# given by the LEMON_SOURCE_ROOT_DIR variable.
# If LEMON isn't there, then CMAKE will try to find an installed
# version of LEMON. If it is installed at some non-standard place,
# then you must tell its location in the LEMON_ROOT_DIR CMAKE config
# variable. (Do not hard code it into your config! Others may keep
# LEMON at different places.)

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


## RapidJSON
# Search for RapidJSON under rapidjson and deps/rapidjson.
# If found, add it as a header-only library.
FIND_PATH(RAPIDJSON_SOURCE_ROOT_DIR CMakeLists.txt
  PATHS ${CMAKE_SOURCE_DIR}/rapidjson ${CMAKE_SOURCE_DIR}/deps/rapidjson
  NO_DEFAULT_PATH
  DOC "Location of RapidJSON source as a CMAKE subproject")

IF(EXISTS ${RAPIDJSON_SOURCE_ROOT_DIR})
  SET(RAPIDJSON_INCLUDE_DIRS
    ${RAPIDJSON_SOURCE_ROOT_DIR}/include
    ${CMAKE_BINARY_DIR}/deps/rapidjson/include
  )
  SET(RAPIDJSON_LIBRARIES RapidJSON)

  FILE(GLOB RAPIDJSON_HEADERS ${RAPIDJSON_INCLUDE_DIRS}/rapidjson/*.h)
  ADD_LIBRARY(${RAPIDJSON_LIBRARIES} "${RAPIDJSON_HEADERS}")
  SET_TARGET_PROPERTIES(${RAPIDJSON_LIBRARIES} PROPERTIES LINKER_LANGUAGE CXX)
  TARGET_INCLUDE_DIRECTORIES(${RAPIDJSON_LIBRARIES} INTERFACE ${RAPIDJSON_INCLUDE_DIR})
ELSE()
  FIND_PACKAGE(RAPIDJSON QUIET NO_MODULE)
  FIND_PACKAGE(RAPIDJSON REQUIRED)
ENDIF()


## Doxygen
FIND_PACKAGE(Doxygen)


## LEARNINGNET_LIBRARIES
# Collect all hpp/cpp-files in ./include/learningnet/ and ./src/learningnet.
FILE(GLOB LN_HEADERS include/learningnet/*.hpp)
FILE(GLOB LN_SOURCES src/learningnet/*.cpp)
SET(LN_SOURCES "${LN_HEADERS};${LN_SOURCES}")

# Create the library, link it with lemon, set its include-dir as ./include.
ADD_LIBRARY(${LEARNINGNET_LIBRARIES} "${LN_SOURCES}")
TARGET_LINK_LIBRARIES(${LEARNINGNET_LIBRARIES} ${LEMON_LIBRARIES})
TARGET_INCLUDE_DIRECTORIES(${LEARNINGNET_LIBRARIES} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)


### EXECUTABLE
# Compile main.cpp to a binary with name ${EXECUTABLE}.
ADD_EXECUTABLE(${EXECUTABLE} main.cpp)

# Link ${EXECUTABLE} with lemon and learningnet.
TARGET_LINK_LIBRARIES(${EXECUTABLE} ${LEMON_LIBRARIES})
TARGET_LINK_LIBRARIES(${EXECUTABLE} ${LEARNINGNET_LIBRARIES})
TARGET_LINK_LIBRARIES(${EXECUTABLE} ${RAPIDJSON_LIBRARIES})


# This tells cmake to install the binary to $PREFIX/bin when 'make install'
# is executed. You can give more targets separated by whitespaces.
INSTALL(
  TARGETS learningnet-pathfinder
  RUNTIME DESTINATION ${INSTALL_BIN_DIR}
  COMPONENT bin
)

# Set warning flags
IF(CMAKE_COMPILER_IS_GNUCXX)
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

# Sometimes MSVC overwhelms you with compiler warnings which are impossible to
# avoid. Then comment out these sections. Normally you won't need it as the
# LEMON include headers suppress these warnings anyway.

#IF(MSVC)
#  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}
#      /wd4250 /wd4355 /wd4503 /wd4800 /wd4996")
# # Suppressed warnings:
# # C4250: 'class1' : inherits 'class2::member' via dominance
# # C4355: 'this' : used in base member initializer list
# # C4503: 'function' : decorated name length exceeded, name was truncated
# # C4800: 'type' : forcing value to bool 'true' or 'false'
# #        (performance warning)
# # C4996: 'function': was declared deprecated
# ENDIF(MSVC)

ENABLE_TESTING()

## The auxiliary doxygen files (.dox) should be placed in the 'doc'
## subdirectory. The next line includes the CMAKE config of that directory.

ADD_SUBDIRECTORY(doc)

#######################################################################
## CPACK configuration
##
## It is used to configure the .exe installer created by CPACK.
## Consider editing these values:
##
## - CPACK_PACKAGE_VENDOR
## - CPACK_PACKAGE_DESCRIPTION_SUMMARY
## - CPACK_NSIS_HELP_LINK
## - CPACK_NSIS_URL_INFO_ABOUT
## - CPACK_NSIS_CONTACT
##
## Additionally, you may want to change the icons/images used by the
## NSIS installer, i.e. these variables:
##
## - CPACK_NSIS_MUI_ICON
## - CPACK_PACKAGE_ICON
## - CPACK_NSIS_INSTALLED_ICON_NAME
##
## and/or the files they point to.
#######################################################################

IF(${CMAKE_SOURCE_DIR} STREQUAL ${PROJECT_SOURCE_DIR})
  SET(CPACK_PACKAGE_NAME ${PROJECT_NAME})
  SET(CPACK_PACKAGE_VENDOR "EGRES")
  SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY
      "LEMON PROJECT TEMPLATE - A Template Build Environment for LEMON")
  SET(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")

  SET(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})

  SET(CPACK_PACKAGE_INSTALL_DIRECTORY
      "${PROJECT_NAME}")
  SET(CPACK_PACKAGE_INSTALL_REGISTRY_KEY
      "${PROJECT_NAME} ${PROJECT_VERSION}")

  SET(CPACK_COMPONENTS_ALL bin doc)

  SET(CPACK_COMPONENT_BIN_DISPLAY_NAME "Executables")
  SET(CPACK_COMPONENT_BIN_DESCRIPTION
      "Command line utilities")
  SET(CPACK_COMPONENT_BIN_REQUIRED TRUE)

  SET(CPACK_COMPONENT_DOC_DISPLAY_NAME "Documentation")
  SET(CPACK_COMPONENT_DOC_DESCRIPTION
      "Documentation generated by Doxygen.")


  SET(CPACK_GENERATOR "NSIS")

  SET(CPACK_NSIS_MUI_ICON "${PROJECT_SOURCE_DIR}/cmake/nsis/lemon-project.ico")
  SET(CPACK_NSIS_MUI_UNIICON "${PROJECT_SOURCE_DIR}/cmake/nsis/uninstall.ico")
  SET(CPACK_PACKAGE_ICON "${PROJECT_SOURCE_DIR}/cmake/nsis\\\\installer.bmp")
  SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\lemon-project.ico")

  SET(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} ${PROJECT_NAME}")
  SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\lemon.cs.elte.hu")
  SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\lemon.cs.elte.hu")
  SET(CPACK_NSIS_CONTACT "lemon-user@lemon.cs.elte.hu")
  SET(CPACK_NSIS_CREATE_ICONS_EXTRA "
      CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Documentation.lnk\\\" \\\"$INSTDIR\\\\share\\\\doc\\\\index.html\\\"
      ")
  SET(CPACK_NSIS_DELETE_ICONS_EXTRA "
      !insertmacro MUI_STARTMENU_GETFOLDER Application $MUI_TEMP
      Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Documentation.lnk\\\"
      ")

  INCLUDE(CPack)
ENDIF(${CMAKE_SOURCE_DIR} STREQUAL ${PROJECT_SOURCE_DIR})
