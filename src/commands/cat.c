#include <stdio.h>
#include "commands/basic.h"

MiniOsStatus cmd_cat(MiniOsContext *ctx, int argc, char **argv)
{
    /* TODO: 파일 내용을 출력하고 -n, > 리다이렉션을 처리한다. */
    (void)ctx;
    (void)argc;
    (void)argv;
    printf("cat: TODO implement > redirection and -n\n");
    return MINI_OS_SUCCESS;
}
