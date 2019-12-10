#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <cmath>

using namespace std;

/// Threads Objects
typedef struct counterObject
{
    int id;
} counterObject;
typedef struct monitorObject
{
} monitorObject;
typedef struct collectorObject
{
} collectorObject;

/// Threads Handlers
void* counterHandler(void *ptr);
void* monitorHandler(void *ptr);
void* collectorHandler(void *ptr);

/// Global variables
bool running;
int COUNTER;
int THREADS_COUNT, BUFFER_SIZE, TIME_INTERVAL;
sem_t SEM_COUNT;
sem_t SEM_FULL;
sem_t SEM_EMPTY;
sem_t SEM_BUFFER;
sem_t SEM_BATCH;
pthread_t *MCOUNTERS;
pthread_t MONITOR_THREAD;
pthread_t COLLECTOR_THREAD;
counterObject *OBJ_MCOUNTERS;
monitorObject OBJ_MONITOR;
collectorObject OBJ_COLLECTOR;

/// Functions Prototypes
void initialize();
void dispatchThreads();
void readInput();
int generateRandomInt(int low, int high);
void milli_sec_sleep(long period);

int main()
{
    initialize();
    dispatchThreads();
    return 0;
}

void* counterHandler(void *ptr)
{
    counterObject ob = *((counterObject *) ptr);
//    while(running)
//    {
        milli_sec_sleep(generateRandomInt(1000,4000)); //1 to 50 milliseconds
        printf("Counter thread %2d: received a message\n", (ob).id);
        int res = sem_trywait(&SEM_COUNT);
        if(res == 0)
        {
            COUNTER++;
            printf("Counter thread %2d: now adding to counter, counter value=%d\n", (ob).id, COUNTER);
            sem_post(&SEM_COUNT);
        }
        else
        {
            printf("Counter thread %2d: waiting to write!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", (ob).id);
            sem_wait(&SEM_COUNT);
        }
//    }
    return NULL;
}

void* monitorHandler(void *ptr)
{
    return NULL;
}

void initialize()
{
    //initialize counters and flags
    running = true;
    COUNTER = 0;
    readInput();
    //initialize semaphores
    sem_init(&SEM_COUNT, 0, 1);
    sem_init(&SEM_FULL, 0, BUFFER_SIZE);
    sem_init(&SEM_EMPTY, 0, 0);
    sem_init(&SEM_BUFFER, 0, 1);
    sem_init(&SEM_BATCH, 0, 1);
    //initialize counter threads and there objects
    MCOUNTERS = (pthread_t*) malloc(THREADS_COUNT * sizeof(pthread_t));
    pthread_t newThreads[THREADS_COUNT+2];
    OBJ_MCOUNTERS = (counterObject*) malloc(THREADS_COUNT * sizeof(counterObject));
    counterObject newObj[THREADS_COUNT];
    int i;
    for(i = 0 ; i < THREADS_COUNT ; i++)
    {
        MCOUNTERS[i] = newThreads[i];
        newObj[i].id = i + 1;
        OBJ_MCOUNTERS[i] = newObj[i];
    }
    //initialize monitor thread and its object
    MONITOR_THREAD = newThreads[i];
    monitorObject m;
    OBJ_MONITOR = m;
    //initialize collector thread and its object
    i++;
    COLLECTOR_THREAD = newThreads[i];
    collectorObject c;
    OBJ_COLLECTOR = c;
}

void dispatchThreads()
{
    int i, j, c, limit;
    int batch_size = 300;
    int batches = ceil(THREADS_COUNT / batch_size);
    for(c = 0 ; c <= batches ; c++)
    {
        limit = (c + 1) * batch_size;
        i = limit - batch_size;
        j = limit - batch_size;
        if(limit > THREADS_COUNT)
        {
            limit = THREADS_COUNT;
            i = limit - (THREADS_COUNT % batch_size);
            j = limit - (THREADS_COUNT % batch_size);
        }
        sem_wait(&SEM_BATCH);
        for( ; i < limit ; i++)
        {
            pthread_create(&MCOUNTERS[i], NULL, counterHandler, (void*) &OBJ_MCOUNTERS[i]);
        }
        for( ; j < limit ; j++)
        {
            pthread_join(MCOUNTERS[j], NULL);
        }
        sem_post(&SEM_BATCH);
    }

}

void readInput()
{
    printf("Reading input...\n");
    FILE *f = fopen("input.txt", "r");
    fscanf(f, "%d\n", &THREADS_COUNT);
    fscanf(f, "%d\n", &BUFFER_SIZE);
    fscanf(f, "%d", &TIME_INTERVAL);
    fclose(f);
    printf("Number of thread: %d thread\nBuffer size: %d entry\nMonitor check interval: %d ms\n\nReading completed!\n\n", THREADS_COUNT, BUFFER_SIZE, TIME_INTERVAL);
}

int generateRandomInt(int low, int high)
{
    int range=(high-low)+1;
    return range * (rand() / (RAND_MAX + 1.0));
}

void milli_sec_sleep(long period)
{
    struct timespec t = {0};
    t.tv_sec = 0;
    t.tv_nsec = period * 1000000L;
    nanosleep(&t, (struct timespec *)NULL);
}
