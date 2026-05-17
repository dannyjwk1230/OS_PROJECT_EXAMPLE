#include <stdlib.h>
#include <string.h>
#include "fs.h"
#include "storage.h"

FsNode *fs_create_node(const char *name, FsNodeType type)
{
    /* 파일 또는 디렉터리를 표현할 트리 노드를 하나 생성한다. */
    FsNode *node = (FsNode *)calloc(1, sizeof(FsNode));

    if (node == NULL) {
        return NULL;
    }

    /* 노드 이름, 종류, 기본 권한을 초기화한다. */
    strncpy(node->name, name, sizeof(node->name) - 1);
    node->hidden = (name[0] == '.');
    node->type = type;
    node->permissions = 0755;

    return node;
}

void fs_destroy_tree(FsNode *node)
{
    if (node == NULL) {
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
    if (directory == NULL || name == NULL) 
    {
        return NULL;
    }

    /* 첫 번째 자식부터 형제 링크를 따라가며 같은 이름의 노드를 찾는다. */
    FsNode *child = directory->first_child;

    for (; child != NULL; child = child->next_sibling) 
    {
        if (strcmp(child->name, name) == 0) 
        {
            return child;
        }
    }

    return NULL;
}

MiniOsStatus fs_add_child(FsNode *directory, FsNode *child)
{
    if (directory == NULL || child == NULL || directory->type != FS_DIRECTORY) 
    {
        return MINI_OS_ERROR;
    }

    /* 자식 목록이 비어 있으면 첫 자식으로 연결하고, 아니면 마지막 형제 뒤에 붙인다. */
    FsNode *child_v = directory->first_child;
    child->parent = directory;
    child->next_sibling = NULL;
    child->depth = directory->depth + 1;

    if (child_v == NULL) 
    {
        directory->first_child = child;
        return MINI_OS_SUCCESS;
    }
    
    for (; child_v->next_sibling != NULL; child_v = child_v->next_sibling) {}
    child_v->next_sibling = child;

    return MINI_OS_SUCCESS;
}
