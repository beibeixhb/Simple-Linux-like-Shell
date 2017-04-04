#include "shell.h"

/*function definition*/


/*used to write command into a file named history.txt file*/
int writeHistory(char *line)
{
    if (historyFile == NULL) {
	   printerr(debugLevel, "Can't open the history file!\n");
	   exit(1);
    }
    
    /* do not write repeat command into history */
    if (programs[0] == NULL || programs[0] -> argc == 0 || (programs[0] -> argv)[0] == NULL || !strcmp((programs[0] -> argv)[0], "repeat")) {
	       return 0;
    }
    
    commandIndex++;
    fprintf(historyFile, "%d %s", commandIndex, line);
    
     /*flush the buffer immediately*/
    fflush(historyFile);
    return 0;
}

int parseInputLine(void)
{
    char **tokens;
    char *line = NULL;
    size_t linecap = 0;

    if (strcmp(repeatCommand, "") != 0) {
	   line = strdup(repeatCommand); /* get input from the buffer */
	   repeatCommand[0] = '\0'; /* empty command buffer */
    } 
    else if (f_flag && getline(&line, &linecap, pointerOfFile) != -1) {
    } 
    else {	
        
	   fprintf(stdout, "xsh >> ");
	   getline(&line, &linecap, stdin);
    }
    
    tokenizeCommand(line, &tokens);
    replaceVar_xsh(tokens);

    if (x_flag == 1) {
	   for (int i = 0; tokens[i] != NULL; i++) {
	       printf("%s ", tokens[i]);
	   }
	   printf("\n");
    }
 
    setNumOfPipes(tokens);
    getArguments(tokens);
    writeHistory(line);

    /*do the clean up work*/
    token_destroy(tokens);
    free(line);

    return 0;
}


/*if the debugLevel is 0, don't print any error, if it's greater than 0, print some error information*/
void printerr(int debugLevel, char *errmsg) {
    if (debugLevel > 0) {
	   fprintf(stderr, "%s", errmsg);
    }
}


/*parse the command line arguments when executing xsh, use getopt function to find the option and value of each option*/
void parsecml(int argc, char *argv[])
{
    extern char *optarg;
    extern int optind;
    int c, err = 0;
    char *usage = "usage: xsh [-x] [-d <level>] [-f file [arg] ...]\n";

    while ((c = getopt(argc, argv, "xd:f:")) != -1) {
	switch (c) {
	case 'x':
	    x_flag = 1;
	    break;
	case 'd':
	    if (d_flag == 1) {
		  printerr(debugLevel, "Error: -d has been set already\n");
	    }
	    d_flag = 1;
	    debugLevel = atoi(optarg);
	    break;
	case 'f':
	    if (f_flag == 1) {
		  printerr(debugLevel, "Error: -f has been set already\n");
	    }
	    f_flag = 1;
	    batchfile = optarg;
	    break;
	case '?':
	    err = 1;
	    break;
	default:
	    break;
	}
	if (err) {
	    printerr(debugLevel, usage);
	    exit(1);
	}	
    }

    if (optind < argc) { /* arguments assigned to shell variables */
	for (int i = 0; optind < argc; i++) {
	    filearg[i] = argv[optind];
	    optind++;
	    if (i >= MAXFILEARG) {
		  printerr(debugLevel, "entered too many input arguments, the maximum is 100\n");
		  break;
	    }
	}
    }
}


/*tokenize the command line arguments, it takes the input line and the address of the pointer of an array of line tokens*/
int tokenizeCommand(char *input, char **tokens[])
{
    int maxsize = MAXTOKENS, cursize = 0;
    char *inputcpy = strdup(input);    
    char *token;

    if ((*tokens =  malloc(maxsize * sizeof(char*))) == NULL) {
	   printerr(debugLevel, "error: memory allocation failed\n");
    }
    token = strtok(inputcpy, TOKENDELM);

    while (token != NULL && token[0] != '#') {
	   (*tokens)[cursize] = strdup(token);
	   cursize++;

	   if (cursize >= maxsize) {
	       maxsize *= 2;
	       if ((*tokens = realloc(*tokens, maxsize * sizeof(char *))) == NULL) {
		  printerr(debugLevel, "error: memory allocation failed\n");
	       }
	   }
	
	   token = strtok(NULL, TOKENDELM);
    }

    (*tokens)[cursize] = NULL;
    free(inputcpy);
    return 0;
}


/*used to create the programs*/
Program *Program_create(int argc, char **argv, int stdout_redirect, int stdin_redirect, char *outfile, char *infile, int bg)
{
    Program *p = malloc(sizeof(Program));
    assert(p != NULL);

    p -> argc = argc;
    p -> stdout_redirect = stdout_redirect;
    p -> stdin_redirect = stdin_redirect;
    p -> outfile = strdup(outfile);
    p -> infile = strdup(infile);
    p -> argv = malloc((argc + 1) * sizeof(char *));
    for (int i = 0; i < argc; i++) {
	(p -> argv)[i] = strdup(argv[i]);
    }
    (p -> argv)[argc] = NULL;
    p -> bg = bg;

    return p;
}

/*used to destroy the program*/
void Program_destroy(Program *p)
{
    assert(p != NULL);

    free(p -> outfile);
    free(p -> infile);
    for (int i = 0; i < p -> argc; i++) {
	assert((p -> argv)[i] != NULL);
	free((p -> argv)[i]);
    }
    free(p -> argv);
    free(p);
}


/*destroy the tokens*/
int token_destroy(char *tokens[])
{
    for (int i = 0; tokens[i] != NULL; i++) {
	   assert(tokens[i] != NULL);
	   free(tokens[i]);
    }
    assert(tokens != NULL);
    free(tokens);
    return 0;
}


/* go through tokens and store the 
arguments for each program, this function
should be called after tokenizeCommand */
int getArguments(char *tokens[])
{
    int position = 0;
    int numPro = 0;
    char *temp[MAXTOKENS];
    char *infile = "";
    char *outfile = "";
    int stdin_redirect = 0;
    int stdout_redirect = 0;
    int bg = 0;
    
    /* empty old programs */
    for (int i = 0; i < MAXNUMOFPROS; i++) {
	   if (programs[i] != NULL) {
	       Program_destroy(programs[i]);
	       programs[i] = NULL;
	   }	
    }

    if (ispiped()) {
	   for (int i = 0; tokens[i] != NULL; i++) {
	       if (strcmp("|", tokens[i]) != 0) {
		  temp[position++] = tokens[i];
	       } else {
		  Program *p = Program_create(position, temp, 0, 0, "", "", bg);
		  programs[numPro++] = p;
		  position = 0;
	       }	    
	   }
	/* the last program */
	   Program *p = Program_create(position, temp, 0, 0, "", "", bg);
	   programs[numPro] = p;
    } else {
	   position = 0;
	   for (int i = 0; tokens[i] != NULL; i++) {
	       if (strcmp(">", tokens[i]) == 0) {
		  outfile = tokens[++i];
		  stdout_redirect = 1;
	       } else if (strcmp("<", tokens[i]) == 0) {
		  infile = tokens[++i];
		  stdin_redirect = 1;
	       } else if (strcmp("&", tokens[i]) == 0) {
		  bg = 1;
	       }else {
		  temp[position++] = tokens[i];
	       }
	   }
	
	   Program *p = Program_create(position, temp, stdout_redirect, stdin_redirect, outfile, infile, bg);
	   programs[0] = p;
        }
    assert(temp != NULL);
    return 0;
}

/*set the number of pipes, must be called before getArguments()*/
void setNumOfPipes(char *tokens[])
{
    int temp = 0;

    for (int i = 0; tokens[i] != NULL; i++) {
	   if (strcmp("|", tokens[i]) == 0) {
	       temp++;
	   }
    }
    numOfPipes = temp;
}

/*get the number of pipes*/
int getNumOfPipes(void)
{
    return numOfPipes;
}

/*return the number of pipes*/
int ispiped(void)
{
    return getNumOfPipes() > 0;
}


