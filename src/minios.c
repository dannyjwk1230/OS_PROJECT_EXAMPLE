#include "fs.h"
#include <string.h>

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

    /* Mini OS 실행 컨텍스트의 기본 사용자/그룹을 설정한다. */
    strncpy(ctx->current_user, MINI_OS_DEFAULT_USER, MINI_OS_NAME_MAX - 1);
    ctx->current_user[MINI_OS_NAME_MAX - 1] = '\0';
    strncpy(ctx->current_group, MINI_OS_DEFAULT_GROUP, MINI_OS_NAME_MAX - 1);
    ctx->current_group[MINI_OS_NAME_MAX - 1] = '\0';

    /* 이후 다중 작업에서 파일시스템 트리를 보호할 mutex를 준비한다. */
    if (pthread_mutex_init(&ctx->fs_lock, NULL) != 0) {
        fs_destroy_tree(ctx->root);
        ctx->root = NULL;
        ctx->current = NULL;
        return MINI_OS_ERROR;
    }

    return MINI_OS_SUCCESS;
}

void minios_off(MiniOsContext *ctx)
{
    /* 루트부터 연결된 모든 파일/디렉터리 노드를 재귀적으로 해제한다. */
    fs_destroy_tree(ctx->root);

    /* 파일시스템 보호용 mutex도 함께 정리한다. */
    pthread_mutex_destroy(&ctx->fs_lock);

    /* 해제된 포인터를 다시 사용하지 않도록 비워 둔다. */
    ctx->root = NULL;
    ctx->current = NULL;
}
