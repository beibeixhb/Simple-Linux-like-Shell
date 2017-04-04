/* CSE422 Project 1: xsh */

/*
Developed by: Haobo Xing
              Yue Hu
              Taoran Yin
              Xueting Yu
*/

#include "shell.h"

/*set the shell PID*/
void init_shellPid(void);
/*return value*/
void fgProcessReturnValue(int cmd_exit_status);
/*function used to set the background value*/
void bgProcessPid(int background_pid);
/*initial some global variables*/
int initGlobalVariables(void);
/*initial batch file*/
int init_batchfile(void);
/*check whether it is interactive*/
int isInteractive(Program *);
 /* store argv[0], the command to invoke xsh */
char *shellcommand;

void init_environVar(int cmd_exit_status, int background_pid) {
	/*$$: PID of this shell*/
	/*$?: Decimal value returned by the last foreground command*/
	/*$!: PID of the last background command*/
	init_shellPid();
	fgProcessReturnValue(cmd_exit_status);
	bgProcessPid(background_pid);
}

/*set "$" the shell pid*/
void init_shellPid(void) {
	char shell_pid[100];
    /*user sprintf function to write the pid of the shell into the shell_pid array*/
	sprintf(shell_pid, "%d", getpid());
    /*set the environ variable*/
	setenv("$", shell_pid, 1);
}

// set "?" the value returned by the last foreground command
void fgProcessReturnValue(int cmd_exit_status) {
	char last_exit_status[100];
    /*user sprintf function to write the pid of the shell into the last_exit_status array*/
	sprintf(last_exit_status, "%d", cmd_exit_status);
     /*set the environ variable*/
	setenv("?", last_exit_status, 1);
}

// set "!" the last background command pid
void bgProcessPid(int background_pid) {
	char last_bg_pid[100];
    /*user sprintf function to write the pid of the shell into the last_bg_pid array*/
	sprintf(last_bg_pid, "%d", background_pid);
    /*set the environ variable*/
	setenv("!", last_bg_pid, 1);	
}

/* initialize the input file */
int init_batchfile(void) {
    if (!f_flag) {
	   return 1;
    }
    if (batchfile == NULL) {
	   printerr(debugLevel, "file name is empty\n");
	   exit(1);
    }

    pointerOfFile = fopen(batchfile, "r");
    for (int i = 0; filearg[i] != NULL && i < MAXFILEARG; i++) {	
	   char name[20];
	   sprintf(name, "%d", i + 1);
	   char *tempargv[] = {"set", name, filearg[i]};
	   set_xsh(3, tempargv);
    }

    if (pointerOfFile == NULL) {
	   printerr(debugLevel, "could not open the batch file\n");
    }
    //exit(0);
    return 0;
    //exit(0);
}

	
/*this function is used to initialize global variable*/
int initGlobalVariables(void) {
    numOfPipes = 0;
    d_flag = 0;
    x_flag = 0;
    f_flag = 0;
    debugLevel = 0;
    commandIndex = 0;
    repeatCommand[0] = '\0';
    shellpath = malloc(BUFFERSIZE * sizeof(char));
    /*set the environment variable shell*/
    getcwd(shellpath, BUFFERSIZE);    
    if (shellcommand[0] == '.') {
	   shellpath = strncat(shellpath, "/xsh", 5);
    } else {
	   shellpath = strncat(shellpath, "/", 2);
	   shellpath = strncat(shellpath, shellcommand, BUFFERSIZE / 2);
    }
    setenv("shell", shellpath, 1);
    init_shellPid();

    /*set readme path and history path*/
    strncpy(readmepath, shellpath, BUFFERSIZE);
    readmepath[strlen(shellpath) - 3] = '\0';
    sprintf(historyfilename, "%shistory%d", readmepath, getpid());
    strncat(readmepath, "/readme", BUFFERSIZE);

    /* open history file */
    historyFile = fopen(historyfilename, "w+");

    /* initialize the array of var structs */
    init_localVar();
 
    return 0;
}

int isInteractive(Program *pro) {    
    return !(pro -> bg);
}


int main(int argc, char *argv[]) {   
    int pid;
    shellcommand = argv[0];
    /*set some global variables*/
    initGlobalVariables();
    /*call the signal initialization function to listen the signal*/
    init_signal();
    /*parse the input line and find out the option and the corresponding value*/
    parsecml(argc, argv);
    /*batch mode*/
    init_batchfile();
    
    printerr(debugLevel, "The xsh shell is now executing, welcome...\n");

    for (;;) {
        /*first parse the input line*/
	   parseInputLine();
        /*check whether it is internal command*/
	   if (isInternal(programs[0] -> argc, programs[0] -> argv)) {
	       continue;
	   }
        /*check for pipes*/
	   if (!ispiped()) {
	       if ((pid = fork()) == 0) {
		  /*fork a child*/
		      commandRedirection(programs[0], d_flag);
		      setenv("parent", shellpath, 1);

		  /*change the group pid of the background process so that it won't be killed*/
		  if (!isInteractive(programs[0])) {		    
		      setpgid(0, 0); 
		  }
               /*if not a built_in function, executed as an external command*/
		  if (!isBuildIn(programs[0] -> argc, programs[0] -> argv)) {
		      execvp((programs[0] -> argv)[0], (programs[0] -> argv));
		  }
		  exit(1);
	       } else {
		  if (isInteractive(programs[0])) {
		      pidOfFg = pid;
		      int status;
		      waitpid(pid, &status, WUNTRACED);
		      fgProcessReturnValue(status);
		  } else {
		      /*record the pid of the last background process*/
		      bgProcessPid(pid);
		      }
	       }
	   } else {
	       commandPipe(programs, getNumOfPipes(), d_flag);
	       }
//        if (!feof(pointerOfFile)) {
//            exit(1);
//        }
    }
//    /*when executed in batch mode, after finishing all the commands in a file, the xsh exit*/
//        if (pointerOfFile != NULL) {
//            exit(0);
//        }
}


