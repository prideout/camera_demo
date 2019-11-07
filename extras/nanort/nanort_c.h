#ifndef NANORT_C_H
#define NANORT_C_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void foo();

#ifdef __cplusplus
}
#endif

// -----------------------------------------------------------------------------
// END PUBLIC API
// -----------------------------------------------------------------------------
#ifdef NANO_RT_C_IMPLEMENTATION

#include <vector>

void foo() {
  //
}

#endif  // NANO_RT_C_IMPLEMENTATION
#endif  // NANO_RT_C_H
