#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>


int main (int argc,char* argv[]) {

   key_t kulcs = ftok(argv[0],1); // a parancs nevevel es az 1 verzio szammal kulcs generalas
   int oszt_mem_id = shmget(kulcs, 500, IPC_CREAT|S_IRUSR|S_IWUSR); // osztott memoria letrehozasa, irasra olvasasra, 500 bajt mrettel
   
   char *s;
   s = shmat(oszt_mem_id,NULL,0);    // csatlakozunk az osztott memoriahoz, a 2. parameter akkor kell, ha sajat cimhez akarjuk illeszteni
   // a 3. paraméter lehet SHM_RDONLY, ekkor csak read van


   pid_t child;
   child = fork();
   if(child>0) //parent
   { 
    
      char buffer[] = "Hajra Fradi! \n"; 
      strcpy(s,buffer); // beirunk a memoriaba 
   
      shmdt(s);   // elengedjuk az osztott memoriat  
      wait(NULL); 
      // IPC_RMID- torolni akarjuk a memoriat, ekkor nem kell 3. parameter
      // IPC_STAT- osztott memoria adatlekerdezes a 3. parameterbe,
      //  ami shmid_ds struct tipusu mutato
      shmctl(oszt_mem_id,IPC_RMID,NULL);  //FONTOS!! Csak akkor töröljük az osztott memóriát, ha már kiolvasta.
   } 
   else if ( child == 0 ) 
	{
	   sleep(1); //aludjunk, hogy addig beirja az adatokat a másik process. //ne legyen "konkurens modifikáció"              
      printf("A gyerek ezt olvasta az osztott memoriabol: %s",s);
      shmdt(s);// gyerek is elengedi
	}

   return 0;
}
