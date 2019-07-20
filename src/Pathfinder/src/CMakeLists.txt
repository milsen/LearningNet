INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_BINARY_DIR}/src
)

# Here we define an executable target.
ADD_EXECUTABLE(learningnet-pathfinder
    main.cc
)

# This tells cmake to install 'lemon-project' to $PREFIX/bin when
# 'make install' is executed. You can give more targets separated
# by whitespaces.
TARGET_LINK_LIBRARIES(learningnet-pathfinder
    ${LEMON_LIBRARIES}
)

INSTALL(
  TARGETS learningnet-pathfinder
  RUNTIME DESTINATION ${INSTALL_BIN_DIR}
  COMPONENT bin
)
