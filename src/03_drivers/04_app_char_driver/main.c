#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static const char* simple_string = "This is the text for the device!";


int main(int argc, char* argv[])
{
    int file;
    char out_buf[100];

    // Cannot execute if no parameters passed
    if(argc <= 1)
    {
        printf("No parameters provided, try again with parameters\n");
        return 0;
    }

    printf("Simple application using the drivers\n");

    // Write in node file passed as a parameter
    file = open(argv[1], O_WRONLY);
    write(file, simple_string, strlen(simple_string));
    close(file);

    // READ
    file = open(argv[1], O_RDONLY);
    read(file, out_buf, strlen(simple_string));
    printf("Read out_buf %s\n", out_buf);
    close(file);

    return 0;
}