#include <sys/shm.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <fstream>
#include <iostream>
#include <sys/ipc.h>  
#include "msg.h"      /* For the message struct */
using namespace std;

/* The size of the shared memory chunk */
#define SHARED_MEMORY_CHUNK_SIZE 1000

/* The ids for the shared memory segment and the message queue */
int shmid, msqid;

/* The pointer to the shared memory */
void* sharedMemPtr;

/**
 * Sets up the shared memory segment and message queue
 * @param shmid - the id of the allocated shared memory
 * @param msqid - the id of the shared memory
 * @param sharedMemPtr - the pointer to the shared memory
 */

/*
key_t createKeyFile(string fileName)
{
    //Open output stream with file name as parameter
    ofstream outputFile (fileName.c_str());
    // Input with the following text
    outputFile << "Hello World!" << endl;
    //Close the output steam
    outputFile.close();
    //Set the key
    key_t keyFile = ftok(fileName.c_str(), 'a');
    
    return keyFile;
}
*/ 
void init(int& shmid, int& msqid, void*& sharedMemPtr)
{
   
    //key_t key = createKeyFile("keyfile.txt");
    
    key_t key = ftok("keyfile.txt",'a');

    if(key == -1) 
    {
       	   perror("ftok: file name key failed");
           exit(1);
    }
   
    
    if ((shmid = shmget(key,SHARED_MEMORY_CHUNK_SIZE, IPC_CREAT | 0660)) == -1)
    {
          perror("shmget: shared memory segment failed");
          exit(1);
        
    }
    
    
    sharedMemPtr = shmat(shmid, (void *)0, 0);
    if (sharedMemPtr == (void *) -1)
    {
	  perror("shmat: shared memory pointer failed"); 
	  exit(1);
    }

      
        
        /*Create a message queue */
    
    if((msqid = msgget(key, IPC_CREAT | 0660)) == -1)
    {
           perror("msgget: msgget failed");
           exit(1);
    }
    

    
}

/**
 * Performs the cleanup functions
 * @param sharedMemPtr - the pointer to the shared memory
 * @param shmid - the id of the shared memory segment
 * @param msqid - the id of the message queue
 */

void cleanUp(const int& shmid, const int& msqid, void* sharedMemPtr)
{
    int temp;
    //Got the same clean up file from recv.cpp since it works
	//reciever clean up is different, -Eric
    temp = shmdt(sharedMemPtr);
    if(temp == -1)
    {
        perror("shmdt: shmdt failed");
	exit(1);
    }
}


/**
 * The main send function
 * @param fileName - the name of the file
 */
void send(const char* fileName)
{
    /* Open the file for reading */
    FILE* fp = fopen(fileName, "r");
	printf("File opened\n");    


    int msgSize = 0;

    /* A buffer to store message we will send to the receiver. */
    message sndMsg;
    
    /* A buffer to store message received from the receiver. */
    message rcvMsg;
    
   //// Length of message in bytes (argument for msgsnd)


    /* Was the file open? */
    if(!fp)
    {
        perror("fopen");
        exit(-1);
    }
    
    /* Read the whole file */
    while(!feof(fp))
    {
       
        if((sndMsg.size = fread(sharedMemPtr, sizeof(char), SHARED_MEMORY_CHUNK_SIZE, fp)) < 0)
        {
            perror("fread");
            exit(-1);
        }
        
        
	//Set the message type (this was also set in the main loop of recv), but obviously for recv type
	sndMsg.mtype = SENDER_DATA_TYPE; 
	
	//Check SENDING message size by redefining msgSize (same as the function in recv.cpp)
	msgSize = msgsnd(msqid, &sndMsg,sizeof(message)-sizeof(long),0);
	if(msgSize == -1)
        {
	    perror("msgsnd: sending message failed");
            exit(1);
	}
	printf("Message sent, waiting on reciever\n"); 



	 //Check RECEIVED message size by redefining msgSize (same as function in recv.cpp)
	msgSize = msgrcv(msqid, &rcvMsg, sizeof(message)-sizeof(long), RECV_DONE_TYPE, 0);
	if(msgSize == -1)
	{
	    perror("msgrcv: receiving message failed");
            exit(1);
	}
	printf("Recieved message\n");

    }
    
	
	
	sndMsg.mtype = SENDER_DATA_TYPE;
	sndMsg.size = 0;  
	msgSize = msgsnd(msqid, &sndMsg,sizeof(message)-sizeof(long),0);
	if(msgSize == -1)
	{
	     perror("sgsnd: sending message failed");
	     exit(1);
	}
	printf("End of file, closing\n");

    
    /* Close the file */
    fclose(fp);
    
    
}


int main(int argc, char** argv)
{

    
    if(argc < 2)
    {
        fprintf(stderr, "USAGE: %s <FILE NAME>\n", argv[0]);
        exit(-1);
    }
    
    /* Connect to shared memory and the message queue */

    init(shmid, msqid, sharedMemPtr);
    
    /* Send the file */
    send(argv[1]);
    
    /* Cleanup */
    cleanUp(shmid, msqid, sharedMemPtr);
    
	
   printf("made it to the end");

    return 0;
}


