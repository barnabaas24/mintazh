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
#include <string.h>



int main(int argc, char *argv[]){


    int pipefd[2];
    if (pipe(pipefd) == -1) //create pipe
    {
        perror("Hiba a pipe nyitaskor!");
        exit(EXIT_FAILURE);
    }

    pid_t child;
    child = fork();

    if(child==0) //gyerek
    {
        char buffer[100];
        close(pipefd[1]); // Usually we close the unused write end

        int length;
        read(pipefd[0], &length, sizeof(int));

        int c = read(pipefd[0], buffer, length); // honnan, mibe, mekkorát olvas, vissza adja mennyi bite-ot olvasott
        printf("Gyerek olvasta uzenet: %s - %i\n", buffer, c);

        //printf("buffer méret %i\n",sizeof(buffer));
        c = read(pipefd[0], buffer, sizeof(buffer));    //mindenképpen \0-ig olvas
        printf("Gyerek olvasta uzenet: %s -%i\n", buffer, c);
        
        close(pipefd[0]); // finally we close the used read end

        printf("Gyerek befejezte!\n");
    }
    else //szulo
    {
        close(pipefd[0]); // Usually we close unused read end


        char *text = "Hajra Fradi!";
        int textLength = strlen(text) + 1; // +1 => \0 miatt

        write(pipefd[1], &textLength, sizeof(textLength));
        write(pipefd[1], text, textLength);

        write(pipefd[1], "Hajra Fradi!2", 14);

        close(pipefd[1]); // Closing write descriptor
        fflush(NULL); // flushes all write buffers (not necessary)

        printf("Szulo megvarja amig gyerek terminal\n");
        wait(NULL);	  // waiting for child process to terminate (not necessary)
        printf("Szulo befejezte!\n");
    }


    return 0;
}