/* 
 * mysleep的实现
 * encoding: UTF-8
 * xmuliushuo
 */
#include "apue.h"

//#define DEBUG

static void sig_alrm(int signo)
{

}

unsigned int mysleep(unsigned int nsecs)
{
    #ifdef DEBUG
    printf("DEBUG - mysleep(%u)\n", nsecs);
    #endif

    struct sigaction newact, oldact;
    sigset_t newmask, oldmask, suspmask, blockedmask;
    int issuspend = 0;/* SIGALRM是否被阻塞 */
    int isblocked = 0;/* 是否存在未决的SIGALRM */
    unsigned int unslept, slept;
    unsigned int oldalarm;

    oldalarm = alarm(0);

    /* 保存以前的SIGALRM配置，并设置成我们需要的配置 */
    newact.sa_handler = sig_alrm;
    sigemptyset(&newact.sa_mask);
    newact.sa_flags = 0;
    sigaction(SIGALRM, &newact, &oldact);

    /* 处理以前的SIGALRM */
    /* 判断SIGALRM是否被阻塞 */
    sigprocmask(SIG_BLOCK, NULL, &oldmask);
    if (sigismember(&oldmask, SIGALRM)) {
        /* SIGALRM被阻塞 */
        issuspend = 1;
    }

    /* 判断是否存在未决的SIGALRM */
    if (sigpending(&blockedmask) < 0) {
        err_sys("sigpending error");
    }
    if (sigismember(&blockedmask, SIGALRM)) {
        /* 存在未决的SIGALRM */
        suspmask = oldmask;
        isblocked = 1;
        sigdelset(&suspmask, SIGALRM);
        sigsuspend(&suspmask);
        sigprocmask(SIG_SETMASK, &oldmask, NULL);
    }

    sigemptyset(&newmask);
    sigaddset(&newmask, SIGALRM);
    sigprocmask(SIG_BLOCK, &newmask, &oldmask);

    #ifdef DEBUG
    printf("DEBUG - oldalarm = %u\n", oldalarm);
    #endif

    if ((oldalarm != 0) && (oldalarm < nsecs) && (issuspend == 0)) {
        /* 以前剩余的闹钟值比睡眠时间少且没有被阻塞 */
        alarm(oldalarm);
    }
    else {
        /* 以前剩余的闹钟值比睡眠时间多或者被阻塞 */
        alarm(nsecs);
    }

    suspmask = oldmask;
    sigdelset(&suspmask, SIGALRM);
    sigsuspend(&suspmask);

    unslept = alarm(0);
    sigaction(SIGALRM, &oldact, NULL);

    sigprocmask(SIG_SETMASK, &oldmask, NULL);

    /* 计算实际睡眠时间 */
    if ((oldalarm != 0) && (oldalarm < nsecs) && (issuspend == 0)) {
        /* 以前的闹钟值比睡眠时间少 */
        slept = oldalarm - unslept;
    }
    else {
        /* 以前的闹钟值比睡眠时间多 */
        slept = nsecs - unslept;
    }

    if (isblocked == 1) {
        kill(getpid(), SIGALRM);
    }
    if (slept < oldalarm) {
        /* 以前的闹钟值比实际睡眠时间多 */
        alarm(oldalarm - slept);
        return nsecs - slept;
    }
    else {
        if (oldalarm != 0) {
            kill(getpid(), SIGALRM);
        }
        return nsecs - slept;
    }
    return 0; /* 出错 */
}
