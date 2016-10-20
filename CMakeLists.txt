# Copyright 2015 by Matthijs van Dorp.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 2.6)
project(STRATA)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${STRATA_SOURCE_DIR})

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenAL REQUIRED)
find_package(Lua REQUIRED)
find_package(Selene REQUIRED)
find_package(Tinygame REQUIRED)

INCLUDE(FindPkgConfig)

PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
PKG_SEARCH_MODULE(SDL2IMAGE REQUIRED SDL2_image>=2.0.0)
PKG_SEARCH_MODULE(SDL2TTF REQUIRED SDL2_ttf>=2.0.0)
PKG_SEARCH_MODULE(SDL2NET REQUIRED SDL2_net>=2.0.0)

configure_file(config.h.cmake ${CMAKE_BINARY_DIR}/config.h)

set(CMAKE_CXX_FLAGS "-O2 -g -Wall -Wextra -Wshadow -ansi -pedantic -std=c++11 -ldl") # ldl for lua loadlib.c which uses it; c++11 for Selene
#set(CMAKE_CXX_FLAGS "-O3 -Wall -DNDEBUG")
#set(CMAKE_EXE_LINKER_FLAGS "-lrt")
#set(CMAKE_VERBOSE_MAKEFILE true)

include_directories(${OPENGL_INCLUDE_DIR})
link_directories(${OPENGL_LIBRARY_DIR})
include_directories(${GLEW_INCLUDE_DIR})
link_directories(${GLEW_LIBRARY_DIR})
include_directories(${OPENAL_INCLUDE_DIR})
include_directories(${SDL2_INCLUDE_DIRS})
link_directories(${SDL2_LIBRARY_DIRS})
include_directories(${SDL2_IMAGE_INCLUDE_DIRS})
link_directories(${SDL2_IMAGE_LIBRARY_DIRS})
include_directories(${SDL2_TTF_INCLUDE_DIRS})
link_directories(${SDL2_TTF_LIBRARY_DIRS})
include_directories(${SDL2_NET_INCLUDE_DIRS})
link_directories(${SDL2_NET_LIBRARY_DIRS})
include_directories(${LUA_INCLUDE_DIR})
link_directories(${LUA_LIBRARY_DIR})
include_directories(${TINYGAME_INCLUDE_DIR})
link_directories(${TINYGAME_LIBRARY_DIR})
include_directories(${SELENE_INCLUDE_DIR})
include_directories(${STRATA_SOURCE_DIR})
include_directories(${STRATA_BINARY_DIR})
include_directories(${STRATA_SOURCE_DIR}/src/)

set(USED_LIBS 
              ${TINYGAME_LIBRARIES} # MUST be first, otherwise tinygame's calls to the below libraries don't get linked!
              ${LUA_LIBRARY}
              ${OPENGL_LIBRARY}
              ${GLEW_LIBRARY}
              ${OPENAL_LIBRARY}
              ${SDL2_LIBRARIES}
              ${SDL2IMAGE_LIBRARIES}
              ${SDL2TTF_LIBRARIES}
              ${SDL2NET_LIBRARIES})

file(GLOB STRATA_SOURCES RELATIVE ${STRATA_SOURCE_DIR}
	src/core/*.cpp
	src/interface/*.cpp
	src/mesh/*.cpp
	src/ui/*.cpp
)

add_executable(strata src/strata.cpp ${STRATA_SOURCES})
target_link_libraries(strata ${USED_LIBS})

add_executable(tests src/tests.cpp ${STRATA_SOURCES})
target_link_libraries(tests ${USED_LIBS})

