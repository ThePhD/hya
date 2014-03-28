#pragma once

#include "real.h"
#include <chrono>

typedef std::chrono::duration<real, std::milli> milliseconds;
typedef std::chrono::duration<real> seconds;
typedef std::chrono::duration<real, std::ratio<60>> minutes;
