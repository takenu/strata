#pragma once

#include <string>

#define SCREEN_WIDTH 1620
#define SCREEN_HEIGHT 800
#define DATA_DIRECTORY std::string("${STRATA_SOURCE_DIR}/data/")

#ifndef NDEBUG
#cmakedefine DEBUG
#endif

