#ifndef FS_H
#define FS_H

#include <stddef.h>
#include <pthread.h>
#include "minios.h"

#define MINI_OS_NAME_MAX 32
#define MINI_OS_DEFAULT_USER "os"
#define MINI_OS_DEFAULT_GROUP "os"

typedef enum FsNodeType {
    FS_DIRECTORY,
    FS_FILE
} FsNodeType;

/* 가상 파일 시스템의 파일/디렉터리를 표현하는 링크 트리 노드이다. */
typedef struct FsNode {
    char name[256];
    FsNodeType type;
    int depth;
    int permissions;
    char user[MINI_OS_NAME_MAX];
    char group[MINI_OS_NAME_MAX];
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
    char current_user[MINI_OS_NAME_MAX];
    char current_group[MINI_OS_NAME_MAX];
    pthread_mutex_t fs_lock;
};

FsNode *fs_create_node(const char *name, FsNodeType type);
void fs_destroy_tree(FsNode *node);

MiniOsStatus fs_load(MiniOsContext *ctx, const char *path);
MiniOsStatus fs_save(const MiniOsContext *ctx, const char *path);

FsNode *fs_find_child(FsNode *directory, const char *name);
MiniOsStatus fs_add_child(FsNode *directory, FsNode *child);

#endif
