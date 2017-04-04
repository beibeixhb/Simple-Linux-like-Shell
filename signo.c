#include "shell.h"

void init_signal(void);

struct job_t {              /* The job struct */
    pid_t pid;              /* job PID */
    int jid;                /* job ID [1, 2, ...] */
    int state;              /* UNDEF, BG, FG, or ST */
    char cmdline[MAXLINE];  /* command line */
};

struct job_t job_list[MAXJOBS];

void sighandler(int);
pid_t fgpid(struct job_t *job_list);


void init_signal(void) {

	/* signal to be cached */
	if (signal(SIGINT, sighandler) == SIG_ERR) {
		printf("Can't catch SIGINT");
    }
    if (signal(SIGQUIT, sighandler) == SIG_ERR) {
		printf("Can't catch SIGQUIT");
    }
    if (signal(SIGCONT, sighandler) == SIG_ERR) {
		printf("Can't catch SIGCONT");
    }
    if (signal(SIGTSTP, sighandler) == SIG_ERR) {
		printf("Can't catch SIGTSTP");
    }
    /* signals to be ignored */
    if (signal(SIGABRT, sighandler) == SIG_ERR) {
		printf("Can't catch SIGABRT");
    }
    if (signal(SIGALRM, sighandler) == SIG_ERR) {
		printf("Can't catch SIGALRM");
    }
    if (signal(SIGHUP, sighandler) == SIG_ERR) {
		printf("Can't catch SIGHUP");
    }
    if (signal(SIGTERM, sighandler) == SIG_ERR) {
		printf("Can't catch SIGTERM");
    }
    if (signal(SIGUSR1, sighandler) == SIG_ERR) {
		printf("Can't catch SIGUSR1");
    }
    if (signal(SIGUSR2, sighandler) == SIG_ERR) {
		printf("Can't catch SIGUSR2");
    }
    /* signal from dead child */
    if (signal(SIGCHLD, sighandler) == SIG_ERR) {
		printf("Can't catch SIGCHLD");
    }
    
}


// signal handler
void sighandler(int signum)
{
	pid_t tmppid = fgpid (job_list);
	tmppid *=  -1;
    switch (signum) {
    case SIGINT:
    	if (tmppid != 0)
    		kill (tmppid, SIGINT);
		printerr(debugLevel, "caught signal %d, coming\n");
		fflush(stdout);
		break;
    case SIGQUIT:
    	if (tmppid != 0)
    		kill (tmppid, SIGQUIT);
		printerr(debugLevel, "caught signal %d, coming\n");
		fflush(stdout);
		break;
    case SIGCONT:
    	if (tmppid != 0)
    		kill (tmppid, SIGCONT);
		printerr(debugLevel, "caught signal %d, coming\n");
		fflush(stdout);
		break;
    case SIGTSTP:
    	if (tmppid != 0)
    		kill (tmppid, SIGTSTP);
		printerr(debugLevel, "caught signal %d, coming\n");
		fflush(stdout);
		break;
    case SIGCHLD:
		printerr(debugLevel, "caught signal %d, coming\n");
		fflush(stdout);
		break;
    default:
		printerr(debugLevel, " other signals\n");
		break;
    }
    return;
}

pid_t 
fgpid(struct job_t *job_list) {
    int i;

    for (i = 0; i < MAXJOBS; i++)
        if (job_list[i].state == FG)
            return job_list[i].pid;
    return 0;
}