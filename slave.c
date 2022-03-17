/*
Jacob Drew
4760 Project 3
slave.c

*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <sys/sem.h>

union semun
{
    int val;               /* value for SETVAL */
    struct semid_ds *buf;  /* buffer for IPC_STAT & IPC_SET */
    unsigned short *array; /* array for GETALL & SETALL */
    struct seminfo *__buf; /* buffer for IPC_INFO */
};

struct sembuf p = {0, -1, SEM_UNDO};
struct sembuf v = {0, +1, SEM_UNDO};

key_t key;
int semid;
union semun mySemaphore;
int childProc;

pid_t *children;

void handle_sigterm(int signum, siginfo_t *info, void *ptr)
{
    // detaching and deleting shared memory
    /* remove it: */
    // if (semctl(semid, 0, IPC_RMID, mySemaphore) == -1)
    // {
    //     perror("semctl");
    //     exit(1);
    // }

    fprintf(stderr, "Process #%i was terminated by master\n", childProc);
    exit(0);
}

void catch_sigterm()
{
    static struct sigaction _sigact;
    memset(&_sigact, 0, sizeof(_sigact));
    _sigact.sa_sigaction = handle_sigterm;
    _sigact.sa_flags = SA_SIGINFO;
    sigaction(SIGTERM, &_sigact, NULL);
}

int main(int argc, char *argv[])
{

    struct sembuf sb = {0, -1, 0}; /* set to allocate resource */

    if ((key = ftok("master.c", 'J')) == -1)
    {
        perror("ftok");
        exit(1);
    }
    /* grab the semaphore set created by seminit.c: */
    if ((semid = semget(key, 1, 0)) == -1)
    {
        perror("semget");
        exit(1);
    }

    char *mypid = malloc(6); // ex. 34567
    sprintf(mypid, "%s", argv[1]);

    char logName[80];
    strcpy(logName, "logfile.");
    strcat(logName, mypid);

    char nn[6];
    strcat(nn, argv[2]);

    FILE *outputFile;
    outputFile = freopen(logName, "w+", stdout);

    FILE *cTestStream;
    cTestStream = fopen("ctest", "a");

    pid_t parent;
    pid_t child;
    parent = getppid();
    child = getpid();
    childProc = (int)(child - parent);

    catch_sigterm();
    signal(SIGINT, SIG_IGN);

    int i;
    // child process loops five times
    for (i = 0; i < 5; i++)
    {

        if (semop(semid, &sb, 1) == -1)
        {
            perror("semop");
            exit(1);
        }

        // write to "c test"

        // get the time:
        char cTestString[100];
        time_t temp;
        struct tm *timeptr;
        temp = time(NULL);
        timeptr = localtime(&temp);

        // sleep for random number between 1-5
        int randomnumber;
        int pNum = atoi(argv[1]);
        srand(time(NULL) + (pNum * 10));
        randomnumber = (rand() % 5) + 1;

        char randomIntChar[2];
        sprintf(randomIntChar, "%d", randomnumber);

        sleep(randomnumber);

        // building the string
        strftime(cTestString, sizeof(cTestString), "%T", timeptr); // formats string in HH:MM:SS
        strcat(cTestString, " Queue ");
        strcat(cTestString, randomIntChar);
        strcat(cTestString, " File modified by process number ");
        strcat(cTestString, mypid);

        fprintf(outputFile, "%s\n", cTestString);
        fprintf(cTestStream, "%s\n", cTestString);

        randomnumber = (rand() % 5) + 1;
        sleep(randomnumber);

        // exit from critical section
        sb.sem_op = 1; /* free resource */
        if (semop(semid, &sb, 1) == -1)
        {
            perror("semop");
            exit(1);
        }
    }

    fclose(outputFile);

    return 0;
}