#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// static const char* simple_string = "1";


int main(int argc, char* argv[])
{
    if(argc <= 1)
    {
        printf("No parameters provided, try again with parameters\n");
        return 0;
    }

    printf("Simple application using the drivers\n");
    printf("argv[1] = %s\n", argv[1]);
    int file;
    // if(argc != 0)
    // {
    // WRITE
    // file = open(argv[1], O_RDWR);
    // write(file, argv[1], strlen(argv[1]));
    // // write(file, simple_string, strlen(simple_string));
    //     // fwrite(simple_string, sizeof(simple_string), sizeof(simple_string), file_ptr);
    // close(file);

    file = open(argv[1], O_WRONLY);
    printf("file status %d\n", file);
    write(file, /*simple_string*/ "b", 1);
    close(file);

    // READ
    file = open(argv[1], O_RDONLY);
    while(1)
    {
        char out_buf[100];
        ssize_t size_left = read(file, out_buf, sizeof(out_buf)-1);
        printf("Read out_buf %s size_left %d\n", out_buf, (int) size_left);
        if(size_left <= 0)
        {
            printf("break\n");
            break;
        }
        out_buf[sizeof(out_buf) - 1] = 0;
    }
    close(file);
        // fclose(file_ptr);
    return 0;
}