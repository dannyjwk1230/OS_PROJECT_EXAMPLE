#include <stdio.h>
#include "commands/even.h"

MiniOsStatus cmd_chmod(MiniOsContext *ctx, int argc, char **argv)
{
    /* TODO: 대상 파일/디렉터리의 권한 값을 변경한다. */
    (void)ctx;
    (void)argc;
    (void)argv;
    printf("chmod: TODO implement permission changes\n");
    return MINI_OS_NOT_IMPLEMENTED;
}
