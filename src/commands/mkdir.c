#include <stdio.h>
#include "commands/basic.h"

MiniOsStatus cmd_mkdir(MiniOsContext *ctx, int argc, char **argv)
{
    /* TODO: 디렉터리를 생성하고 -p 및 다중 생성 시 멀티스레딩을 적용한다. */
    (void)ctx;
    (void)argc;
    (void)argv;
    printf("mkdir: TODO implement -p and threaded multi-directory creation\n");
    return MINI_OS_NOT_IMPLEMENTED;
}
