#include <stdio.h>
#include "commands/basic.h"

MiniOsStatus cmd_echo(MiniOsContext *ctx, int argc, char **argv)
{
    /* echo는 파일 시스템 상태를 읽거나 쓰지 않으므로 ctx가 필요 없습니다. */
    (void)ctx;

    // 인자가 없으면 (즉, 'echo'만 쳤으면) 단순 줄바꿈만 출력
    if (argc < 2) {
        printf("\n");
        return MINI_OS_SUCCESS;
    }

    // 입력받은 인자들을 공백을 사이에 두고 순서대로 출력
    for (int i = 1; i < argc; i++) {
        printf("%s", argv[i]);
        // 마지막 인자가 아니면 띄어쓰기 추가
        if (i < argc - 1) {
            printf(" ");
        }
    }
    printf("\n");

    return MINI_OS_SUCCESS;
}
