#include <stdio.h>
#include "commands/even.h"

MiniOsStatus cmd_rmdir(MiniOsContext *ctx, int argc, char **argv)
{
    /* TODO: 빈 디렉터리를 삭제하고 -p 옵션으로 상위 빈 디렉터리까지 처리한다. */
    (void)ctx;
    (void)argc;
    (void)argv;
    printf("rmdir: TODO implement -p\n");
    return MINI_OS_NOT_IMPLEMENTED;
}
