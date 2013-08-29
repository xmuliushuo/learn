#include	<signal.h>
#include	<stddef.h>
#include	<time.h>
#include	"apue.h"

unsigned int mysleep(unsigned int nsecs);

static char *testpos;
static unsigned int smallsecs = 5;
static unsigned int bigsecs = 10;

static void sig_alrm(int signo)
{
	printf("%s: SIGALRM is caught\n", testpos);
	return;
}

static void sig_usr1(int signo)
{
	printf("%s: SIGUSR1 is caught\n", testpos);
	return;
}

static void loopwait(unsigned int nsecs)
{
	time_t start;

	start = time(NULL);
	while( time(NULL) < (start + nsecs) );
}


void testsleep(unsigned int oldsecs, unsigned int nsecs)
{
	pid_t pid;
	
	testpos = "in testsleep, 1st pass";
	alarm(oldsecs);
	printf("%s: unslept is %u seconds\n", testpos, mysleep(nsecs) );
	loopwait(bigsecs);
	
	testpos = "in testsleep, 2nd pass";
	if( (pid = fork() ) < 0 )
		err_sys("fork error");
	if( pid == 0) {
		loopwait(2);
		kill(getppid(), SIGUSR1);
		exit(0);
	}
	else {
		alarm(oldsecs);
		printf("%s: unslept is %u seconds\n", testpos, mysleep(nsecs) );
		if( wait(NULL) != pid )
			err_sys("wait error");
	}
}

main()
{
	sigset_t	alrmmask, oldmask;

	setvbuf(stdout, NULL, _IONBF, 0);

	signal(SIGUSR1, sig_usr1);
	signal(SIGALRM, sig_alrm);
	sigemptyset(&alrmmask);
	sigaddset(&alrmmask, SIGALRM);

	printf("set alarm to 0 seconds and sleep 5 seconds\n");
	testsleep(0, 5);
	testpos = "out of testsleep";
	printf("************* 1st sleep test finished *************\n\n");

	printf("set alarm to %u seconds and sleep %u seconds\n", smallsecs, bigsecs);
	testsleep(smallsecs, bigsecs);
	testpos = "out of testsleep";
	loopwait(bigsecs);
	printf("************* 2nd sleep test finished *************\n\n");

	printf("set alarm to %u seconds and sleep %u seconds\n", bigsecs, smallsecs);
	testsleep(bigsecs, smallsecs);
	testpos = "out of testsleep";
	loopwait(bigsecs);
	printf("************* 3rd sleep test finished *************\n\n");
	
	printf("Now SIGALRM is blocked\n");
	sigprocmask(SIG_BLOCK, &alrmmask, &oldmask);
	printf("set alarm to %u seconds and sleep %u seconds\n", smallsecs, bigsecs);
	testsleep(smallsecs, bigsecs);
	testpos = "out of testsleep";
	printf("Now SIGALRM is unblocked\n");
	sigprocmask(SIG_SETMASK, &oldmask, NULL);
	loopwait(bigsecs);
	printf("************* 4th sleep test finished *************\n\n");

	printf("Now SIGALRM is blocked\n");
	sigprocmask(SIG_BLOCK, &alrmmask, &oldmask);
	printf("set alarm to %u seconds and sleep %u seconds\n", bigsecs, smallsecs);
	testsleep(bigsecs, smallsecs);
	testpos = "out of testsleep";
	printf("Now SIGALRM is unblocked\n");
	sigprocmask(SIG_SETMASK, &oldmask, NULL);
	loopwait(bigsecs);
	printf("************* 5th sleep test finished *************\n\n");

	printf("Now SIGALRM is blocked and a SIGALRM is pending\n");
	sigprocmask(SIG_BLOCK, &alrmmask, &oldmask);
	kill(getpid(), SIGALRM);
	printf("set alarm to 0 seconds and sleep 5 seconds\n");
	testsleep(0, 5);
	testpos = "out of testsleep";
	printf("Now SIGALRM is unblocked\n");
	sigprocmask(SIG_SETMASK, &oldmask, NULL);
	printf("************* 6th sleep test finished *************\n\n");
}

