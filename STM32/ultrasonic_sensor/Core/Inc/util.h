#pragma once

#include "results.h"

#define ARRAY_LENGTH(arr) (sizeof(arr)/sizeof((arr)[0]))

#define app_assert_ok(expr) ((expr == R_OK) ? (void) 0 : __app_abort(__FILE__, __LINE__))
#define app_abort() __app_abort(__FILE__, __LINE__)

void __app_abort(const char *file, int line);
