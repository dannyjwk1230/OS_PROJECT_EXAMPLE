#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include "commands/basic.h"
#include "fs.h"

static FsNode *resolve_path(MiniOsContext *ctx, const char *path)
{
    char buf[1024];
    char *tok;
    char *save = NULL;
    FsNode *cur;
    size_t len;

    if (ctx == NULL || path == NULL || path[0] == '\0') return NULL;
    if (strcmp(path, "/") == 0) return ctx->root;

    strncpy(buf, path, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    len = strlen(buf);
    while (len > 1 && buf[len - 1] == '/') {
        buf[len - 1] = '\0';
        len--;
    }

    cur = (buf[0] == '/') ? ctx->root : ctx->current;
    tok = strtok_r(buf, "/", &save);

    while (tok != NULL) {
        if (strcmp(tok, ".") == 0) {
            tok = strtok_r(NULL, "/", &save);
            continue;
        }
        if (strcmp(tok, "..") == 0) {
            if (cur->parent != NULL) cur = cur->parent;
            tok = strtok_r(NULL, "/", &save);
            continue;
        }
        cur = fs_find_child(cur, tok);
        if (cur == NULL) return NULL;
        tok = strtok_r(NULL, "/", &save);
    }

    return cur;
}

MiniOsStatus cmd_touch(MiniOsContext *ctx, int argc, char **argv)
{
    if (ctx == NULL) return MINI_OS_ERROR;

    if (argc < 2) {
        printf("touch: missing operand\n");
        return MINI_OS_ERROR;
    }

    for (int i = 1; i < argc; i++) {
        char *path = argv[i];

        pthread_mutex_lock(&ctx->fs_lock);

        FsNode *existing = resolve_path(ctx, path);
        if (existing != NULL) {
            existing->modified_time = time(NULL);
            pthread_mutex_unlock(&ctx->fs_lock);
            continue;
        }

        char path_copy[1024];
        strncpy(path_copy, path, sizeof(path_copy) - 1);
        path_copy[sizeof(path_copy) - 1] = '\0';

        char *last_slash = strrchr(path_copy, '/');
        FsNode *parent = NULL;
        char *file_name = NULL;

        if (last_slash == NULL) {
            parent = ctx->current;
            file_name = path_copy;
        } else {
            if (last_slash == path_copy) {
                parent = ctx->root;
                file_name = last_slash + 1;
            } else {
                *last_slash = '\0';
                parent = resolve_path(ctx, path_copy);
                file_name = last_slash + 1;
            }
        }

        if (parent == NULL || file_name == NULL || *file_name == '\0') {
            printf("touch: cannot touch '%s': No such file or directory\n", path);
            pthread_mutex_unlock(&ctx->fs_lock);
            continue;
        }

        if (parent->type != FS_DIRECTORY) {
            printf("touch: cannot touch '%s': Not a directory\n", path);
            pthread_mutex_unlock(&ctx->fs_lock);
            continue;
        }

        FsNode *new_file = fs_create_node(file_name, FS_FILE);
        if (new_file == NULL) {
            pthread_mutex_unlock(&ctx->fs_lock);
            return MINI_OS_ERROR;
        }
        
        new_file->depth = parent->depth + 1;
        new_file->content = NULL;
        new_file->content_size = 0;

        if (fs_add_child(parent, new_file) != MINI_OS_SUCCESS) {
            fs_destroy_tree(new_file);
            printf("touch: failed to create file '%s'\n", path);
        }

        pthread_mutex_unlock(&ctx->fs_lock);
    }

    return MINI_OS_SUCCESS;
}
