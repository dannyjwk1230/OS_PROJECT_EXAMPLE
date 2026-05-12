#ifndef COMMANDS_EVEN_H
#define COMMANDS_EVEN_H

#include "minios.h"

MiniOsStatus cmd_chmod(MiniOsContext *ctx, int argc, char **argv);
MiniOsStatus cmd_clear(MiniOsContext *ctx, int argc, char **argv);
MiniOsStatus cmd_cp(MiniOsContext *ctx, int argc, char **argv);
MiniOsStatus cmd_rmdir(MiniOsContext *ctx, int argc, char **argv);

#endif
