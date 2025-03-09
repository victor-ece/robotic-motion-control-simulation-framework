#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

int main (int argc, char *argv[]) {
    char input;
    int file_descriptor, returnValue = 0;

    printf("[m] Getting Up\n");
    printf("[s] Side Step\n");
    printf("[g] Step-in-loop\n");
    printf("[q] Quit\n");

    // creates a named pipe "menu_pipe" with read/write permissions for all
    if (mkfifo("menu_pipe", 0666) == -1 && errno != EEXIST) {
        perror("Error creating the pipe");
        return 1;
    }

    file_descriptor = open("menu_pipe", O_WRONLY);
    if(file_descriptor == -1) {
        perror("Error opening the pipe\n");
        return 1;
    }

    do {
        scanf(" %c", &input);

        switch(input) {
            case 'm':
                printf("Getting Up\n");
                returnValue = write(file_descriptor, &input, sizeof(char));
                if(returnValue == -1) {
                    perror("Error writing to the pipe\n");
                    return 1;
                }
                break;
            case 's':
                printf("Side Step\n");                
                returnValue = write(file_descriptor, &input, sizeof(char));
                if(returnValue == -1) {
                    perror("Error writing to the pipe\n");
                    return 1;
                }
                break;
            case 'g':
                printf("Step-in-loop\n");
                returnValue = write(file_descriptor, &input, sizeof(char));
                if(returnValue == -1) {
                    perror("Error writing to the pipe\n");
                    return 1;
                }
                break;
            case 'q':
                printf("Quit\n");
                returnValue = write(file_descriptor, &input, sizeof(char));
                if(returnValue == -1) {
                    perror("Error writing to the pipe\n");
                    return 1;
                }
                break;
            default:
                printf("Invalid choice\n");
                break;
        }
    } while (1);
    
    return 0;
}