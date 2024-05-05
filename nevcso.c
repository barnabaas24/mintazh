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
#include <errno.h>


int main(int argc, char *argv[]) {

    int fd;
    char pipename[20];
    sprintf(pipename, "/tmp/%d", getpid()); //bufferbe beleirja a 2. paramétert, ami formázható
    int fid = mkfifo(pipename, S_IRUSR | S_IWUSR); // creating named pipe file
    // S_IWGRP, S_IROTH (other jog), file permission mode
    if (fid == -1) {
        printf("Error number: %i", errno);
        perror("Gaz van:");
        exit(EXIT_FAILURE);
    }


    pid_t child = fork();

    if(child>0) //parent
    {
        char buffer[1024];
        fd = open(pipename, O_RDONLY);  //open pipe for reading
        int c = read(fd,buffer,sizeof(buffer)); //honnan, hova, mekkorát olvas (csak \0-ig olvas)
        close(fd);  //close after reading

        printf("Ezt olvastam a csobol: %s - %i \n", buffer,c);

        unlink(pipename);   //unlink after no more usage
    }
    else    //child
    {
        fd = open(pipename, O_WRONLY);  //open pipe for writingú
        int c = write(fd, "Hajra Fradi!\n",12); // melyik file descriptorba, mit és mekkorát írsz
        printf("Ennyit irt a gyerek: %i\n",c);
        close(fd);  //close after writing
    }

    return 0;
}