#pragma once

#include <string>

#define SCREEN_WIDTH 3520
#define SCREEN_HEIGHT 1600
#define DATA_DIRECTORY std::string("${STRATA_SOURCE_DIR}/data/")

#ifndef NDEBUG
#cmakedefine DEBUG
#endif

