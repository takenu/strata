# Loosely based on https://github.com/LuaDist/lua/blob/master/cmake/FindLua.cmake (as of May 9, 2015).
# This module defines
#  LUA_LIBRARIES
#  LUA_FOUND, if false, do not try to link to Lua 
#  LUA_INCLUDE_DIR, where to find lua.h 
#
# Some say that the expected include convention is
#  #include "lua.h"
# and not
#  #include <lua/lua.h>
# since the lua location is not standardized and may exist
# in locations other than lua/. Chathran includes as
#  #include <lua.h>
# anyways, because of consistent notation.

FIND_PATH(LUA_INCLUDE_DIR lua.h
  PATHS
  /usr/local/include
  /usr/include
  /opt/include
)

FIND_LIBRARY(LUA_LIBRARY
  NAMES lua53 lua5.3 liblua.a
  PATHS
  /usr/local/lib
  /usr/lib
  /opt/local/lib
)

#IF(LUA_LIBRARY)
  # include the math library for Unix
#  IF(UNIX AND NOT APPLE)
#    FIND_LIBRARY(LUA_MATH_LIBRARY m)
#    SET( LUA_LIBRARIES "${LUA_LIBRARY};${LUA_MATH_LIBRARY}" CACHE STRING "Lua Libraries")
  # For Windows and Mac, don't need to explicitly include the math library
#  ELSE(UNIX AND NOT APPLE)
#    SET( LUA_LIBRARIES "${LUA_LIBRARY}" CACHE STRING "Lua Libraries")
#  ENDIF(UNIX AND NOT APPLE)
#ENDIF(LUA_LIBRARY)

INCLUDE(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LUA_FOUND to TRUE if 
# all listed variables are TRUE
#FIND_PACKAGE_HANDLE_STANDARD_ARGS(Lua53  DEFAULT_MSG  LUA_LIBRARIES LUA_INCLUDE_DIR)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Lua53  DEFAULT_MSG  LUA_INCLUDE_DIR)

#MARK_AS_ADVANCED(LUA_INCLUDE_DIR LUA_LIBRARIES LUA_LIBRARY LUA_MATH_LIBRARY)
MARK_AS_ADVANCED(LUA_INCLUDE_DIR LUA_LIBRARIES LUA_LIBRARY)
