#include "apue.h"
#include "lock.h"

void philosopher(int);
void thinking(int, int);
void takeFolk(int);
void eating(int, int);
void putFolk(int);

static const int N = 5;
static const char *forks[5] = {"folk1", "folk2", "folk3", "folk4", "folk5"};
static int nsec = 2;

int main(int argc, char *argv[])
{
    int i;
    int pid;

    if (argc == 3) {
        nsec = atoi(argv[2]);
    }

    /* 首先初始化锁 */
    for (i = 0; i < 5; i++) {
        initlock(forks[i]);
    }

    for (i = 0; i < N; i++) {
        if ((pid = fork()) < 0) {
            err_sys("fork error");
        }
        if (pid == 0) { /* 子进程 */
            philosopher(i);
        }
    }
    wait(NULL);
    return 0;
}

void philosopher(int i)
{
    while (1) {
        thinking(i, nsec);
        takeFolk(i);
        eating(i, nsec);
        putFolk(i);
    }
}

void thinking(int i, int nsec)
{
    printf("philosopher %d is thinking\n", i);
    sleep(nsec);
}

void takeFolk(int i)
{
    //printf("philosopher %d is takes the fork\n");
    if (i == 0) { /* 设置一个左撇子，防止死锁 */
        lock(forks[(i + 1) % N]); 
        lock(forks[i % N]);
        return;
    }
    lock(forks[i % N]);
    lock(forks[(i + 1) % N]);
}

void eating(int i, int nsec)
{
    printf("philosopher %d is eating\n", i);
    sleep(nsec);
}

void putFolk(int i)
{
    //printf("philosopher %d puts the fork\n");
    unlock(forks[i % N]);
    unlock(forks[(i + 1) % N]);
}
