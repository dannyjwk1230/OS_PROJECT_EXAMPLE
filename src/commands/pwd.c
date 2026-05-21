#include <stdio.h>
#include <pthread.h>
#include "commands/basic.h"
#include "fs.h"

MiniOsStatus cmd_pwd(MiniOsContext *ctx, int argc, char **argv)
{
    (void)argc;
    (void)argv;

    if (ctx == NULL) return MINI_OS_ERROR;

    pthread_mutex_lock(&ctx->fs_lock);

    FsNode *cur = ctx->current;
    const char *path_stack[256];
    int depth = 0;

    /* 부모를 타고 루트까지 거슬러 올라감 */
    while (cur != NULL && cur != ctx->root) {
        path_stack[depth++] = cur->name;
        cur = cur->parent;
    }

    /* 출력 (루트면 /만 출력, 아니면 역순으로 /이름 조립) */
    if (depth == 0) {
        printf("/\n");
    } else {
        for (int i = depth - 1; i >= 0; i--) {
            printf("/%s", path_stack[i]);
        }
        printf("\n");
    }

    pthread_mutex_unlock(&ctx->fs_lock);
    return MINI_OS_SUCCESS;
}