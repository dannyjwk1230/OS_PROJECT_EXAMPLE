#ifndef MINIOS_H
#define MINIOS_H

#define MINI_OS_MAX_ARGS 32
#define MINI_OS_MAX_INPUT 1024

typedef enum MiniOsStatus {
    MINI_OS_OK = 0,
    MINI_OS_ERROR = 1,
    MINI_OS_NOT_IMPLEMENTED = 2
} MiniOsStatus;

typedef struct MiniOsContext MiniOsContext;

MiniOsStatus minios_init(MiniOsContext *ctx);
void minios_shutdown(MiniOsContext *ctx);

#endif
