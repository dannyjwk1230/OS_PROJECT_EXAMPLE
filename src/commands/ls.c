#include <stdio.h>
#include "commands/basic.h"

MiniOsStatus cmd_ls(MiniOsContext *ctx, int argc, char **argv)
{
    /* TODO: 현재 디렉터리 또는 지정 경로의 목록을 출력하고 -a, -l, -al 옵션을 처리한다. */
    (void)ctx;
    (void)argc;
    (void)argv;
    printf("ls: TODO implement -a, -l, -al\n");
    return MINI_OS_SUCCESS;
}
