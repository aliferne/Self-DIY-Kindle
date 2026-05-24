#pragma once

#include "cmsis_os.h"

/*
 * system handler, handling system-level operations, such as delay, etc.
 */

#define Delay(x) osDelay(x)

