#include <stdio.h>
#include <stdlib.h>
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

static MiniOsStatus resolve_parent_and_name(
    MiniOsContext *ctx, const char *path, FsNode **parent_out, char *name_out, size_t cap)
{
    char buf[1024];
    char *last;
    FsNode *parent;
    size_t len;

    if (ctx == NULL || path == NULL || parent_out == NULL || name_out == NULL || cap == 0) return MINI_OS_ERROR;
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

    if (parent == NULL || parent->type != FS_DIRECTORY || name_out[0] == '\0') return MINI_OS_ERROR;
    *parent_out = parent;
    return MINI_OS_SUCCESS;
}

MiniOsStatus cmd_cat(MiniOsContext *ctx, int argc, char **argv)
{
    if (ctx == NULL) return MINI_OS_ERROR;

    if (argc < 2) {
        printf("cat: usage: cat <file> | cat -n <file> | cat > <file>\n");
        return MINI_OS_ERROR;
    }

    int write_mode = 0;
    int line_number_mode = 0;
    const char *target_path = NULL;

    if (argc == 3 && strcmp(argv[1], ">") == 0) {
        write_mode = 1;
        target_path = argv[2];
    } else if (argc == 3 && strcmp(argv[1], "-n") == 0) {
        line_number_mode = 1;
        target_path = argv[2];
    } else if (argc == 2) {
        target_path = argv[1];
    } else {
        printf("cat: invalid arguments\n");
        return MINI_OS_ERROR;
    }

    /* 1. 파일 쓰기 모드 (cat > file) */
    if (write_mode) {
        printf("입력을 종료하려면 Ctrl+D (EOF)를 누르세요.\n");
        
        // 입력 중에는 fs_lock을 걸지 않음 (다른 스레드 멈춤 방지)
        char buffer[4096] = {0};
        size_t current_len = 0;
        char line[256];
        
        while (fgets(line, sizeof(line), stdin) != NULL) {
            size_t len = strlen(line);
            if (current_len + len < sizeof(buffer) - 1) {
                strcpy(buffer + current_len, line);
                current_len += len;
            } else {
                printf("cat: 경고: 최대 파일 크기를 초과하여 입력이 잘립니다.\n");
                break;
            }
        }
        clearerr(stdin); // 터미널 셸의 다음 명령어 입력을 위해 EOF 플래그 초기화

        // 파일 시스템 락을 걸고 생성/수정
        pthread_mutex_lock(&ctx->fs_lock);
        
        FsNode *target = resolve_path(ctx, target_path);
        if (target != NULL) {
            if (target->type == FS_DIRECTORY) {
                pthread_mutex_unlock(&ctx->fs_lock);
                printf("cat: %s: Is a directory\n", target_path);
                return MINI_OS_ERROR;
            }
            // 기존 파일 덮어쓰기
            free(target->content);
            target->content = (char *)malloc(current_len + 1);
            if (target->content) {
                memcpy(target->content, buffer, current_len);
                target->content[current_len] = '\0';
            }
            target->content_size = current_len;
            target->modified_time = time(NULL);
        } else {
            // 새 파일 생성
            FsNode *parent;
            char name[256];
            if (resolve_parent_and_name(ctx, target_path, &parent, name, sizeof(name)) == MINI_OS_SUCCESS) {
                FsNode *new_node = fs_create_node(name, FS_FILE);
                if (new_node) {
                    new_node->content = (char *)malloc(current_len + 1);
                    if (new_node->content) {
                        memcpy(new_node->content, buffer, current_len);
                        new_node->content[current_len] = '\0';
                    }
                    new_node->content_size = current_len;
                    fs_add_child(parent, new_node);
                }
            } else {
                pthread_mutex_unlock(&ctx->fs_lock);
                printf("cat: cannot create '%s': No such file or directory\n", target_path);
                return MINI_OS_ERROR;
            }
        }
        pthread_mutex_unlock(&ctx->fs_lock);
        return MINI_OS_SUCCESS;
    }

    /* 2. 파일 읽기 모드 (cat file 또는 cat -n file) */
    pthread_mutex_lock(&ctx->fs_lock);
    
    FsNode *target = resolve_path(ctx, target_path);
    if (target == NULL) {
        pthread_mutex_unlock(&ctx->fs_lock);
        printf("cat: %s: No such file or directory\n", target_path);
        return MINI_OS_ERROR;
    }

    if (target->type == FS_DIRECTORY) {
        pthread_mutex_unlock(&ctx->fs_lock);
        printf("cat: %s: Is a directory\n", target_path);
        return MINI_OS_ERROR;
    }

    if (target->content == NULL || target->content_size == 0) {
        pthread_mutex_unlock(&ctx->fs_lock);
        return MINI_OS_SUCCESS; // 빈 파일은 아무것도 출력하지 않음
    }

    if (line_number_mode) {
        int line_num = 1;
        printf("%6d  ", line_num++);
        for (size_t i = 0; i < target->content_size; i++) {
            putchar(target->content[i]);
            if (target->content[i] == '\n' && i + 1 < target->content_size) {
                printf("%6d  ", line_num++);
            }
        }
        if (target->content[target->content_size - 1] != '\n') {
            printf("\n"); // 셸 프롬프트 엉킴 방지
        }
    } else {
        printf("%s", target->content);
        if (target->content[target->content_size - 1] != '\n') {
            printf("\n");
        }
    }

    pthread_mutex_unlock(&ctx->fs_lock);
    return MINI_OS_SUCCESS;
}