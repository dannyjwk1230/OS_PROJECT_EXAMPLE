#include <stdio.h>
#include "commands/even.h"

MiniOsStatus cmd_clear(MiniOsContext *ctx, int argc, char **argv)
{
    (void)ctx;
    (void)argc;
    (void)argv;

    printf("\033[2J\033[H");
    return MINI_OS_SUCCESS;
}
