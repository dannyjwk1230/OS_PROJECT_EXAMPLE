#ifndef STORAGE_H
#define STORAGE_H

#include "fs.h"

#define MINI_OS_STORAGE_FILE "minios_state.dat"

MiniOsStatus storage_load_tree(MiniOsContext *ctx, const char *path);
MiniOsStatus storage_save_tree(const MiniOsContext *ctx, const char *path);

#endif
