#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

int main()
{
    int amountBytesToAlloc = 0;
    void *alloc;
    for(amountBytesToAlloc = 0; amountBytesToAlloc < 50; amountBytesToAlloc++)
    {
        alloc = malloc(1024*1024);
        memset(alloc, 0, 1024*1024);
        printf("Allocated %d MB\n", amountBytesToAlloc+1);
        sleep(1);
    }
    free(alloc);
    return 0;
}