#include <stdlib.h>
#include <string.h>
#include "fs.h"
#include "storage.h"

FsNode *fs_create_node(const char *name, FsNodeType type)
{
    /* 파일 또는 디렉터리를 표현할 트리 노드를 하나 생성한다. */
    FsNode *node = (FsNode *)calloc(1, sizeof(FsNode));

    if (node == 0) {
        return 0;
    }

    /* 노드 이름, 종류, 기본 권한을 초기화한다. */
    strncpy(node->name, name, sizeof(node->name) - 1);
    node->type = type;
    node->permissions = 0755;

    return node;
}

void fs_destroy_tree(FsNode *node)
{
    if (node == 0) {
        return;
    }

    /* 자식과 형제 노드를 먼저 정리한 뒤 현재 노드를 해제한다. */
    fs_destroy_tree(node->first_child);
    fs_destroy_tree(node->next_sibling);
    free(node->content);
    free(node);
}

MiniOsStatus fs_load(MiniOsContext *ctx, const char *path)
{
    /* 실제 저장/복원 형식은 storage 모듈에서 담당한다. */
    return storage_load_tree(ctx, path);
}

MiniOsStatus fs_save(const MiniOsContext *ctx, const char *path)
{
    /* 현재 트리 상태를 파일로 저장하는 작업을 storage 모듈에 위임한다. */
    return storage_save_tree(ctx, path);
}

FsNode *fs_find_child(FsNode *directory, const char *name)
{
    /* TODO: directory의 자식 목록에서 name과 일치하는 노드를 찾는다. */
    (void)directory;
    (void)name;
    return 0;
}

MiniOsStatus fs_add_child(FsNode *directory, FsNode *child)
{
    /* TODO: child를 directory의 자식 연결 리스트에 추가한다. */
    (void)directory;
    (void)child;
    return MINI_OS_NOT_IMPLEMENTED;
}
