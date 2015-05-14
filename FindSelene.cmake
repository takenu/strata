# Try to find Selene. This will define:
#
#  SELENE_FOUND - system has a selene include dir
#  SELENE_INCLUDE_DIR - the selene include directory

FIND_PATH(SELENE_INCLUDE_DIR NAMES selene.h
  PATHS
  ${PROJECT_BINARY_DIR}/include
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/../selene/include/                # this one added
  ${PROJECT_SOURCE_DIR}/../Selene/include/                # this one added
  ENV CPATH
  /usr/include
  /usr/local/include
  /opt/local/include
  NO_DEFAULT_PATH
)
 
IF(SELENE_INCLUDE_DIR)
   SET(SELENE_FOUND TRUE)
ENDIF(SELENE_INCLUDE_DIR)

