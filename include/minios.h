#ifndef MINIOS_H
#define MINIOS_H

#define MINI_OS_MAX_ARGS 32
#define MINI_OS_MAX_INPUT 1024

typedef enum MiniOsStatus {
    MINI_OS_SUCCESS = 0,
    MINI_OS_ERROR = 1
} MiniOsStatus;

typedef struct MiniOsContext MiniOsContext;

MiniOsStatus minios_on(MiniOsContext *ctx);
void minios_off(MiniOsContext *ctx);

#endif
