#include <stdio.h>
#include "commands/basic.h"

MiniOsStatus cmd_pwd(MiniOsContext *ctx, int argc, char **argv)
{
    /* TODO: 루트부터 현재 디렉터리까지의 경로를 출력한다. */
    (void)ctx;
    (void)argc;
    (void)argv;
    printf("pwd: TODO implement current path printing\n");
    return MINI_OS_NOT_IMPLEMENTED;
}
