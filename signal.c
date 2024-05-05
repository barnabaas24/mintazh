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


void handler(int signumber) {
  printf("Signal with number %i has arrived\n", signumber);
}

int main(){

  signal(SIGTERM,handler); //subscribe the signals to handler function
  // handler = SIG_IGN - ignore the signal (not SIGKILL,SIGSTOP),
  // handler = SIG_DFL - back to default behavior

    pid_t child = fork();
    if (child > 0) //parent
    {
        pause(); // waits till a signal arrives
        printf("Signal arrived\n");
        wait(NULL);  //wait until child process finishes
        printf("Parent process ended\n");
    } else {
        printf("Waits 3 seconds, then send a SIGTERM %i signal\n", SIGTERM);
        sleep(3); // sleep, hogy a szülő "biztos" előbb érjen el a pause() -hoz és ne legyen deadlock
        kill(getppid(), SIGTERM);
        // 1. parameter the pid number of process, we send the signal
        // 		if -1, then eacho of the processes of the same uid get the
        // signal 		we kill our bash as well! The connection will close
        // 2. parameter the name or number of signal
        sleep(3);
        printf("Child process ended\n");
    }


    return 0;
}