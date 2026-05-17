#ifndef FS_H
#define FS_H

#include <stddef.h>
#include "minios.h"

typedef enum FsNodeType {
    FS_DIRECTORY,
    FS_FILE
} FsNodeType;

/* 가상 파일 시스템의 파일/디렉터리를 표현하는 링크 트리 노드이다. */
typedef struct FsNode {
    char name[256];
    FsNodeType type;
    int depth;
    int hidden;
    int permissions;
    char *content;
    size_t content_size;
    struct FsNode *parent;
    struct FsNode *first_child;
    struct FsNode *next_sibling;
} FsNode;

/* Mini OS 전체 실행 상태를 담는 구조체이다. */
struct MiniOsContext {
    FsNode *root;
    FsNode *current;
};

FsNode *fs_create_node(const char *name, FsNodeType type);
void fs_destroy_tree(FsNode *node);

MiniOsStatus fs_load(MiniOsContext *ctx, const char *path);
MiniOsStatus fs_save(const MiniOsContext *ctx, const char *path);

FsNode *fs_find_child(FsNode *directory, const char *name);
MiniOsStatus fs_add_child(FsNode *directory, FsNode *child);

#endif
