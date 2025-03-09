#include <stdio.h>  
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char *argv[]) {

    int file_descriptor, returnValue = 0, receivedArray[3][3], i, j;

    // initializes the receivedArray with zeros
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            receivedArray[i][j] = 0;
        }
    }

    while(1) {
        // reads the input from "controller_motor_pipe"
        file_descriptor = open("controller_motor_pipe", O_RDONLY);
        returnValue = read(file_descriptor, receivedArray, 9*sizeof(int));
        if(returnValue == -1) {
            printf("Error reading the pipe\n");
            return 1;
        }

        for(i=0; i<3; i++) {
            for(j=0; j<3; j++) {
                printf("%d ", receivedArray[i][j]);
            }
            printf("\n");
            sleep(1);
        }

    }

    return(0);
}