#include <stdio.h>
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

static void get_permissions_str(int perms, FsNodeType type, char *str)
{
    str[0] = (type == FS_DIRECTORY) ? 'd' : '-';
    str[1] = (perms & 0400) ? 'r' : '-';
    str[2] = (perms & 0200) ? 'w' : '-';
    str[3] = (perms & 0100) ? 'x' : '-';
    str[4] = (perms & 0040) ? 'r' : '-';
    str[5] = (perms & 0020) ? 'w' : '-';
    str[6] = (perms & 0010) ? 'x' : '-';
    str[7] = (perms & 0004) ? 'r' : '-';
    str[8] = (perms & 0002) ? 'w' : '-';
    str[9] = (perms & 0001) ? 'x' : '-';
    str[10] = '\0';
}

MiniOsStatus cmd_ls(MiniOsContext *ctx, int argc, char **argv)
{
    if (ctx == NULL) return MINI_OS_ERROR;

    int aflag = 0;
    int lflag = 0;
    const char *path = NULL;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-' && argv[i][1] != '\0') {
            for (int j = 1; argv[i][j] != '\0'; j++) {
                if (argv[i][j] == 'a') aflag = 1;
                else if (argv[i][j] == 'l') lflag = 1;
                else {
                    printf("ls: invalid option -- '%c'\n", argv[i][j]);
                    return MINI_OS_ERROR;
                }
            }
        } else {
            if (path != NULL) {
                printf("ls: too many arguments\n");
                return MINI_OS_ERROR;
            }
            path = argv[i];
        }
    }

    pthread_mutex_lock(&ctx->fs_lock);

    FsNode *target = path ? resolve_path(ctx, path) : ctx->current;
    if (target == NULL) {
        pthread_mutex_unlock(&ctx->fs_lock);
        printf("ls: cannot access '%s': No such file or directory\n", path);
        return MINI_OS_ERROR;
    }

    if (target->type == FS_FILE) {
        if (lflag) {
            char perm_str[12];
            char time_buf[26];
            get_permissions_str(target->permissions, target->type, perm_str);
            struct tm *tm_info = localtime(&target->modified_time);
            strftime(time_buf, sizeof(time_buf), "%b %d %H:%M", tm_info);
            printf("%s %s %s %8zu %s %s\n", perm_str, target->user, target->group, target->content_size, time_buf, target->name);
        } else {
            printf("%s\n", target->name);
        }
        pthread_mutex_unlock(&ctx->fs_lock);
        return MINI_OS_SUCCESS;
    }

    if (aflag) {
        char perm_str[12];
        char time_buf[26];
        struct tm *tm_info;

        if (lflag) {
            get_permissions_str(target->permissions, target->type, perm_str);
            tm_info = localtime(&target->modified_time);
            strftime(time_buf, sizeof(time_buf), "%b %d %H:%M", tm_info);
            printf("%s %s %s %8d %s .\n", perm_str, target->user, target->group, 0, time_buf);

            FsNode *parent = target->parent ? target->parent : target;
            get_permissions_str(parent->permissions, parent->type, perm_str);
            tm_info = localtime(&parent->modified_time);
            strftime(time_buf, sizeof(time_buf), "%b %d %H:%M", tm_info);
            printf("%s %s %s %8d %s ..\n", perm_str, parent->user, parent->group, 0, time_buf);
        } else {
            printf(".   ..   ");
        }
    }

    FsNode *child = target->first_child;
    while (child != NULL) {
        if (lflag) {
            char perm_str[12];
            char time_buf[26];
            get_permissions_str(child->permissions, child->type, perm_str);
            struct tm *tm_info = localtime(&child->modified_time);
            strftime(time_buf, sizeof(time_buf), "%b %d %H:%M", tm_info);
            printf("%s %s %s %8zu %s %s\n", perm_str, child->user, child->group, child->content_size, time_buf, child->name);
        } else {
            printf("%s   ", child->name);
        }
        child = child->next_sibling;
    }

    if (!lflag) {
        printf("\n");
    }

    pthread_mutex_unlock(&ctx->fs_lock);
    return MINI_OS_SUCCESS;
}
