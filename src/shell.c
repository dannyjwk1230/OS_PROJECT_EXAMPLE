#include <stdio.h>
#include <string.h>
#include "fs.h"
#include "shell.h"
#include "commands/basic.h"
#include "commands/even.h"

static const CommandEntry COMMANDS[] = {
    /* 명령어 이름과 실제 처리 함수를 연결하는 명령어 테이블이다. */
    {"ls", cmd_ls, "list directory contents"},
    {"cd", cmd_cd, "change current directory"},
    {"mkdir", cmd_mkdir, "create directories"},
    {"cat", cmd_cat, "print or write file contents"},
    {"pwd", cmd_pwd, "print working directory"},
    {"chmod", cmd_chmod, "change file permissions"},
    {"clear", cmd_clear, "clear terminal"},
    {"cp", cmd_cp, "copy files or directories"},
    {"rmdir", cmd_rmdir, "remove empty directories"},
    {"touch", cmd_touch, "change file timestamps / create empty file"},
    {"echo", cmd_echo, "print arguments to the standard output"},
    {"search", cmd_search, "search files and directories by name"},
};

int parse_input(char *input, char **argv)
{
    int argc = 0;

    /* 공백 문자를 기준으로 사용자의 입력을 명령어와 인자로 나눈다. */
    char *token = strtok(input, " \t\r\n");

    while (token != NULL && argc < MINI_OS_MAX_ARGS) {
        argv[argc++] = token;
        token = strtok(NULL, " \t\r\n");
    }

    return argc;
}

static MiniOsStatus shell_dispatch(MiniOsContext *ctx, int argc, char **argv);

static void shell_print_current_path(const MiniOsContext *ctx)
{
    const FsNode *stack[128];
    const FsNode *current;
    int count = 0;

    /* root 디렉토리 생성 오류 */
    if (ctx == NULL || ctx->current == NULL) {
        fputs("/", stdout);
        return;
    }

    for (current = ctx->current; current != NULL && count < 128; current = current->parent) {
        stack[count++] = current;
    }

    if (count <= 1) {
        fputs("/", stdout);
        return;
    }

    /* root 디렉토리는 제외하고 출력한다 */
    for (int i = count - 2; i >= 0; i--) {
        printf("/%s", stack[i]->name);
    }
}

MiniOsStatus shell_run(MiniOsContext *ctx)
{
    char input[MINI_OS_MAX_INPUT];
    char *argv[MINI_OS_MAX_ARGS];

    /* exit 또는 EOF가 들어올 때까지 Mini OS 프롬프트를 반복한다. */
    while (1) {
        /* 현재 사용자 정보를 포함한 프롬프트를 출력한다. */
        printf("%s@minios:", ctx->current_user);
        shell_print_current_path(ctx);
        printf("> ");

        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        int argc = parse_input(input, argv);
        if (argc == 0) {
            continue;
        }

        /* exit는 별도 명령어 구현 없이 셸 루프 종료용으로 처리한다. */
        if (strcmp(argv[0], "exit") == 0) {
            break;
        }

        /* 파싱된 명령어를 등록된 핸들러로 전달한다. */
        shell_dispatch(ctx, argc, argv);
    }

    return MINI_OS_SUCCESS;
}

static MiniOsStatus shell_dispatch(MiniOsContext *ctx, int argc, char **argv)
{
    size_t command_count = sizeof(COMMANDS) / sizeof(COMMANDS[0]);

    /* 명령어 테이블에서 입력한 명령어 이름과 일치하는 항목을 찾는다. */
    for (size_t i = 0; i < command_count; i++) {
        if (strcmp(argv[0], COMMANDS[i].name) == 0) {
            return COMMANDS[i].handler(ctx, argc, argv);
        }
    }

    /* 등록되지 않은 명령어는 오류로 처리한다. */
    fprintf(stderr, "unknown command: %s\n", argv[0]);
    return MINI_OS_ERROR;
}
