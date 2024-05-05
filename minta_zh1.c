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


struct szavazo {
    int id;
    int szavazhat;
};


struct arany{
    int ervenyes;
    int ervenytelen;
};


void handler(int signumber) {
  printf("Signal with number %i has arrived\n", signumber);
}

int main(int argc, char** argv){

    const int partok = 4;

    srand(time(NULL));
    signal(SIGUSR1,handler);

    int pipefd[2]; 
    if (pipe(pipefd) == -1) {
        perror("Hiba a pipe nyitaskor!");
        exit(EXIT_FAILURE);
    }
    int status;

    char pipename[20];
    sprintf(pipename, "/tmp/%d", getpid());
    int fid = mkfifo(pipename, S_IRUSR | S_IWUSR); // creating named pipe file

    char pipename2[20];
    sprintf(pipename2, "/tmp/%d978234", getpid());
    int fid2 = mkfifo(pipename2, S_IRUSR | S_IWUSR); // creating named pipe file


    char* mqname="/complextorzobarnabas";
	struct mq_attr attr;
	attr.mq_maxmsg=atoi(argv[0])+1; //MAXMSGS
	attr.mq_msgsize=sizeof(int); //MSGSIZE 
	mqd_t mq1;
    mq_unlink(mqname);
    mq1=mq_open(mqname, O_CREAT|O_RDWR,0600,&attr);


    pid_t child1, child2;


    child2 = fork();
    if(child2==0){
        //pecsételő
        close(pipefd[0]); //close unused unnamed pipe
        close(pipefd[1]);
        
        sleep(1);
        kill(getppid(), SIGUSR1);

        int fd2 = open(pipename2, O_RDONLY); //open named pipe
        pause();
        printf("child2 vette a drótot child1-től\n");

        int count;
        read(fd2, &count, sizeof(int)); // honnan, mibe, mekkorát olvasol
        struct szavazo* szavazok = (struct szavazo*) malloc(count * sizeof(struct szavazo));

        read(fd2, szavazok, count*sizeof(szavazok)); // honnan, mibe, mekkorát olvasol
        close(fd2);
        unlink(pipename2);


        struct arany aranyok = {0,0};
        for (int i = 0; i < count; i++)
        {
            if(szavazok[i].szavazhat){
                aranyok.ervenyes++;
            }
            else{
                aranyok.ervenytelen++;
            }

            printf("id: %d, szavazhat: %s",szavazok[i].id, szavazok[i].szavazhat ? "igen\n" : "nem\n");
        }

        
        int fd = open(pipename, O_WRONLY); //open named pipe
        write(fd, &aranyok, sizeof(aranyok)); // melyik file descriptorba, mit és mekkorát írsz
        close(fd);

        sleep(2);
        int db=mq_send(mq1,(char*) &aranyok.ervenyes,sizeof(int),5);

        for (int i = 0; i < aranyok.ervenyes; i++)
        {
            int randomPart = ((rand() % partok)+1);
            mq_send(mq1, (char*) &randomPart, sizeof(int),5);
        }
        

    	mq_close(mq1);

        printf("child2BYEBYE\n");
        free(szavazok);
        return 0;
    }

    child1 = fork();
    if(child1==0){
        //adatellenőrző
        sleep(3);
        kill(getppid(), SIGUSR1);

        close(pipefd[1]);
        int count;
        read(pipefd[0], &count, sizeof(int));

        int* ids = (int*) malloc(count * sizeof(int));
        read(pipefd[0], ids, count * sizeof(int));
        close(pipefd[0]);


        printf("Adatellenőrző fogadta az adatokat:\n");
        for (int i = 0; i < count; i++)
        {
            printf("%d\n",ids[i]);
        }


        int fd2 = open(pipename2, O_WRONLY);
        write(fd2, &count, sizeof(int)); // melyik file descriptorba, mit és mekkorát írsz        

        struct szavazo* szavazok = (struct szavazo*) malloc(count * sizeof(struct szavazo));

        for (int i = 0; i < count; i++)
        {
            szavazok[i].id = ids[i];
            szavazok[i].szavazhat = rand()%5!=1;
        }


        write(fd2, szavazok, count*sizeof(szavazok)); // melyik file descriptorba, mit és mekkorát írsz
        close(fd2);


        sleep(1);
        kill(child2,SIGUSR1);
        
        
        free(szavazok);
        printf("child1BYEBYE\n");
        return 0;
    }

    pause();
    pause();

    
    int count = atoi(argv[1]);
    int* ids = (int*) malloc(count*sizeof(int));
    for (int i = 0; i < count; i++)
    {
        ids[i] = rand() % 100;
    }

    close(pipefd[0]);
    write(pipefd[1], &count, sizeof(int));
    write(pipefd[1], ids, count * sizeof(int)); //nem kell &, mert az ids eleve pointer
    close(pipefd[1]);



    struct arany aranyok;
    int fd = open(pipename, O_RDONLY);    //open named pipe
    read(fd, &aranyok, sizeof(aranyok)); // honnan, hova, mekkorát olvasol
    close(fd);
    unlink(pipename);


    FILE *file = fopen("arany.txt", "w");
    if (file == NULL) {
        fclose(file);
        return 1;
    }

    fprintf(file, "Ervenyes szavazatok: %d\n", aranyok.ervenyes);
    fprintf(file, "Ervenytelen szavazatok: %d\n", aranyok.ervenytelen);
    int sum = aranyok.ervenytelen + aranyok.ervenyes;
    fprintf(file, "Ervenyesek aránya: %2.f%%\n", aranyok.ervenytelen == 0 ? 100 : (double)aranyok.ervenyes / sum * 100);
    fprintf(file, "Ervenytelenek aránya: %2.f%%", aranyok.ervenyes == 0 ? 100 : (double)aranyok.ervenytelen / sum * 100);
    printf("ervenyesek: %d, ervenytelenek: %d\n",aranyok.ervenyes,aranyok.ervenytelen);
    fclose(file);


    int szavazokszama; // repalce with aranyok.ervenyes
    int db=mq_receive(mq1, (char*) &szavazokszama, sizeof(int),NULL);
    printf("ervenyes szavazok szama: %i\n",szavazokszama);

    int szavazatok [partok];
    for (int i = 0; i < partok; i++) {
        szavazatok[i] = 0;
    }

    for (int i = 0; i < szavazokszama; i++)
    {
        int partszam;
        mq_receive(mq1, (char*) &partszam, sizeof(int), NULL);
        printf("Párt száma: %i\n",partszam);
        szavazatok[partszam-1]++;
    }

    printf("megjött az összes üzenet\n");
    
    int max = -1;
    int maxi = -1;
    for (int i = 0; i < partok; i++)
    {
        if(szavazatok[i]>=max){
            max = szavazatok[i];
            maxi = i;
        }
        printf("%i. párt szavazatainak száma: %i\n",(i+1),szavazatok[i]);
    }

    printf("A választást a %i. párt nyerte, %i szavazattal\n",maxi+1,max);


    



    waitpid(child1, &status, 0);
    waitpid(child2, &status, 0);

    printf("parent byebye\n");

    mq_close(mq1);
    mq_unlink(mqname);
    free(ids);
    return 0;
}