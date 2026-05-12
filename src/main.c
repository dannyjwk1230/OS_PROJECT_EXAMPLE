#include <stdio.h>
#include "fs.h"
#include "shell.h"
#include "storage.h"

int main(void)
{
    MiniOsContext ctx;

    /* Mini OS에서 사용할 루트 디렉터리와 현재 작업 위치를 준비한다. */
    if (minios_init(&ctx) != MINI_OS_OK) {
        fprintf(stderr, "failed to initialize Mini OS\n");
        return 1;
    }

    /* 이전 실행에서 저장한 가상 파일 시스템 상태가 있으면 불러온다. */
    fs_load(&ctx, MINI_OS_STORAGE_FILE);

    /* 사용자의 명령어를 입력받고 처리하는 셸 루프를 실행한다. */
    shell_run(&ctx);

    /* 종료 전에 현재 가상 파일 시스템 상태를 파일로 저장한다. */
    fs_save(&ctx, MINI_OS_STORAGE_FILE);

    /* 동적으로 만든 트리 구조를 정리하고 프로그램을 종료한다. */
    minios_shutdown(&ctx);
    return 0;
}
