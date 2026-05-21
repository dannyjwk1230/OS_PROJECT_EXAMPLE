#ifndef COMMANDS_BASIC_H
#define COMMANDS_BASIC_H

#include "minios.h"

MiniOsStatus cmd_ls(MiniOsContext *ctx, int argc, char **argv);
MiniOsStatus cmd_cd(MiniOsContext *ctx, int argc, char **argv);
MiniOsStatus cmd_mkdir(MiniOsContext *ctx, int argc, char **argv);
MiniOsStatus cmd_cat(MiniOsContext *ctx, int argc, char **argv);
MiniOsStatus cmd_pwd(MiniOsContext *ctx, int argc, char **argv);
MiniOsStatus cmd_echo(MiniOsContext *ctx, int argc, char **argv); 

#endif
