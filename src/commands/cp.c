#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "commands/even.h"
#include "fs.h"

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

static MiniOsStatus resolve_parent_and_name(
    MiniOsContext *ctx, const char *path, FsNode **parent_out, char *name_out, size_t cap)
{
    char buf[1024];
    char *last;
    FsNode *parent;
    size_t len;

    if (ctx == NULL || path == NULL || parent_out == NULL || name_out == NULL || cap == 0) {
        return MINI_OS_ERROR;
    }

    if (strlen(path) >= sizeof(buf)) return MINI_OS_ERROR;

    strncpy(buf, path, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    len = strlen(buf);
    while (len > 1 && buf[len - 1] == '/') {
        buf[len - 1] = '\0';
        len--;
    }

    if (strcmp(buf, "/") == 0) return MINI_OS_ERROR;

    last = strrchr(buf, '/');

    if (last == NULL) {
        parent = ctx->current;
        strncpy(name_out, buf, cap - 1);
        name_out[cap - 1] = '\0';
    } else if (last == buf) {
        parent = ctx->root;
        strncpy(name_out, last + 1, cap - 1);
        name_out[cap - 1] = '\0';
    } else {
        *last = '\0';
        parent = resolve_path(ctx, buf);
        strncpy(name_out, last + 1, cap - 1);
        name_out[cap - 1] = '\0';
    }

    if (parent == NULL || parent->type != FS_DIRECTORY || name_out[0] == '\0') {
        return MINI_OS_ERROR;
    }

    *parent_out = parent;
    return MINI_OS_SUCCESS;
}

static MiniOsStatus copy_file_content(FsNode *dst, const FsNode *src)
{
    if (src->type != FS_FILE) return MINI_OS_SUCCESS;

    if (src->content == NULL || src->content_size == 0) {
        dst->content = NULL;
        dst->content_size = 0;
        return MINI_OS_SUCCESS;
    }

    dst->content = (char *)malloc(src->content_size + 1);
    if (dst->content == NULL) return MINI_OS_ERROR;

    memcpy(dst->content, src->content, src->content_size);
    dst->content[src->content_size] = '\0';
    dst->content_size = src->content_size;

    return MINI_OS_SUCCESS;
}

static int is_ancestor(FsNode *a, FsNode *b)
{
    FsNode *cur = b;

    while (cur != NULL) {
        if (cur == a) return 1;
        cur = cur->parent;
    }

    return 0;
}

static MiniOsStatus deep_copy_into(FsNode *src, FsNode *dst_parent, const char *new_name)
{
    FsNode *copy;
    FsNode *ch;

    if (src == NULL || dst_parent == NULL || new_name == NULL) return MINI_OS_ERROR;
    if (dst_parent->type != FS_DIRECTORY) return MINI_OS_ERROR;
    if (fs_find_child(dst_parent, new_name) != NULL) return MINI_OS_ERROR;

    copy = fs_create_node(new_name, src->type);
    if (copy == NULL) return MINI_OS_ERROR;

    copy->permissions = src->permissions;
    strncpy(copy->user, src->user, sizeof(copy->user) - 1);
    copy->user[sizeof(copy->user) - 1] = '\0';
    strncpy(copy->group, src->group, sizeof(copy->group) - 1);
    copy->group[sizeof(copy->group) - 1] = '\0';
    copy->modified_time = src->modified_time;

    if (copy_file_content(copy, src) != MINI_OS_SUCCESS) {
        fs_destroy_tree(copy);
        return MINI_OS_ERROR;
    }

    if (fs_add_child(dst_parent, copy) != MINI_OS_SUCCESS) {
        fs_destroy_tree(copy);
        return MINI_OS_ERROR;
    }

    if (src->type == FS_DIRECTORY) {
        for (ch = src->first_child; ch != NULL; ch = ch->next_sibling) {
            if (deep_copy_into(ch, copy, ch->name) != MINI_OS_SUCCESS) {
                detach_from_parent(copy);
                fs_destroy_tree(copy);
                return MINI_OS_ERROR;
            }
        }
    }

    return MINI_OS_SUCCESS;
}

MiniOsStatus cmd_cp(MiniOsContext *ctx, int argc, char **argv)
{
    FsNode *src;
    FsNode *dst;
    FsNode *dst_parent;
    char new_name[256];

    if (ctx == NULL) return MINI_OS_ERROR;

    if (argc != 3) {
        printf("cp: usage: cp <source> <destination>\n");
        return MINI_OS_ERROR;
    }

    pthread_mutex_lock(&ctx->fs_lock);

    src = resolve_path(ctx, argv[1]);
    if (src == NULL) {
        pthread_mutex_unlock(&ctx->fs_lock);
        printf("cp: cannot stat '%s': No such file or directory\n", argv[1]);
        return MINI_OS_ERROR;
    }

    dst = resolve_path(ctx, argv[2]);

    if (dst != NULL) {
        if (dst->type != FS_DIRECTORY) {
            pthread_mutex_unlock(&ctx->fs_lock);
            printf("cp: destination '%s' exists and is not a directory\n", argv[2]);
            return MINI_OS_ERROR;
        }

        if (src->type == FS_DIRECTORY && is_ancestor(src, dst)) {
            pthread_mutex_unlock(&ctx->fs_lock);
            printf("cp: cannot copy a directory into itself\n");
            return MINI_OS_ERROR;
        }

        if (deep_copy_into(src, dst, src->name) != MINI_OS_SUCCESS) {
            pthread_mutex_unlock(&ctx->fs_lock);
            printf("cp: copy failed\n");
            return MINI_OS_ERROR;
        }

        pthread_mutex_unlock(&ctx->fs_lock);
        return MINI_OS_SUCCESS;
    }

    if (resolve_parent_and_name(ctx, argv[2], &dst_parent, new_name, sizeof(new_name)) != MINI_OS_SUCCESS) {
        pthread_mutex_unlock(&ctx->fs_lock);
        printf("cp: invalid destination '%s'\n", argv[2]);
        return MINI_OS_ERROR;
    }

    if (src->type == FS_DIRECTORY && is_ancestor(src, dst_parent)) {
        pthread_mutex_unlock(&ctx->fs_lock);
        printf("cp: cannot copy a directory into itself\n");
        return MINI_OS_ERROR;
    }

    if (deep_copy_into(src, dst_parent, new_name) != MINI_OS_SUCCESS) {
        pthread_mutex_unlock(&ctx->fs_lock);
        printf("cp: copy failed\n");
        return MINI_OS_ERROR;
    }

    pthread_mutex_unlock(&ctx->fs_lock);
    return MINI_OS_SUCCESS;
}
