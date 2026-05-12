#ifndef SHELL_H
#define SHELL_H

#include "minios.h"

typedef MiniOsStatus (*CommandHandler)(MiniOsContext *ctx, int argc, char **argv);

/* 하나의 명령어가 어떤 이름, 처리 함수, 설명을 가지는지 나타낸다. */
typedef struct CommandEntry {
    const char *name;
    CommandHandler handler;
    const char *summary;
} CommandEntry;

MiniOsStatus shell_run(MiniOsContext *ctx);
MiniOsStatus shell_dispatch(MiniOsContext *ctx, int argc, char **argv);

#endif
