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
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include <sys/shm.h>

void handler(int signumber) {
  printf("Gyerek készen áll\n", signumber);
}

struct uzenet { 
    long mtype; // ez egy szabadon hasznalhato ertek, pl uzenetek osztalyozasara
    char mtext [1024]; 
};


int main(int argc, char** argv){

    signal(SIGUSR1,handler);

    char pipename[20];
    sprintf(pipename, "/tmp/%d", getpid());
    int fid = mkfifo(pipename, S_IRUSR | S_IWUSR); // creating named pipe file

    char pipename2[20];
    sprintf(pipename2, "/tmp/%d234234", getpid());
    int fid2 = mkfifo(pipename2, S_IRUSR | S_IWUSR); // creating named pipe file
    
    char textBuffer[1024];


    key_t key = ftok(argv[0],1); // random kulcsot general -> ha nem jo akkor másik szám 2.param-nak
    int uzenetsor = msgget( key, 0600 | IPC_CREAT ); // 0600 user-nek olvasas és írás jog

    key_t kulcs = ftok(argv[0],52); // a parancs nevevel es az 52 verzio szammal kulcs generalas
    int oszt_mem_id = shmget(kulcs, 500, IPC_CREAT|S_IRUSR|S_IWUSR); // osztott memoria letrehozasa, irasra olvasasra, 500 bajt mrettel
    int* meresSzam;
    meresSzam = shmat(oszt_mem_id,NULL,0);


    pid_t child1, child2;
    child1 = fork();

    if(child1>0)
    {

        child2 = fork();
        if(child2>0)
        {   //parent, moderator

            pause();
            pause();
            printf("Szülő indul\n");
            int fd = open(pipename, O_WRONLY);  //open pipe for writing
            char* text = "Megvan minden dokumentuma?"; 
            write(fd, text, (strlen(text) + 1)); //nem kell &text, mert a text már eleve pointer
            close(fd);


            int fd2 = open(pipename2, O_RDONLY);  //open pipe for writing
            read(fd2, textBuffer, sizeof(textBuffer)); //honnan, hova, mekkorát olvas (csak \0-ig olvas)
            close(fd2);
            unlink(pipename2);
            printf("Moderátor üzenetet kap: - '%s'\n",textBuffer);


            struct uzenet uz;
            int status;

            status = msgrcv(uzenetsor, &uz, 1024, 5, 0); // honnan, mibe, mekkorát, milyen kategóriájút, ...
            //0 -> bevárja a küldést, ha nem jön msgsnd, akkor deadlock
            if (status < 0){
                perror("msgrcv");
            }

            printf("A moderátor üzenetsoron kapott üzenetet: - '%s'\n", uz.mtext);
            
	        sleep(2); //aludjunk, hogy addig beirja az adatokat a másik process. //ne legyen "konkurens modifikáció"              
            printf("A moderátor osztott memóriából ezt a mérésszámot olvasta:  %i\n",*meresSzam);

            shmdt(meresSzam);
            shmctl(oszt_mem_id,IPC_RMID,NULL);
            msgctl(uzenetsor, IPC_RMID, NULL);
            wait(NULL);
            wait(NULL);
        }
        else
        {   //child2, szakértő
            sleep(2);
            kill(getppid(),SIGUSR1);


            int fd = open(pipename, O_RDONLY);  //open pipe for writing
            read(fd, textBuffer, sizeof(textBuffer)); //honnan, hova, mekkorát olvas (csak \0-ig olvas)
            close(fd);
            unlink(pipename);
            printf("Szakértő üzenetet kap: - '%s'\n",textBuffer);

            int fd2 = open(pipename2, O_WRONLY);  //open pipe for writing
            char* text2 = "Igen"; 
            write(fd2, text2, (strlen(text2) + 1)); //nem kell &text, mert a text már eleve pointer
            close(fd2);
        }
    }
    else
    {   //child1, nyilatkozó
        sleep(1);
        kill(getppid(),SIGUSR1);


        const struct uzenet uz = {5, "Igen, méréseink vannak, amiket publikálni fogunk."};
        int status;
        status = msgsnd(uzenetsor, &uz, strlen(uz.mtext) + 1, 0);
        if (status < 0){
            perror("msgsnd");
        }


        *meresSzam = 123;
        shmdt(meresSzam);
        printf("gyerek előbb irt a memóriába\n");
    }


    return 0;
}