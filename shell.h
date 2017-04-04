/*macro definition*/
#define MAXFILEARG 100  /* maximum number of arguments associated with input file */
#define MAXTOKENS 1000  /* maximum number of tokens from command line input */
#define BUFFERSIZE 1000 /* size of all buffer strings */
#define MAXNUMOFPROS 4  /* maximum number of input programs */
#define TOKENDELM " \t\n" /* delimiters for parsing the command line input */
#define UNDEF         0   /* undefined */
#define FG            1   /* running in foreground */
#define BG            2   /* running in background */
#define ST            3 
#define MAXLINE    1024   /* max line size */
#define MAXJOBS      16

/*header needed for our program*/
#include <string.h> /* for strdup */
#include <assert.h> /* for assertion */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> /* for getopt */
#include <signal.h> /* needed for signal */
#include <fcntl.h> /* needed for open */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h> /* needed for waitpid */
#include <dirent.h> /* needed for alphasort */


/*variable decalaration*/
/*indicate the number of pipes*/
int numOfPipes; 
/*indicate the d flag*/
int d_flag;  
/*indicate the x flag*/
int x_flag; 
/*indicate the f flag*/
int f_flag;  
/*indicate the debuglevel*/
int debugLevel;  
/*command Index*/
int commandIndex;  
/* pid of the foreground program */
int pidOfFg; 
/*used to store the name of the history file name*/
char historyfilename[BUFFERSIZE];  
/*readme file path*/
char readmepath[BUFFERSIZE];  
/*pointer of the history file*/
FILE *historyFile;  
/* buffer for repeat command */
char repeatCommand[BUFFERSIZE]; 
/*environment variable shell*/
char *shellpath;  
/*environment variables*/
extern char **environ;
//Inbuilt sorting function
extern int alphasort(); 


/*used in batch mode*/
char *batchfile; 
/*pointer of the file*/
FILE *pointerOfFile; 
 /*file arguments*/
char *filearg[MAXFILEARG];  /*file arguments*/



/*define a struct for every command executed*/
typedef struct {
    int argc;
    char **argv;
    int stdout_redirect;
    int stdin_redirect;
    char *outfile;
    char *infile;
    int bg;
} Program;

Program *programs[MAXNUMOFPROS]; /* input programs */

/* signal handling */
void init_signal(void); /* initialize signal functions */
/* build-in function */
int isBuildIn(int, char **);
void set_xsh(int argc, char *argv[]);
int isInternal(int argc, char *argv[]);
/* Program object methods */
Program *Program_create(int, char **, int, int, char *, char *, int);
void Program_destroy(Program *);
void Program_print(Program *);
/* parser related functions */
void parsecml(int, char **);
int tokenizeCommand(char *, char ***);
int token_destroy(char **);
void printerr(int, char*);
int getArguments(char **);
/* io redirection functions */
void commandPipe(Program **, int, int);
void commandRedirection(Program *, int);
void setNumOfPipes(char **);
int getNumOfPipes(void);
int ispiped(void);
int parseInputLine(void);
/* environment variable setter */
void fgProcessReturnValue(int);
void init_shellPid(void);
void bgProcessPid(int background_pid);
void init_localVar();
void replaceVar_xsh(char *argv[]);
void exit_xsh(int argc, char *argv[]);