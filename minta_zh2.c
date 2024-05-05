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


void handler(int signumber) {
  //printf("Signal with number %i has arrived\n", signumber);
}

struct arany {
    long mtype;
    int beteg;
    int egeszseges;
    char nev[1024];
};


int main(int argc, char** argv){

    srand(time(NULL));
    signal(SIGUSR1,handler);

    char pipename[20];
    sprintf(pipename, "/tmp/%d", getpid());
    int fid = mkfifo(pipename, S_IRUSR | S_IWUSR); // creating named pipe file

    char pipename2[20];
    sprintf(pipename2, "/tmp/%d978234", getpid());
    int fid2 = mkfifo(pipename2, S_IRUSR | S_IWUSR); // creating named pipe file

    int uzenetsor;
    key_t key = ftok(argv[0],1); // random kulcsot general -> ha nem jo akkor másik szám 2.param-nak
    uzenetsor = msgget( key, 0600 | IPC_CREAT ); // 0600 user-nek olvasas és írás jog



    pid_t child1, child2;

    child1 = fork();
    if(child1>0)
    {
        child2 = fork();
        if(child2>0)//parent
        {
            pause();
            pause();
            int paciensek = atoi(argv[1]);
            printf("paciensek szama: %i\n",paciensek);


            int fd = open(pipename, O_WRONLY);  //open pipe for writing
            int fd2 = open(pipename2, O_WRONLY);
            int fele = paciensek/2;

            if(paciensek%2==0){
                int c = write(fd, &fele, sizeof(int));
                c = write(fd2, &fele, sizeof(int));
            }
            else{
                int masikfele = fele+1;
                int c = write(fd, &fele, sizeof(int));
                c = write(fd2, &masikfele, sizeof(int));
            }
            close(fd);
            close(fd2);


            //csak azért, hogyha esetleg jelzésre kellene kiolvasni:
            sleep(1); 
            kill(child1, SIGUSR1);
            kill(child2, SIGUSR1);

            struct arany aranyok1;
            int status = msgrcv(uzenetsor, &aranyok1, sizeof(aranyok1)-sizeof(long), 5, 0);   // honnan, mibe, mekkorát, milyen kategóriájút, ...
            if (status < 0){
                perror("msgrcv");
            }
            printf("%s -től érkezett oltottak száma: %i, és betegek száma: %i\n",aranyok1.nev, aranyok1.egeszseges, aranyok1.beteg);

            struct arany aranyok2;
            status = msgrcv(uzenetsor, &aranyok2, sizeof(aranyok2)-sizeof(long), 5, 0);   // honnan, mibe, mekkorát, milyen kategóriájút, ...
            if (status < 0){
                perror("msgrcv");
            }
            printf("%s -től érkezett oltottak száma: %i, és betegek száma: %i\n",aranyok2.nev, aranyok2.egeszseges, aranyok2.beteg);

            int kapott = aranyok1.egeszseges+aranyok2.egeszseges;
            int nemkapott = aranyok1.beteg+aranyok2.beteg;
            FILE* file;
            file = fopen("naplo.txt","w");
            fprintf(file,"%i db oltásra érkezőből %i kapott és %i nem kapott oltást.\n",paciensek,kapott,nemkapott);
            fclose(file);


            //fajlba
            //fputs ->ilyenkor lehet formazni sprintf-el 
            //, fprintf -> ez formaz alapbol


            msgctl(uzenetsor, IPC_RMID, NULL);
            wait(NULL);
            wait(NULL);
        }
        else //child2, csőrmester
        {
            sleep(2);
            kill(getppid(), SIGUSR1);

            int paciensek;
            int fd2 = open(pipename2, O_RDONLY);  //open pipe for writing
            pause(); //csak azért, ha "jelzésre kellene kiolvasni"
            read(fd2,&paciensek, sizeof(int));
            close(fd2);
            unlink(pipename2);

            printf("Őrmester %i db pacienst kapott.\n",paciensek);

            struct arany aranyok = {5,0,0,"Őrmester"};
            for (int i = 0; i < paciensek; i++)
            {
                if(rand() % 5 == 1){
                    aranyok.beteg++;
                }
            }
            printf("Őrmester betegei: %i\n",aranyok.beteg);
            aranyok.egeszseges = paciensek-aranyok.beteg;

            int status = msgsnd(uzenetsor, &aranyok, sizeof(aranyok) - sizeof(long), 0);
            if (status < 0){
                perror("msgsnd");
            }


            //printf("gyerek 2 vége\n");
        }
    }
    else //child1, ursula
    {
        sleep(1);
        kill(getppid(), SIGUSR1);

        int paciensek;
        int fd = open(pipename, O_RDONLY);  //open pipe for writing
        pause(); //csak azért, ha "jelzésre kellene kiolvasni"
        read(fd,&paciensek, sizeof(int));
        close(fd);
        unlink(pipename);

        printf("Ursula %i db pacienst kapott.\n",paciensek);
        sleep(3);

        struct arany aranyok = {5,0,0,"Ursula"};
        for (int i = 0; i < paciensek; i++)
        {
            if(rand() % 5 == 0){
                aranyok.beteg++;
            }
        }
        printf("Ursula betegei: %i\n",aranyok.beteg);
        aranyok.egeszseges = paciensek-aranyok.beteg;

        int status = msgsnd(uzenetsor, &aranyok, sizeof(aranyok) - sizeof(long), 0);
        if (status < 0){
            perror("msgsnd");
        }





        //printf("gyerek 1 vége\n");
    }



    return 0;
}