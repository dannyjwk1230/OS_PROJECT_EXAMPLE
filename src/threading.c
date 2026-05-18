#include <stdlib.h>
#include <pthread.h>
#include "threading.h"

typedef struct ThreadJob {
    ThreadTask task;
    void *arg;
    MiniOsStatus status;
} ThreadJob;

static void *thread_job_run(void *raw)
{
    /* pthread 시작 루틴은 사용자 정의 task를 호출하고 결과 상태만 기록한다. */
    ThreadJob *job = (ThreadJob *)raw;
    job->status = job->task(job->arg);
    return NULL;
}

MiniOsStatus thread_run_tasks(ThreadTask *tasks, void **args, int task_count)
{
    /* 잘못된 입력은 즉시 오류로 처리한다. */
    if (tasks == NULL || args == NULL || task_count <= 0) {
        return MINI_OS_ERROR;
    }

    pthread_t *threads = (pthread_t *)calloc((size_t)task_count, sizeof(pthread_t));
    ThreadJob *jobs = (ThreadJob *)calloc((size_t)task_count, sizeof(ThreadJob));

    if (threads == NULL || jobs == NULL) {
        free(threads);
        free(jobs);
        return MINI_OS_ERROR;
    }

    /* 각 작업의 메타데이터를 채우고 thread를 만든다. */
    int created = 0;
    int create_failed = 0;

    for (int i = 0; i < task_count; ++i) {
        jobs[i].task = tasks[i];
        jobs[i].arg = args[i];
        jobs[i].status = MINI_OS_SUCCESS;

        if (pthread_create(&threads[i], NULL, thread_job_run, &jobs[i]) != 0) {
            create_failed = 1;
            break;
        }
        ++created;
    }

    /* 이미 생성된 thread는 항상 join 해서 자원을 회수한다. */
    MiniOsStatus result = MINI_OS_SUCCESS;
    for (int i = 0; i < created; ++i) {
        pthread_join(threads[i], NULL);
        if (jobs[i].status != MINI_OS_SUCCESS) {
            result = MINI_OS_ERROR;
        }
    }

    if (create_failed) {
        result = MINI_OS_ERROR;
    }

    free(threads);
    free(jobs);
    return result;
}
