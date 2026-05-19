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

MiniOsStatus cmd_chmod(MiniOsContext *ctx, int argc, char **argv)
{
    if (ctx == NULL) return MINI_OS_ERROR;

    if (argc != 3) {
        printf("chmod: usage: chmod <permissions> <file_or_dir>\n");
        return MINI_OS_ERROR;
    }

    /* 권한 값을 8진수(octal)로 파싱 */
    char *endptr;
    long new_perms = strtol(argv[1], &endptr, 8);
    if (*endptr != '\0') {
        printf("chmod: invalid mode: '%s'\n", argv[1]);
        return MINI_OS_ERROR;
    }

    pthread_mutex_lock(&ctx->fs_lock);

    FsNode *target = resolve_path(ctx, argv[2]);
    if (target == NULL) {
        pthread_mutex_unlock(&ctx->fs_lock);
        printf("chmod: cannot access '%s': No such file or directory\n", argv[2]);
        return MINI_OS_ERROR;
    }

    /* 권한 변경 */
    target->permissions = (int)new_perms;

    pthread_mutex_unlock(&ctx->fs_lock);
    return MINI_OS_SUCCESS;
}