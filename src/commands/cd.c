#include <stdio.h>
#include "commands/basic.h"

MiniOsStatus cmd_cd(MiniOsContext *ctx, int argc, char **argv)
{
    /* TODO: 현재 작업 디렉터리를 변경하고 ., .. 경로 처리를 구현한다. */
    (void)ctx;
    (void)argc;
    (void)argv;
    printf("cd: TODO implement ., .. path handling\n");
    return MINI_OS_SUCCESS;
}
