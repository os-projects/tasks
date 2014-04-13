#include "stdlib.h"
#include "stdio.h"
#include "time.h"
#include "pthread.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
#include "vector"

using std::vector;
using std::cout;
using std::endl;



class Block
{
public:
    explicit Block(const size_t length)
          : data(new char[length])
          , length(length)
          {}
          
    void setRandomElementToOne()
    {
        data[rand() % length] = 1;
    }
    
    ~Block()
    {
        delete[] data;
    }

private:
    char *data;
    const size_t length;
};


static inline void doRandomAction(vector<Block*> &blocks)
{
    double f = drand48();
    if (f < 0.333 || !blocks.size())
    {
        size_t length = 256 * (1 + rand() % 4);
        blocks.push_back(new Block(length));
    }
    else if (f < 0.666)
    {
        size_t i = rand() % blocks.size();
        free(blocks[i]);
        blocks[i] = blocks.back();
        blocks.pop_back();
    }
    else
    {
        Block *randBlock = blocks[rand() % blocks.size()];
        randBlock->setRandomElementToOne();
    }
}


static inline void* doTest(void *)
{
    vector<Block*> blocks;
    const size_t repeats = 50000;
    for (size_t i = 0; i < repeats; ++i)
        doRandomAction(blocks);
}


static inline double timeAction(void (*action) ())
{
    const double startTime = (double)clock()/CLOCKS_PER_SEC;
    action();
    const double endTime = (double)clock()/CLOCKS_PER_SEC;
    return endTime - startTime;
}


const size_t threadsCount = 20;

void testThreads()
{
    pthread_t threads[threadsCount];
    for (size_t i = 0; i < threadsCount; ++i)
        pthread_create(&threads[i], NULL, doTest, NULL);

    for (size_t i = 0; i < threadsCount; ++i)
        pthread_join(threads[i], NULL);
}


void testProcesses()
{
    pid_t pids[threadsCount];
    for(size_t i = 0; i < threadsCount; ++i)
    {
        pids[i] = fork();
        if (pids[i] == 0)
        {
            doTest(NULL);
            exit(0);
        }
    }
    wait(NULL);
}


int main(void)
{
    srand((int)time(0));

    printf("Threads time: %0.2f\n", timeAction(testThreads));
    printf("Processes time: %0.2f\n", timeAction(testProcesses));
    
    /*
    Threads time: 7.41
    Processes time: 0.26
    */
    
    return EXIT_SUCCESS;    
}


