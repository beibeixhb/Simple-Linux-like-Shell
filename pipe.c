#include "shell.h"


/*Execute redirection commands*/
void commandRedirection(Program* command, int d_flag){
    // If output redirection
    if(command -> stdout_redirect){
        int fout = open(command -> outfile, O_WRONLY | O_CREAT, 0666);
        if(fout < 0){
            printerr(d_flag, "Can not open output file\n");
        }
        // copy fout to stdout
        dup2(fout, STDOUT_FILENO);
        close(fout);
    }
    // If input redirecction
    if(command -> stdin_redirect){
        int fin = open(command -> infile, O_RDONLY);
        if(fin < 0){
            printerr(d_flag, "Can not open input file\n");
        }
        // copy fin to stdin
        dup2(fin, STDIN_FILENO);
        close(fin);
    }
}


// Execute piped commands
void commandPipe(Program *command[], int numPipes, int d_flag) {
    int status;
    pid_t pid;
    // Create file descriptors according to 
    // number of pipes
    int pipefds[2*numPipes];
    // create all pipes up-front
    for(int i = 0; i < numPipes; i++){
        if(pipe(pipefds + i*2) < 0){
            printerr(d_flag,"Can not create pipe\n");
            exit(1);
        }
    }


    // Iterate through the argv, on each iteration, 
    // fork a new child process, set up its pipe 
    // according to its position
    // no parent process is involved in this while 
    // loop.  Because the parent process waits for 
    // all its children to finish, then can the parent 
    // process continue
    for( int k = 0, j = 0; k < numPipes+1; k++, j+=2){
        pid = fork();//forking a new child on each loop
        if(pid == 0) {//child

            //if not last command
            if(command[k+1] != NULL){
                // copy pipe write end to stdout
                if(dup2(pipefds[j + 1], 1) < 0){
                    printerr(d_flag, "dup2 error\n");
                    exit(1);
                }
            }

            //if not first command
            if(j != 0 ){
                // copy pipe read end to stdin
                if(dup2(pipefds[j-2], 0) < 0){
                    printerr(d_flag, "dup2 error\n");
                    exit(1);
                }
            }
            //close pipe file descriptors
            for(int i = 0; i < 2*numPipes; i++){
                close(pipefds[i]);
            }
            // execute command
	    if (!isBuildIn(command[k] -> argc, command[k] -> argv)) {
		if( execvp((command[k]->argv)[0], command[k]->argv) < 0 ){
		    printerr(d_flag, "execvp error\n");
		    exit(1);
		}
	    }

	    exit(1);
        }// child done
        else if(pid < 0){ // fork error
            printerr(d_flag, "fork error\n");
            exit(1);
        }
    }//for done


    // only the parent get here and waits for 'numPipes' 
    // many children to finish
    //Parent closes the pipes
    for(int i = 0; i < 2 * numPipes; i++){
        close(pipefds[i]);
    }
    //Parent wait for children
    for(int i = 0; i < numPipes + 1; i++) {
        waitpid(0, &status, WUNTRACED);
	    fgProcessReturnValue(status);
    }
}

