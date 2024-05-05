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
    int uzenetsor;
    key_t key = ftok(argv[0],1); // random kulcsot general -> ha nem jo akkor másik szám 2.param-nak
    uzenetsor = msgget( key, 0600 | IPC_CREAT ); // 0600 user-nek olvasas és írás jog


    child1 = fork();
    if(child1==0){
        const struct uzenet uz = {3, "Hajra Fradi3!"};
        int status;
        sleep(3); // sleep hogy lássuk, hogy a msgrcv bevérja a msgsnd-et
        status = msgsnd(uzenetsor, &uz, strlen(uz.mtext) + 1, 0);
        // a 3. param ilyen is lehet: sizeof(uz.mtext) vagy sizeof(uzenet) - sizeof(long)
        // a 4. parameter gyakran IPC_NOWAIT, ez a 0-val azonos ??? -> nem kell foglalkozni vele
        if (status < 0){
            perror("msgsnd");
        }

        return 0;
    }

    child2 = fork();
    if(child2==0){

        const struct uzenet uz = {5, "Hajra Fradi5!"};
        int status;
        status = msgsnd(uzenetsor, &uz, strlen(uz.mtext) + 1, 0); //hova, min keresztül, mekkorát, NINCS KATEGÓRIA!!!, 0 -> nem kell vele foglalkonzi
        if (status < 0){
            perror("msgsnd");
        }

        return 0;
    }
    //parent

    struct uzenet uz;
    int status;


    status = msgrcv(uzenetsor, &uz, 1024, 3, 0); // honnan, mibe, mekkorát, milyen kategóriájút, ...
    //0 -> bevárja a küldést, ha nem jön msgsnd, akkor deadlock
    if (status < 0){
        perror("msgrcv");
    }
    else{
        printf("A kapott uzenet kodja: %ld, szovege:  %s\n", uz.mtype, uz.mtext);
    }


    status = msgrcv(uzenetsor, &uz, 1024, 5, IPC_NOWAIT);   // honnan, mibe, mekkorát, milyen kategóriájút, ...
    //IPC_NOWAIT -> nem várja meg a küldést, tehát ha előbb van a msgrcv, mint msgsnd akkor tovább megy. ->ilyenkor érdemes sleep
    if (status < 0){
        perror("msgrcv");
    }
    else{
        printf("A kapott uzenet kodja: %ld, szovege:  %s\n", uz.mtype, uz.mtext);
    }

    
    


    wait(NULL);
    wait(NULL);
    msgctl(uzenetsor, IPC_RMID, NULL); // After terminating child processes, the message queue is deleted.
    //FONTOS!!! csak akkor toroljuk a mq-t ha már megvolt minden, olvasás, különben "kihúzhatjuk az erőforrást a olvasó process alól"


    return 0;
}