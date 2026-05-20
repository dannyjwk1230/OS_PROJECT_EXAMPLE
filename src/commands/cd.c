#include <stdio.h>
#include <string.h>
#include <pthread.h>
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

MiniOsStatus cmd_cd(MiniOsContext *ctx, int argc, char **argv)
{
    if (ctx == NULL) return MINI_OS_ERROR;

    const char *path = "/"; 
    if (argc > 2) {
        printf("cd: too many arguments\n");
        return MINI_OS_ERROR;
    } else if (argc == 2) {
        path = argv[1];
    }

    pthread_mutex_lock(&ctx->fs_lock);

    FsNode *target = resolve_path(ctx, path);
    if (target == NULL) {
        pthread_mutex_unlock(&ctx->fs_lock);
        printf("cd: %s: No such file or directory\n", path);
        return MINI_OS_ERROR;
    }

    if (target->type != FS_DIRECTORY) {
        pthread_mutex_unlock(&ctx->fs_lock);
        printf("cd: %s: Not a directory\n", path);
        return MINI_OS_ERROR;
    }

    ctx->current = target;

    pthread_mutex_unlock(&ctx->fs_lock);
    return MINI_OS_SUCCESS;
}
