#include "stdlib.h"
#include "stdio.h"
#include "time.h"
#include "pthread.h"


const size_t testsCount = 50000;
const size_t threadsCount = 100;

volatile int counters[threadsCount];
volatile int splitted_counters[threadsCount][128];


void* threadAction(void *arg)
{
    const size_t threadNum = *reinterpret_cast<size_t*>(arg);
    for (size_t i = 0; i < testsCount; ++i)
        ++counters[threadNum];
}


void* splittedThreadAction(void *arg)
{
    const size_t threadNum = *reinterpret_cast<size_t*>(arg);
    for (size_t i = 0; i < testsCount; ++i)
        ++splitted_counters[threadNum][0];
}



double timeTest(void *(*action)(void*))
{
    const double startTime = (double)clock()/CLOCKS_PER_SEC;

    pthread_t threads[threadsCount];
    for (size_t i = 0; i < threadsCount; ++i)
        pthread_create(&threads[i], NULL, action, new size_t(i));

    for (size_t i = 0; i < threadsCount; ++i)
        pthread_join(threads[i], NULL);

    const double endTime = (double)clock()/CLOCKS_PER_SEC;
    return endTime - startTime;
}


int main(void)
{
    srand((int)time(0));

    printf("Tightly: %0.2f\n", timeTest(threadAction));
    printf("Splitted: %0.2f\n", timeTest(splittedThreadAction));
    
    /*
    Tightly: 0.19
    Splitted: 0.08
    */

    return EXIT_SUCCESS;    
}


