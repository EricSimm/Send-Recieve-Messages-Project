#include <sys/shm.h>
#include <sys/msg.h>
#include <signal.h>
#include <stdio.h>
#include <fstream>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <unistd.h>
#include "msg.h"    /* For the message struct */
//#include "sndr.cpp"
using namespace std;


/* The size of the shared memory chunk */
#define SHARED_MEMORY_CHUNK_SIZE 1000

/* The ids for the shared memory segment and the message queue */
int shmid, msqid;

/* The pointer to the shared memory */
void *sharedMemPtr;

/* The name of the received file */
const char recvFileName[] = "recvfile";


/**
 * Sets up the shared memory segment and message queue
 * @param shmid - the id of the allocated shared memory
 * @param msqid - the id of the shared memory
 * @param sharedMemPtr - the pointer to the shared memory
 */



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


void init(int& shmid, int& msqid, void*& sharedMemPtr)
{
    
    /* Store the IDs and the pointer to the shared memory region in the corresponding parameters */
    //generates a key
    //key_t key = ftok("keyfile.txt", 'a');
    //Replaced the code of line above with:
    
    key_t key = createKeyFile("keyfile.txt");
    cout << "key for receiver is: " << key;
    if(key == -1)
    {
        perror("ftok: file name key failed");
        exit(1);
    }
    
    /* Allocate a piece of shared memory. The size of the segment must be SHARED_MEMORY_CHUNK_SIZE. */
    // the 0660 paramter is a unix file system permissions that allows user and groups to read/write
    // Return error message if does not get shared memory segment
    if ((shmid = shmget(key,SHARED_MEMORY_CHUNK_SIZE, IPC_CREAT | 0660)) == -1)
    {
        perror("shmget: shared memory segment failed");
        exit(1);
        
    }
    
    /*Attach to the shared memory */
    // Return error message if it does not attach
    sharedMemPtr = shmat(shmid,(void*)0,0);



        /*Create a message queue */
    msqid = msgget(key, IPC_CREAT | 0660);
        if ( msqid == -1)
        {
            perror("msgget: msgget failed");
            exit(1);
        }
    
    //sources:
    //http://www.cs.cf.ac.uk/Dave/C/node27.html
    //www-01.ibm.com/support/knowledgecenter/
    //http://beej.us/guide/bgipc/output/html/multipage/mq.html
    //http://man7.org/linux/man-pages/man2/msgget.2.html
    //http://www.rainydayz.org/beej/bgipc/mq.html#mqftok
    
    // Flag permissions
    //http://space.wccnet.edu/~chasselb/linux275/ClassNotes/ipc/shared_mem.htm
    
}


/**
 * The main loop
 */
void mainLoop()
{
    /* Open the file for writing */
    FILE* fp = fopen(recvFileName, "w");
    
    /* Error checks */
    if(!fp)
    {
        perror("fopen: open file failed");
        exit(-1);
    }
    
    /* The size of the mesage */
    int msgSize = 0;
    
    
    
    // Create a messages
    struct message sendMessage;
    struct message recvMessage;
    
    
    
    // Length of message in bytes (argument for msgrcv)
    const int sizeInBytes = sizeof(recvMessage)-sizeof(long);
    
    // Message Size
    msgSize = msgrcv(msqid, &recvMessage, sizeInBytes, SENDER_DATA_TYPE, 0);
    cout << "msqid: " << msqid <<endl << "recvMessage: " << &recvMessage<<endl<< " sizeinBytes: " << sizeInBytes << endl;
    
    if(msgSize == -1)
    {
        perror("msgrcv: message receive failed");
        exit(1);
    }
    
    
    
    while(msgSize != 0)
    {
        /* If the sender is not telling us that we are done, then get to work */
        if(msgSize != 0)
        {
            
            
            /* Save the shared memory to file */
            if(fwrite(sharedMemPtr, sizeof(char), msgSize, fp) == -1)
            {
                perror("fwrite: writing to file failed");
            }
            
            //Sending message of type RECV_DONE_TYPE
            sendMessage.mtype=RECV_DONE_TYPE;
            
            //Sending message
            //Check SENDING message size by redefining msgSize
            msgSize = msgsnd(msqid, &sendMessage, sizeInBytes,0);
            
            if( msgSize == -1)
            {
                perror("msgsnd: sending message failed");
                exit(1);
            }
            
            //Continue receiving meessages
            //Check RECEIVED message size by redefining msgSize
            msgSize = msgrcv(msqid, &recvMessage, sizeInBytes, SENDER_DATA_TYPE, 0);
            
            if(msgSize == -1)
            {
                perror("msgrcv: message receive failed");
                exit(1);
            }
            
            
        }
        /* We are done */
        else
        {
            /* Close the file */
            fclose(fp);
        }
    }
}



/**
 * Perfoms the cleanup functions
 * @param sharedMemPtr - the pointer to the shared memory
 * @param shmid1 - the id of the shared memory segment
 * @param msqid1 - the id of the message queue
 */

void cleanUp(const int& shmid, const int& msqid, void* sharedMemPtr)
{
    int temp;
    //refer to http://www.cs.cf.ac.uk/Dave/C/node27.html for handling shared memory
    //http://beej.us/guide/bgipc/output/html/multipage/mq.html for handling message queues
    /*Detach from shared memory */
    temp = shmdt(sharedMemPtr);
    if(temp == -1)
    {
        perror("shmdt: shmdt failed");
    }
    /*Deallocate the shared memory chunk */
    temp = shmctl(shmid, IPC_RMID, NULL);
    if(temp == -1)
    {
        perror("schmid: schmid failed");
    }
    temp = msgctl(msqid, IPC_RMID, NULL);
    /*Deallocate the message queue */
    if(temp == -1)
    {
        perror("msgctl, msgctl failed");
    }
}

/**
 * Handles the exit signal
 * @param signal - the signal type
 */

void ctrlCSignal(int signal)
{
    /* Free system V resources */
    cleanUp(shmid, msqid, sharedMemPtr);
}

int main(int argc, char** argv)
{
    
    /* Install a singnal handler (see signaldemo.cpp sample file).
     * In a case user presses Ctrl-c your program should delete message
     * queues and shared memory before exiting. You may add the cleaning functionality
     * in ctrlCSignal().
     */
    signal(SIGINT, ctrlCSignal);
    
    /* Initialize */
    init(shmid, msqid, sharedMemPtr);
    cout << "we made it past init"<<endl;
    /* Go to the main loop */
    mainLoop();
    
    /**Detach from shared memory segment, and deallocate shared memory and message queue (i.e. call cleanup) **/
    
    cleanUp(shmid, msqid, sharedMemPtr);
    return 0;
}
