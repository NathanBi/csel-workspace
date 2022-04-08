#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>

int main()
{
    //Open /dev/mem
    int pf = open("/dev/mem", O_RDWR);

    //Test if the file is opened
    if(pf > 0)
    {
        //Mapping the registers
        int* map = mmap(NULL,4096,PROT_READ,MAP_SHARED,pf,0x01c14000);

        //Test if mapping failed
        if(map == MAP_FAILED)
        {
            printf("Error mapping");
        }
        else
        {
            unsigned int id[4];
            int offset = 0x01c14200 - 0x01c14000;

            //Read the registers for the chip-ID
            id[0] = *(map + (offset) / sizeof(int));
            id[1] = *(map + (offset + 0x04) / sizeof(int));
            id[2] = *(map + (offset + 0x08) / sizeof(int));
            id[3] = *(map + (offset + 0x0c) / sizeof(int));

            printf("Chip-ID : %08x'%08x'%08x'%08x\n", id[0], id[1], id[2], id[3]);
        
            //Free the mapping
            munmap(map,0x80);
        }

        //Close /dev/mem
        close(pf);

    }
    else
    {
        printf("Error open file");
    }
    

    return 0;
}