#ifndef DOBBY_H
#define DOBBY_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  RS_SUCCESS = 0,
  RS_FAILED
} RetStatus;

RetStatus DobbyHook(void *function_address, void *replace_call, void **origin_call);
RetStatus DobbyInstrument(void *instr_address, void *instr_handler);
RetStatus DobbyEnableNEONRegister();

#ifdef __cplusplus
}
#endif

#endif  // DOBBY_H
