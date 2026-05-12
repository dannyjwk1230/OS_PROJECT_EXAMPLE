#include "storage.h"

MiniOsStatus storage_load_tree(MiniOsContext *ctx, const char *path)
{
    /* TODO: path 파일에서 저장된 트리 정보를 읽어 Mini OS 상태로 복원한다. */
    (void)ctx;
    (void)path;
    return MINI_OS_NOT_IMPLEMENTED;
}

MiniOsStatus storage_save_tree(const MiniOsContext *ctx, const char *path)
{
    /* TODO: 현재 트리 구조와 파일 내용을 path 파일에 저장한다. */
    (void)ctx;
    (void)path;
    return MINI_OS_NOT_IMPLEMENTED;
}
