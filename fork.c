#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <mqueue.h>



//===============FORK======================


int main(){

    int status;
    pid_t child, child2;
    child = fork();

    if(child>0){
        //parent

        waitpid(child, &status, 0); //wait for child process to end
        waitpid(child2, &status, 0);
        wait(NULL);
    }
    else{
        //child
    }

    child2 = fork();

    if(child2==0){
        //child2
        return 0;
    }

    //parent

    return 0;
}



