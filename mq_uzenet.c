#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h> 
#include <fcntl.h>
#include <sys/stat.h> 
#include <sys/types.h> 
#include <sys/wait.h> 
#include <mqueue.h>
#include <errno.h>


struct asd {
    char arr[1200];
    int asd2;
};

int main(int argc, char* argv[]){

    char* mqname="/complextorzobarnabas";   // valami randomot adjunk, hogy ne létezzen még
    mq_unlink(mqname); // delete mq if exist

	struct mq_attr attr;
	attr.mq_maxmsg=5; //MAXMSGS
	attr.mq_msgsize=sizeof(struct asd); //MSGSIZE 
	//attr.mq_msgsize=sizeof(int); //->ha intet kuldunk 


	mqd_t mq1;
    mq1=mq_open(mqname, O_CREAT|O_RDWR,0600,&attr); //milyen névvel, milyen jogokkal, milyen attribútumokkal
	if ( mq1 < 0 ) { 
		printf("mq_open error: %d \n",errno); 
		return 1; 
	} 


    char buffer[1024];	// mq_send, mq_receive uses char array
	pid_t child; 

    child = fork(); 
    if (child > 0) { 
        char uzenet[30]="Hajra Fradi!";
        printf("uzenet mérete : %i\n",sizeof(uzenet));
        sleep(3); //kuldes elott varunk, hogy lassuk, az mq_receive megvarja a kuldest
	    int db=mq_send(mq1,uzenet,sizeof(uzenet), 5 );  //min keresztül, mit, mekkorát, prioritas
        //printf("ez viszont iras utan egybol tovabb megy\n");

        //=========INT küldése===============
        // int szam = 4;
        // sleep(5);
        // mq_send(mq1, (char*) &szam, sizeof(int), 5);


        mq_close(mq1);
        wait(NULL);
    }
    else{
        
        //sleep(5); //kuldes kozben alszunk, hogy lassuk, a kuldes utan egybol tovabb megy a masik process
        int db=mq_receive(mq1,buffer, 54,NULL); //mibol, hova, mekkorát, milyen prioritassal fogadunk
        printf("csak fogadás után megy tovább\n");
        printf("mq_receive : %s, olvasott bajtok hossza: %d\n",buffer, db);


        //=========INT fogadás===============
        // int fogadottszam;
        // int db = mq_receive(mq1,(char*) &fogadottszam, sizeof(int),NULL);
        // if(db==-1){
        //     printf("mq_receive error: %d\n",errno);
        // }
        // printf("Fogadott szám: %i\n",fogadottszam);


        mq_close(mq1); //close mq after no more usage
        mq_unlink(mqname); //unlink (delete) after no more usage 
        //FONTOS!!! addig ne töröljük az mq-t amig olvasas folyamatban van, mert "elvesszük az erőforrást"

        printf("gyerek leall\n");
        return 0; 
    }





    printf("parent leall\n");
    return 0;
}