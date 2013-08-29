#include "apue.h"
#include <pthread.h>
#include <semaphore.h>

static void *philosopher(void *arg);
static void thinking(int);
static void takeFolk(int);
static void eating(int);
static void putFolk(int);

static int N;
static int nsec = 2;
static sem_t *forks;

int main(int argc, char *argv[])
{
    int i;
    pthread_t tid;
    if (argc < 2) {
        printf("usage: philosopher_th <N> [ -t <time> ]\n");
        return 0;
    }
    N = atoi(argv[1]);
    if (argc == 4) {
        nsec = atoi(argv[3]);
    }

    /* 首先初始化信号量 */
    forks = (sem_t *)malloc(sizeof(sem_t) * N);
    for (i = 0; i < N; i++) {
        if (sem_init(forks + i, 0, 1) < 0) {
            err_sys("sem_init error");
        }
    }

    for (i = 0; i < N; i++) {
        if (pthread_create(&tid, NULL, philosopher, (void *)i) < 0) {
            err_sys("pthread_create error");
        }
    //     if ((pid = fork()) < 0) {
    //         err_sys("fork error");
    //     }
    //     if (pid == 0) { /* 子进程 */
    //         philosopher(i);
    //     }
    }
    pause();
    return 0;
}

static void *philosopher(void *arg)
{
    pthread_detach(pthread_self());
    int i = (int)arg;
    while (1) {
        thinking(i);
        takeFolk(i);
        eating(i);
        putFolk(i);
    }
}

static void thinking(int i)
{
    printf("philosopher %d is thinking\n", i);
    sleep(nsec);
}

static void takeFolk(int i)
{
    //printf("philosopher %d is takes the fork\n");
    if (i == 0) { /* 设置一个左撇子，防止死锁 */
        sem_wait(&forks[(i + 1) % N]);  
        sem_wait(&forks[i % N]);
        return;
    }
    sem_wait(&forks[i % N]);
    sem_wait(&forks[(i + 1) % N]);
}

static void eating(int i)
{
    printf("philosopher %d is eating\n", i);
    sleep(nsec);
}

static void putFolk(int i)
{
    //printf("philosopher %d puts the fork\n");
    sem_post(&forks[i % N]);
    sem_post(&forks[(i + 1) % N]);
}
