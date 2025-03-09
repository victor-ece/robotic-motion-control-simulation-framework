#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <signal.h>

int sLoopArray[3][3], stepInLoopControl = 0, file_descriptor2, movesArray[3][3] = {{0, -25, -40}, {-50, -30, -40}, {-120, 0, -40}};
char last_valid_input = ' ';

void find_pid(const char *process_name, int *processID);
void end_programs();
void add_arrays(int A[3][3], int B[3][3]);
void handle_errors(int sig);

int main (int argc, char *argv[]) {
    int i, j, file_descriptor1, mArray[3][3] = {{0, 25, 40}, {50, 30, 40}, {120, 0, 40}},
    sArray[3][3] = {{25, 12, 30}, {-45, 214, 30}, {25, -214, 30}}, stepLoop = 0,
    returnValue = 0, *pid = NULL;
    char input;

    pid = (int *) malloc(sizeof(int));

    // SIGUSR1 simulates high temperature 
    signal(SIGUSR1, handle_errors);
    // SIGUSR2 simulates dangerous angle
    signal(SIGUSR2, handle_errors);

    // print the pid of the controller process
    *pid = getpid();
    printf("Controller PID: %d\n", *pid);

    // creates a named pipe "controller_motor_pipe" to communicate with motor.c
    if (mkfifo("controller_motor_pipe", 0666) == -1) {
        if (errno != EEXIST) {
            perror("Error creating the controller_motor pipe");
            return 1;
        }
    }

    // opens the controller_motor pipe for writing
    file_descriptor2 = open("controller_motor_pipe", O_WRONLY);
    if(file_descriptor2 == -1) {
        perror("Error opening the controller_motor pipe\n");
        return 1;
    }

    file_descriptor1 = open("menu_pipe", O_RDONLY | O_NONBLOCK);

    while(1) {
        input = ' '; // resets the input

        // reads the input from "menu_pipe"
        returnValue = read(file_descriptor1, &input, sizeof(char));
        if(returnValue == -1 && errno != EAGAIN && errno != EWOULDBLOCK) { // exclude errors due to non-blocking
            perror("Error reading form menu_pipe\n");
            return 1;
        }
        
        // in case the step-in-loop is active
        if (stepLoop == 1) {
            if (input == 'g') {
                // deactivates the step-in-loop and proceeds to the next iteration
                stepLoop = 0; 
                continue;
            } else {
                // in case we just recovered from a dangerous angle
                if (stepInLoopControl == 1) {
                    for(i=0; i<3; i++) {
                        for(j=0; j<3; j++) {
                            sLoopArray[i][j] = sArray[i][j];
                        }
                    }
                    stepInLoopControl = 0;
                } else { // increases the elements of sLoopArray by 10%
                    for(i=0; i<3; i++) {
                        for(j=0; j<3; j++) {
                            sLoopArray[i][j] = sLoopArray[i][j] + sLoopArray[i][j]*0.1;
                        }
                    }
                }

                // writes the sLoopArray to the controller_motor pipe
                returnValue = write(file_descriptor2, &sLoopArray, 9*sizeof(int));
                if(returnValue == -1) {
                    perror("Error writing to the controller_motor pipe\n");
                    return 1;
                }
                add_arrays(movesArray, sLoopArray);
                sleep(5);
                continue;
            }
        }

        switch (input)
        {
            case 'm':
                returnValue = write(file_descriptor2, mArray, 9*sizeof(int));
                if(returnValue == -1) {
                    perror("Error writing to the controller_motor pipe\n");
                    return 1;
                }
                add_arrays(movesArray, mArray);
                last_valid_input = 'm';
                break;
            case 's':
                returnValue = write(file_descriptor2, sArray, 9*sizeof(int));
                if(returnValue == -1) {
                    perror("Error writing to the controller_motor pipe\n");
                    return 1;
                }
                add_arrays(movesArray, sArray);
                last_valid_input = 's';
                break;
            case 'g':
                // initialising the sLoopArray
                for(i=0; i<3; i++) {
                    for(j=0; j<3; j++) {
                        sLoopArray[i][j] = sArray[i][j];
                    }
                }

                // writes the sLoopArray to the controller_motor pipe
                returnValue = write(file_descriptor2, &sLoopArray, 9*sizeof(int));
                if(returnValue == -1) {
                    perror("Error writing to the controller_motor pipe\n");
                    return 1;
                }
                add_arrays(movesArray, sLoopArray);
                last_valid_input = 'g';
                
                stepLoop = 1; // to indicate that the step-in-loop is active
                sleep(5);
                break;
            case 'q':
                end_programs();
                return(0);   
            default:
                break;
        }
    }
    return(0);
}

/*
 * Function to find the PID of a process
 * Parameters: the name of the process and a pointer to the PID
 * Returns: void
*/
void find_pid(const char *process_name, int *processID) {
    char command[256];
    char pid[10];
    FILE *filePointer;

    /* creates the command string to find the PID of the process
    'pgrep' returns the PID of the process that matches the exact process name */
    snprintf(command, sizeof(command), "pgrep -x %s", process_name); 

    // runs a shell command and establishes a pipe between the calling program and the executed command
    filePointer = popen(command, "r"); // 'r' for reading
    if (filePointer == NULL) {
        perror("popen failed");
        return;
    }
    
    // reads a line from the pipe and stores it in the pid variable
    while (fgets(pid, sizeof(pid), filePointer) != NULL);
    pclose(filePointer);

    *processID = atoi(pid);
}

/*
 * Handles the errors of high temperature and dangerous angle
*/
void handle_errors(int sig) {
    int returnValue = 0, sLoopArrayOpp[3][3], i, j;

    if(last_valid_input == 'g') {
        for(i=0; i<3; i++) {
            for(j=0; j<3; j++) {
                sLoopArrayOpp[i][j] = - movesArray[i][j];
            }
        }
        returnValue = write(file_descriptor2, sLoopArrayOpp, 9*sizeof(int));
        if(returnValue == -1) {
            perror("Error writing to the controller_motor pipe\n");
            exit(1);
        }
    }

    // in case it was a temperature error we close the system, otherwise we restart
    if(sig == SIGUSR1) {
        if(last_valid_input == 'g') {
            sleep(6); // to give time to the motor to write the last command
        } else {
            sleep(3);
        }
        end_programs();
        exit(0);
    }
    else {
        // the "movement" in the loop starts from the beginning in case of a dangerous angle
        stepInLoopControl = 1;
    }
}

// Adds array B to array A 
void add_arrays(int A[3][3], int B[3][3]) {
    int i, j;

    for(i=0; i<3; i++) {
        for(j=0; j<3; j++) {
            A[i][j] += B[i][j];
        }
    }
}

// ends the programs and deletes the named pipes
void end_programs() {
    int *pid = (int *) malloc(sizeof(int));

    find_pid("motor", pid);
    printf("Killing motor process with PID: %d\n", *pid);
    kill(*pid, SIGKILL);

    find_pid("menu_handler", pid);
    printf("Killing menu_handler process with PID: %d\n", *pid);
    kill(*pid, SIGKILL);

    free(pid);

    // deletes the named pipes
    if (unlink("controller_motor_pipe") == 0) {
        printf("controller_motor_pipe deleted successfully.\n");
    } else {
        perror("Error removing named pipe");
    }

    if (unlink("menu_pipe") == 0) {
        printf("menu_pipe deleted successfully.\n");
    } else {
        perror("Error removing named pipe");
    }
}