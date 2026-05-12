#ifndef THREADING_H
#define THREADING_H

#include "minios.h"

typedef MiniOsStatus (*ThreadTask)(void *arg);

MiniOsStatus thread_run_tasks(ThreadTask *tasks, void **args, int task_count);

#endif
