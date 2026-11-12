#include <stdio.h>
#include "server.h"

int server(char puerto;);
int main(void){
	int puerto = 8080;
    	printf("MiniHTTP Server\n");
    	if (server(puerto) < 0) {
        	fprintf(stderr, "Error en el servidor\n");
        	return 1;
    		}
    	return 0;
}
