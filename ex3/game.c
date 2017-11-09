#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

static void GPIO_interrupt_handler(int unused)
{
	// This is run whenever we recieve a GPIO interrupt
	printf("DEBUG: Interrupt recieved by game");
}

int main(int argc, char *argv[])
{
	// Import gamepad-driver
	//system("modprobe gamepad-driver");
	char command[50];
	strcpy(command, "modprobe driver-gamepad");
	system(command);
	// Save file descriptor (ID) to "test" if no error
	int test = open("/dev/GPIO", O_RDWR, (mode_t)0600); //Sjekk om det er flaggene vi trenger
	// Set SIGIO signal to trigger handler
	struct sigaction GPIO_action = {}; 
	GPIO_action.sa_handler = &GPIO_interrupt_handler; 
	sigaction(SIGUSR1, &GPIO_action, NULL);
	
	int testBuff;
	testBuff = 0;
	printf("Hello World, I'm game!\n");
	while (1) {
		printf("Test: %d \n", test);
		read(test, &testBuff, sizeof(int)); // Read from file descriptor "test", write to testBuff
		printf("Read: %d \n", testBuff);
		usleep(1000000);
	}
	exit(EXIT_SUCCESS);
}


