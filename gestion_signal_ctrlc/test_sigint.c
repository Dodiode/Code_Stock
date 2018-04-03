#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
volatile sig_atomic_t stop = 0;

static void my_function(int signo){
	stop = 1;
}

int main(void)
{
	signal(SIGINT, my_function); 
	
	while(1){
		if (stop){
			printf("Signal caught !\n");
		}
		stop = 0;
	}
    return 0;
}
