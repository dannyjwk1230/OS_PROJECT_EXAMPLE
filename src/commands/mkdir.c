#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "commands/basic.h"
#include "fs.h"
#include "threading.h"

typedef struct MkdirTaskArg {
    MiniOsContext *ctx;
    char path[1024];
    int pflag;
} MkdirTaskArg;

static MiniOsStatus mkdir_single_path_task(void *arg)
{
    MkdirTaskArg *targ = (MkdirTaskArg *)arg;
    MiniOsContext *ctx = targ->ctx;
    char *path = targ->path;
    int pflag = targ->pflag;

    char buf[1024];
    strncpy(buf, path, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    size_t len = strlen(buf);
    while (len > 1 && buf[len - 1] == '/') {
        buf[len - 1] = '\0';
        len--;
    }

    char *tok;
    char *save = NULL;
    
    pthread_mutex_lock(&ctx->fs_lock);
    
    FsNode *cur = (buf[0] == '/') ? ctx->root : ctx->current;
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

        FsNode *child = fs_find_child(cur, tok);
        if (child == NULL) {
            char *next_tok = strtok_r(NULL, "/", &save);
            if (next_tok != NULL && !pflag) {
                printf("mkdir: cannot create directory '%s': No such file or directory\n", path);
                pthread_mutex_unlock(&ctx->fs_lock);
                return MINI_OS_ERROR;
            }
            
            FsNode *new_dir = fs_create_node(tok, FS_DIRECTORY);
            if (new_dir == NULL) {
                pthread_mutex_unlock(&ctx->fs_lock);
                return MINI_OS_ERROR;
            }
            new_dir->depth = cur->depth + 1;
            
            if (fs_add_child(cur, new_dir) != MINI_OS_SUCCESS) {
                fs_destroy_tree(new_dir);
                pthread_mutex_unlock(&ctx->fs_lock);
                return MINI_OS_ERROR;
            }
            cur = new_dir;
            tok = next_tok;
        } else {
            if (child->type != FS_DIRECTORY) {
                printf("mkdir: cannot create directory '%s': File exists\n", path);
                pthread_mutex_unlock(&ctx->fs_lock);
                return MINI_OS_ERROR;
            }
            cur = child;
            tok = strtok_r(NULL, "/", &save);
        }
    }

    pthread_mutex_unlock(&ctx->fs_lock);
    return MINI_OS_SUCCESS;
}

MiniOsStatus cmd_mkdir(MiniOsContext *ctx, int argc, char **argv)
{
    if (ctx == NULL) return MINI_OS_ERROR;

    int pflag = 0;
    int dir_start_idx = 1;

    if (argc < 2) {
        printf("mkdir: missing operand\n");
        return MINI_OS_ERROR;
    }

    if (strcmp(argv[1], "-p") == 0) {
        pflag = 1;
        dir_start_idx = 2;
        if (argc < 3) {
            printf("mkdir: missing operand after '-p'\n");
            return MINI_OS_ERROR;
        }
    }

    int task_count = argc - dir_start_idx;
    
    ThreadTask *tasks = (ThreadTask *)malloc(sizeof(ThreadTask) * task_count);
    void **args = (void **)malloc(sizeof(void *) * task_count);
    MkdirTaskArg *task_args = (MkdirTaskArg *)malloc(sizeof(MkdirTaskArg) * task_count);

    for (int i = 0; i < task_count; i++) {
        tasks[i] = mkdir_single_path_task;
        task_args[i].ctx = ctx;
        strncpy(task_args[i].path, argv[dir_start_idx + i], sizeof(task_args[i].path) - 1);
        task_args[i].path[sizeof(task_args[i].path) - 1] = '\0';
        task_args[i].pflag = pflag;
        args[i] = &task_args[i];
    }

    MiniOsStatus status = thread_run_tasks(tasks, args, task_count);

    free(tasks);
    free(args);
    free(task_args);

    return status;
}
