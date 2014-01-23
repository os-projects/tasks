#include "stdlib.h"
#include "stdio.h"
#include <time.h>

const size_t trials = 2000000;

static inline void test_memory(char *memory, const size_t *indices)
{
    for (size_t i = 0; i < trials; ++i)
        memory[indices[i]] = 1;
}


int main(void)
{
    srand((int)time(0));
    
    const size_t memSize = (size_t)6 * 1024 * 1024 * 1024 * 5 / 4;
    char *memory = (char *)malloc(memSize);
    if (!memory)
        printf("FAIL\n");
    
    size_t *serial_indices = calloc(trials, sizeof(size_t));
    size_t *random_indices = calloc(trials, sizeof(size_t));
    
    const size_t begin = rand();
    for (size_t i = 0; i < trials; ++i) 
    {
        serial_indices[i] = (begin + i) % memSize;
        random_indices[i] = rand() % memSize;
    }
    
    float startTime = (double)clock()/CLOCKS_PER_SEC;
    test_memory(memory, serial_indices);
    float endTime = (double)clock()/CLOCKS_PER_SEC;
    printf("Serial: %0.2f\n", endTime - startTime);
    
    startTime = (float)clock()/CLOCKS_PER_SEC;
    test_memory(memory, random_indices);
    endTime = (float)clock()/CLOCKS_PER_SEC;
    printf("Random: %0.2f\n", endTime - startTime);
    
    /*
    Serial: 0.03
    Random: 2.08
    */
    
    free(memory);
    free(serial_indices);
    free(random_indices);

    return EXIT_SUCCESS;
}














