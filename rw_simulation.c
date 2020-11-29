#include <math.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h> 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <time.h>
#include <sys/time.h>


union senum {
	int val;
	struct semid_ds *buff;
	unsigned short *array;
};


int main(int argv, char** argc){
/* Diavasma, elegxos kai ektupwsh twn dedomenwn pou eisagontai */
	if(argv<5){
		printf("Incorrect input.Exiting..\n");
		exit(1);
	}
	int peer_num,entries_num, rep_num, readers_percentage,shmid,semid, *data_ptr , error,i;
	peer_num= atoi(argc[1]);
	entries_num=atoi(argc[2]);
	rep_num=atoi(argc[3]);
	readers_percentage=atoi(argc[4]);
	if(readers_percentage< 0 || readers_percentage>100 || peer_num<1 || entries_num<1 || rep_num<1){
		printf("Incorrect input.Exiting.. \n");
		exit(1);
	}

	printf("Number of Peers:%d\nNumber of Entries:%d\nNumber of Repetitions:%d\nPercentage of Readers:%d%% and Writers:%d%%\n",peer_num,entries_num, rep_num, readers_percentage,100-readers_percentage);
 /*   */ 
 /* dhmiourgia shared memory pou den uparxei hdh (ipc_exl) me key to process id kai dikaiwmata read kai write se olous (0666) */
	shmid = shmget( (key_t)getpid() , sizeof(int)*3*entries_num, IPC_CREAT | IPC_EXCL | 0666);  // nbytes gia to shared memory: (ena int gia to counter twn writers +ena gia to counter twn readers + gia to counter twn current readers )* arithmo twn entries
	if(shmid == -1){
		perror("Creating Shared Memory Error");
		exit(1);
	}
/* */
/* dhmiourgia shmaforou gia kathe ena apo ta int ths parapanw shared memory */ 
	semid = semget((key_t) getpid(), 3*entries_num , IPC_CREAT | IPC_EXCL | 0666);
	if(semid == -1){
		perror("Creating Semaphores Error");
		exit(1);
	}
/* */
	union senum arg;
	arg.val=1;
	for(i=0;i<3*entries_num;i++){				//arxikopoihsh olwn twn shmaforwn se 1
		semctl(semid, i, SETVAL, arg);			//sem=arg.val
	}


	data_ptr=shmat(shmid,NULL,0); // data_ptr pointer sthn arxh twn data ths shared memory
	if(data_ptr==NULL){
		perror("Attaching Shared Memory Error");
		exit(1);
	}

	for(i=0;i<entries_num*3;i++){
		data_ptr[i]=0; 	// arxikopoihsh twn counter tou shared memory se 0
	}
	/* dhmiourgia twn peer me fork*/
	pid_t pid;  //sto pid tha ginei to return ths fork
	for(i=0; i < peer_num ; i++){
		pid=fork();
		if(pid<0){
			perror("Fork Error");
			exit(1);
		}
		else if(pid==0){	// if child-> break
			break;
		}
	}
	/* */
	
	/* kwdikas gia peer/child */
	if(pid==0){  
		struct timeval stop, start;
		double u;
		int l=8,sleeptime;
		int *reader_count=data_ptr; // o reader_counter einai pointer sthn arxh twn counter pou aforoun tous current readers
		int *total_writes = data_ptr+entries_num; //total_writes pointer sthn arxh twn counter twn sunolikwn write pou exoun ginei  
		int *total_reads=data_ptr+(2*entries_num); //total_reads pointer sthn arxh twn counter twn sunolikwn read pou exoun ginei
		int current_entry; 
		double waitingRead=0,waitingWrite=0;
		int proc_read=0,proc_write=0; // reads kai writes pou eginan sto kathe process/peer
		struct  sembuf oper[2];
		oper[0].sem_flg=0;  
		oper[0].sem_op=-1; // to oper[0] kanei panta down
		oper[1].sem_flg=0;
		oper[1].sem_op=1; // to oper[1] kanei panta up
		srand(time(0)-11*getpid()); 
		for(i=0 ; i < rep_num ; i++){
			/* READER */
			if( rand() <= RAND_MAX*(double)readers_percentage/100){  //rand gia to an h energeia tha einai read h write
				proc_read++;
				current_entry=rand()%entries_num; //rand gia to se poio entry tha ginei to read
				oper[0].sem_num=current_entry; // etoimazoume to down wste na ginei ston shmaforo twn current_readers (gia to current_entry)
				gettimeofday(&start, NULL);
				error=semop(semid, &oper[0], 1); // ginetai down o shmaforos
				if(error==-1){
					perror("Reader Semaphore Down Error");
					exit(1);
				}
				reader_count[current_entry]++;  // auksanoume ton counter twn energwn readers gia to sugekrimeno entry
				if(reader_count[current_entry]==1){      
					oper[0].sem_num=current_entry + entries_num;
					error=semop(semid, &oper[0], 1);		//an o reader einai o prwtos tote ginetai down o shmaforos twn writers (gia to sugekrimeno entry)
				 	if(error==-1){
				 		perror("Writer Semaphore Down Error");
						exit(1);
					}
				}
				
				
				oper[1].sem_num=current_entry; 
				error=semop(semid, &oper[1], 1); //up tous  readers
				if(error==-1){
					perror("Reader Semaphore Up Error");
					exit(1);
				}
				gettimeofday(&stop, NULL);
				waitingRead+=(stop.tv_sec - start.tv_sec) + ( stop.tv_usec - start.tv_usec)/1000000.0; //auksanoume xrono anamonhs tou reader kai metatroph tou se second 
				

				oper[0].sem_num=current_entry+ 2*entries_num;
				error=semop(semid, &oper[0], 1);		// down ton shmaforo twn sunolikwn readers prokeimenou na auksisoume ton total_reads counter
				if(error==-1){
					perror("Total Reads Semaphore Down");
					exit(1);		
				}											// den ginetai xronometrisi autou tou kommatiou dioti den einai meros ths sumperiforas enos reader. xrhsimopoieitai mono gia thn prosomoiwsh

				total_reads[current_entry]++;	//auksanoume ton counter twn sunolikwn reads
				
				oper[1].sem_num=current_entry+ 2*entries_num;
				error=semop(semid, &oper[1], 1);		// up ton shmaforwn twn total reads
				if(error==-1){
					perror("Total Reads Semaphore Up Error");
					exit(1);
				}
				
				u=(rand()%10000+1)/10000.0;   //tuxaios arithmos anamesa sto 0 kai sto 1

				sleeptime=(int)((-log(u))/l*100000);
				usleep(sleeptime);

				oper[0].sem_num=current_entry;
				gettimeofday(&start, NULL);				
				error=semop(semid, &oper[0], 1); //down ton shmaforo twn current readers prokeimenou na meiwsoume ton counter tous
				if(error==-1){
					perror("Reader Semaphore Down Error");
					exit(1);
				}
				gettimeofday(&stop, NULL);
				waitingRead+=(stop.tv_sec - start.tv_sec) + ( stop.tv_usec - start.tv_usec)/1000000.0;

				reader_count[current_entry]--;
				if(reader_count[current_entry]==0){
					oper[1].sem_num=current_entry + entries_num;
					error=semop(semid, &oper[1], 1);   //an o reader htan o teleutaios, up tous writers
				 	if(error==-1){
				 		perror("Writer Semaphore Up Error");
						exit(1);
					}
				}
				oper[1].sem_num=current_entry;
				error=semop(semid, &oper[1], 1);	//up tous readers
				if(error==-1){
					perror("Reader Semaphore Up");
					exit(1);
				}
			}



			/* WRITER */
			else{
				proc_write++;
				current_entry=rand()%entries_num;  //tuxaia epilogh tou entry pou tha ginei to write
				oper[0].sem_num=current_entry + entries_num;
				gettimeofday(&start, NULL);
				error=semop(semid, &oper[0], 1);	//down tous writers
				if(error==-1){
					perror("Writer Semaphore Down");
					exit(1);
				}
				gettimeofday(&stop, NULL);
				waitingWrite+=(stop.tv_sec - start.tv_sec) + ( stop.tv_usec - start.tv_usec)/1000000.0;
				total_writes[current_entry]++;
				
				u=(rand()%10000+1)/10000.0;
				sleeptime=(int)((-log(u))/l*100000);
				usleep(sleeptime);

				oper[1].sem_num=current_entry + entries_num;
				error=semop(semid, &oper[1], 1);	//up tous writers
				if(error==-1){
					perror("Writer Semaphore Up");
					exit(1);
				}				
			}
		}

		error= shmdt(data_ptr);
		if(error==-1){
			perror("Child Detachment Error");
			exit(1);
		}
		printf("Average Wait for proccess %d in seconds Total:%f ",getpid(),(waitingRead+waitingWrite)/rep_num);
		if(proc_read==0)
			printf("Readers:No Readers ");
		else{
			printf("Readers:%f ",waitingRead/proc_read);
		}

		if(proc_write==0){
			printf("Writers:No Writers ");
		}
		else{
			printf("Writers:%f ",waitingWrite/proc_write);
		}
		printf(" Number of Reads:%d Number of Writes:%d\n",proc_read,proc_write);
		exit(0);
	}
	/* */
	/* kwdikas gia parent/coordinator */
	else{
		for(i=0; i < peer_num; i++){
			error=wait(NULL);	// wait ola ta peer na teliwsoun
			if(error==-1){
				perror("Wait Error");
				exit(1);
			}
		}

		int  *total_entry_writes = data_ptr+entries_num; // total_writes pointer sthn arxh twn counter pou aforoun tous writers
		int  *total_entry_reads=data_ptr+(2*entries_num); //total_readers pointer sthn arxh twn counter pou aforoun tous readers
		int total_writes=0, total_reads=0;
		for(i=0;i<entries_num;i++){
			total_writes=total_writes+total_entry_writes[i];
			total_reads=total_reads+total_entry_reads[i];
			printf("Entry %d: Total #Reads:%d Total #Writes:%d\n",i+1,total_entry_reads[i], total_entry_writes[i]); //ektupwse ton counter tou kathe entry
		}
		printf("Total #Reads:%d Total #Writes:%d\n",total_reads, total_writes); //ektupwse ta sunolika reads kai writes

		error= shmdt(data_ptr);
		if(error==-1){
			perror("Parent Detachment Error");
			exit(1);
		}
		error= shmctl(shmid, IPC_RMID,0);
		if(error==-1){
			perror("Freeing Shared Memory Error");
			exit(1);
		}

		error= semctl(semid , 0, IPC_RMID , 0);
		if(error==-1){
			perror("Freeing Semaphores Error");
			exit(1);
		}
		exit(0);
	}
	/* */
}

