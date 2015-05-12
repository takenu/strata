# Adapted from http://www.openengine.dk/code/extensions/Assimp/FindAssimp.cmake.
# - Try to find tiny-game-engine
# Once done this will define
#
#  TINYGAME_FOUND - system has tiny-game-engine installed
#  TINYGAME_INCLUDE_DIR - the tiny-game-engine include directory (root directory)
#  TINYGAME_LIBRARIES - the compiled files to be linked for using tiny-game-engine
#

SET(TINYGAME "tinygame")

FIND_PATH(TINYGAME_INCLUDE_DIR NAMES tiny/draw/renderable.h
  PATHS
  ${PROJECT_BINARY_DIR}/include
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/../tiny-game-engine                # this one added
  ENV CPATH
  /usr/include
  /usr/local/include
  /opt/local/include
  NO_DEFAULT_PATH
)
 

FIND_LIBRARY(LIBTINYGAME
  NAMES 
  "lib${TINYGAME}.a"
  PATHS
  ${PROJECT_SOURCE_DIR}/../tiny-game-engine/build/tiny/           # this one added
  ENV LD_LIBRARY_PATH
  ENV LIBRARY_PATH
  /usr/lib
  /usr/local/lib
  /opt/local/lib
  NO_DEFAULT_PATH
)

SET (TINYGAME_LIBRARIES
  ${LIBTINYGAME} 
)

IF(TINYGAME_INCLUDE_DIR AND TINYGAME_LIBRARIES)
   SET(TINYGAME_FOUND TRUE)
ENDIF(TINYGAME_INCLUDE_DIR AND TINYGAME_LIBRARIES)

