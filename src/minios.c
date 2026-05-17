#include "fs.h"

MiniOsStatus minios_on(MiniOsContext *ctx)
{
    /* 가상 파일 시스템의 시작점인 루트 디렉터리를 만든다. */
    ctx->root = fs_create_node("/", FS_DIRECTORY);

    if (ctx->root == NULL) {
        return MINI_OS_ERROR;
    }

    ctx->root->depth = 0;

    /* 프로그램 시작 시 현재 위치는 루트 디렉터리로 설정한다. */
    ctx->current = ctx->root;

    return MINI_OS_SUCCESS;
}

void minios_off(MiniOsContext *ctx)
{
    /* 루트부터 연결된 모든 파일/디렉터리 노드를 재귀적으로 해제한다. */
    fs_destroy_tree(ctx->root);

    /* 해제된 포인터를 다시 사용하지 않도록 비워 둔다. */
    ctx->root = NULL;
    ctx->current = NULL;
}
