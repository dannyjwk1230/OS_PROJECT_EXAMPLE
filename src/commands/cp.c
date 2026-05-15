#include <stdio.h>
#include "commands/even.h"

MiniOsStatus cmd_cp(MiniOsContext *ctx, int argc, char **argv)
{
    /* TODO: 파일 또는 디렉터리 노드를 목적지에 복사한다. */
    (void)ctx;
    (void)argc;
    (void)argv;
    printf("cp: TODO implement file/directory copy\n");
    return MINI_OS_SUCCESS;
}
