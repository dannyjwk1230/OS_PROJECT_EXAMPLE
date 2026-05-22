#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "commands/even.h"
#include "fs.h"

static int is_empty_dir(FsNode *n)
{
    return n != NULL && n->type == FS_DIRECTORY && n->first_child == NULL;
}

static FsNode *resolve_path(MiniOsContext *ctx, const char *path)
{
    char buf[1024];
    char *tok;
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
    tok = strtok(buf, "/");

    while (tok != NULL) {
        if (strcmp(tok, ".") == 0) {
            tok = strtok(NULL, "/");
            continue;
        }

        if (strcmp(tok, "..") == 0) {
            if (cur->parent != NULL) cur = cur->parent;
            tok = strtok(NULL, "/");
            continue;
        }

        cur = fs_find_child(cur, tok);
        if (cur == NULL) return NULL;

        tok = strtok(NULL, "/");
    }

    return cur;
}

static MiniOsStatus detach_from_parent(FsNode *node)
{
    FsNode *p;
    FsNode *c;

    if (node == NULL || node->parent == NULL) return MINI_OS_ERROR;

    p = node->parent;
    c = p->first_child;

    if (c == node) {
        p->first_child = node->next_sibling;
        node->next_sibling = NULL;
        node->parent = NULL;
        return MINI_OS_SUCCESS;
    }

    while (c != NULL && c->next_sibling != node) c = c->next_sibling;
    if (c == NULL) return MINI_OS_ERROR;

    c->next_sibling = node->next_sibling;
    node->next_sibling = NULL;
    node->parent = NULL;

    return MINI_OS_SUCCESS;
}

MiniOsStatus cmd_rmdir(MiniOsContext *ctx, int argc, char **argv)
{
    int pflag = 0;
    const char *path;
    FsNode *target;
    FsNode *cur;
    FsNode *up;

    if (ctx == NULL) return MINI_OS_ERROR;

    if (argc < 2) {
        printf("rmdir: missing operand\n");
        return MINI_OS_ERROR;
    }

    if (strcmp(argv[1], "-p") == 0) {
        pflag = 1;

        if (argc != 3) {
            printf("rmdir: usage: rmdir -p <dir>\n");
            return MINI_OS_ERROR;
        }

        path = argv[2];
    } else {
        if (argc != 2) {
            printf("rmdir: usage: rmdir <dir>\n");
            return MINI_OS_ERROR;
        }

        path = argv[1];
    }

    pthread_mutex_lock(&ctx->fs_lock);

    target = resolve_path(ctx, path);
    if (target == NULL) {
        pthread_mutex_unlock(&ctx->fs_lock);
        printf("rmdir: failed to remove '%s': No such directory\n", path);
        return MINI_OS_ERROR;
    }

    if (target == ctx->root) {
        pthread_mutex_unlock(&ctx->fs_lock);
        printf("rmdir: failed to remove '/': Operation not permitted\n");
        return MINI_OS_ERROR;
    }

    if (target == ctx->current) {
        pthread_mutex_unlock(&ctx->fs_lock);
        printf("rmdir: failed to remove '%s': Directory in use\n", path);
        return MINI_OS_ERROR;
    }

    if (target->type != FS_DIRECTORY) {
        pthread_mutex_unlock(&ctx->fs_lock);
        printf("rmdir: failed to remove '%s': Not a directory\n", path);
        return MINI_OS_ERROR;
    }

    if (!is_empty_dir(target)) {
        pthread_mutex_unlock(&ctx->fs_lock);
        printf("rmdir: failed to remove '%s': Directory not empty\n", path);
        return MINI_OS_ERROR;
    }

    if (!pflag) {
        if (detach_from_parent(target) != MINI_OS_SUCCESS) {
            pthread_mutex_unlock(&ctx->fs_lock);
            return MINI_OS_ERROR;
        }

        fs_destroy_tree(target);
        pthread_mutex_unlock(&ctx->fs_lock);
        return MINI_OS_SUCCESS;
    }

    cur = target;
    while (cur != NULL && cur != ctx->root) {
        if (cur == ctx->current) break;
        if (!is_empty_dir(cur)) break;

        up = cur->parent;

        if (detach_from_parent(cur) != MINI_OS_SUCCESS) {
            pthread_mutex_unlock(&ctx->fs_lock);
            return MINI_OS_ERROR;
        }

        fs_destroy_tree(cur);
        cur = up;
    }

    pthread_mutex_unlock(&ctx->fs_lock);
    return MINI_OS_SUCCESS;
}
