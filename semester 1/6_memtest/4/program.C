#include "stdlib.h"
#include "stdio.h"
#include "time.h"
#include "pthread.h"


const size_t threadsCount = 8;
const size_t matrixSize = 2048;

const size_t blockSize = 8;
const size_t chainSize = matrixSize / blockSize;


size_t stdA[matrixSize][matrixSize];
size_t stdB[matrixSize][matrixSize];
size_t stdResult[matrixSize][matrixSize];

size_t blockA[chainSize][chainSize][blockSize][blockSize];
size_t blockB[chainSize][chainSize][blockSize][blockSize];
size_t blockResult[chainSize][chainSize][blockSize][blockSize];


void* StandartMultiple(void *arg)
{
    const size_t threadNum = *reinterpret_cast<size_t*>(arg);
    for (size_t i = threadNum; i < matrixSize; i += threadsCount)
    {
        for (size_t j = 0; j < matrixSize; ++j)
        {
            for (size_t k = 0; k < matrixSize; ++k)
            {
                stdResult[i][j] += stdA[i][k] * stdB[k][j];
            }
        }
    }
}


void* BlockMultiple(void *arg)
{
    const size_t threadNum = *reinterpret_cast<size_t*>(arg);

    for (size_t i = threadNum; i < matrixSize; i += threadsCount)
    {
        const size_t i1 = i / blockSize;
        const size_t i2 = i % blockSize;

        for (size_t j1 = 0; j1 < chainSize; ++j1)
        {
            for (size_t j2 = 0; j2 < blockSize; ++j2)
            {
                for (size_t k1 = 0; k1 < chainSize; ++k1)
                {
                    for (size_t k2 = 0; k2 < blockSize; ++k2)
                    {
                        blockResult[i1][j1][i2][j2] += blockA[i1][k1][i2][k2] * blockB[k1][j1][k2][j2];
                    }
                }
            }
        }
    }
}


void doTest(void *(*action)(void*))
{
    pthread_t threads[threadsCount];
    for (size_t i = 0; i < threadsCount; ++i)
        pthread_create(&threads[i], NULL, action, new size_t(i));

    for (size_t i = 0; i < threadsCount; ++i)
        pthread_join(threads[i], NULL);
}


static inline double getTime()
{
    return (double)clock() / CLOCKS_PER_SEC;
}


int main()
{
    double startTime = getTime();
    doTest(StandartMultiple);
    double endTime = getTime();
    printf("Standard: %0.2f\n", endTime - startTime);

    startTime = getTime();
    doTest(BlockMultiple);
    endTime = getTime();
    printf("Block: %0.2f\n", endTime - startTime);
    
    /*
    Standard: 520.20
    Block: 392.02
    */

    return EXIT_SUCCESS;
}




