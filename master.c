/*
Jacob Drew
4760 Project 3
Master.c

*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>

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

pid_t *children;
int n;
// int shmid;
// struct shmseg *shmp;

int main(int argc, char *argv[])
{

    // declare variables
    int opt;
    int ss;
    ss = 100;

    // getopt
    // process command line args
    while ((opt = getopt(argc, argv, ":t:h")) != -1)
    {
        switch (opt)
        {
        case 'h':
            printf("Help:\n");
            printf("How to run:\n");
            printf("master [-t ss n] [-h]\n");
            printf("ss the maximum time in seconds (default 100 seconds) after which the process should terminate itself if not completed\n");
            printf("n number of slave processes to execute\n");
            printf("If n is over 20 it will be set to 20 for safety!\n");
            // if -h is the only arg exit program
            if (argc == 2)
            {
                exit(0);
            }
            break;
        case 't':
            ss = atoi(optarg);
            n = atoi(argv[3]);
            printf("SS: %d\n", ss);
            if (n > 18)
            {
                n = 18;
                printf("Number of processes set to 20 for safety\n");
                // printf("N: %d\n", n);
            }
            break;
        case ':':
            printf("option needs a value\n");
            break;
        case '?':
            printf("unknown option: %c\n", optopt);
            break;
        }
    }

    // get key
    if ((key = ftok("master.c", 'J')) == -1)
    {
        perror("ftok");
        exit(1);
    }

    /* create a semaphore set with 1 semaphore: */
    // also creates shared memory segment
    if ((semid = semget(key, 1, 0666 | IPC_CREAT)) == -1)
    {
        perror("semget");
        exit(1);
    }

    /* initialize semaphore #0 to 1: */ mySemaphore.val = 1;
    if (semctl(semid, 0, SETVAL, mySemaphore) == -1)
    {
        perror("semctl");
        exit(1);
    }

    /* grab the semaphore set created by seminit.c: */
    if ((semid = semget(key, 1, 0)) == -1)
    {
        perror("semget");
        exit(1);
    }

    // catch sigs
    // catch_sigint();
    // catch_sigalrm();
    alarm(ss); // ss from command args

    // initializing pids
    if ((children = (pid_t *)(malloc(n * sizeof(pid_t)))) == NULL)
    {
        errno = ENOMEM;
        perror("children malloc");
        exit(1);
    }
    pid_t pid;
    int i;
    for (i = 0; i < n; i++)
    {
        // fork and exec one process
        pid = fork();
        if (pid == -1)
        {
            // pid == -1 means error occured
            printf("can't fork, error occured\n");
            perror("fork failed");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        {
            char *childNum = malloc(6);
            sprintf(childNum, "%d", i);

            children[i] = pid;
            char *args[] = {"./slave", (char *)childNum, "8", (char *)0};
            execvp("./slave", args);
            perror("execvp");
            exit(0);
        }
    }

    // waiting for all child processes to finish
    for (i = 0; i < n; i++)
    {
        int status;
        waitpid(children[i], &status, 0);
    }

    free(children);

    /* remove it: */
    if (semctl(semid, 0, IPC_RMID, mySemaphore) == -1)
    {
        perror("semctl");
        exit(1);
    }

    return 0;
}