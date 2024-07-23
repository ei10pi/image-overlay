#if !defined(SHARED_H)

#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int16_t s16;
typedef int32_t s32;

void error_exit(const char *error_message);

#define SHARED_H

#endif
