CC = gcc
CFLAGS = -Wall

all: menu_handler controller motor

menu_handler: menu_handler.c
	$(CC) $(CFLAGS) menu_handler.c -o menu_handler

run_menu_handler: menu_handler
	./menu_handler

controller: controller.c
	$(CC) $(CFLAGS) controller.c -o controller

run_controller: controller
	./controller

motor: motor.c
	$(CC) $(CFLAGS) motor.c -o motor

run_motor: motor
	./motor

clean:
	rm -f menu_handler controller motor controller_motor_pipe menu_pipe *.o

.PHONY: all clean