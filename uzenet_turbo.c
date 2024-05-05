#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <sys/ipc.h> 
#include <sys/msg.h> 

struct uzenet { 
    long mtype; // ez egy szabadon hasznalhato ertek, pl uzenetek osztalyozasara
    char mtext [1024]; 
};

int main(int argc, char** argv){
    pid_t child1, child2; 
    int mq, status; 
    key_t key = ftok(argv[0],1); // random kulcsot general 1 helyett valami random szám
    mq = msgget( key, 0600 | IPC_CREAT ); // 0600 user-nek olvasas és írás jog

    child1 = fork();
    if(child1 > 0){ //Parent
        child2 = fork();
        if(child2 > 0){ //Still parent
            struct uzenet kapott;
            sleep(3);
            for(int i = 0; i < 10; ++i){
                status = msgrcv(mq, &kapott, 1024, 2, IPC_NOWAIT);
                if(status == -1){
                    status = msgrcv(mq, &kapott, 1024, 1, 0);
                }
                printf(kapott.mtext);
            }
            status = msgrcv(mq, &kapott, 1024, 3, 0);
            printf(kapott.mtext);
            status = msgctl( mq, IPC_RMID, NULL );
        }

        else{ //Child2
            const struct uzenet kuldott = { 2, "Gyerek 2 vagyok!\n" };
            sleep(1);
            for(int i = 0; i < 5; ++i){
                status = msgsnd( mq, &kuldott, strlen ( kuldott.mtext ) + 1 , 0);
                printf("Gyerek 2 küldött.\n");
            }
        }
    }

    else{ //Child1
        struct uzenet kuldott = { 1, "Gyerek 1 vagyok!\n" };
        for(int i = 0; i < 5; ++i){
            status = msgsnd( mq, &kuldott, strlen ( kuldott.mtext ) + 1 , 0);
            printf("Gyerek 1 küldött.\n");
        }
        sleep(8);
        kuldott.mtype = 3;
        strcpy(kuldott.mtext, "Extra üzenet\n");
        status = msgsnd( mq, &kuldott, strlen ( kuldott.mtext ) + 1 , 0);
        printf("Extra üzenet elküldve.\n");
    }
}