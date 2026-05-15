#include <stdio.h>
#include "commands/even.h"

MiniOsStatus cmd_clear(MiniOsContext *ctx, int argc, char **argv)
{
    /* TODO: Mini OS 화면을 지우는 동작을 구현한다. */
    (void)ctx;
    (void)argc;
    (void)argv;
    printf("clear: TODO implement terminal clearing\n");
    return MINI_OS_SUCCESS;
}
