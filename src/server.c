#include <sys/socket.h>
#include <stdio.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <linux/limits.h>
#include "mime.h"
#include "files.h"
#include "http.h"

#define MAX_EVENTS 6
#define QLEN              32



int passivesock(const  char *service, const char *transport, int qlen);


int main() {
	signal(SIGPIPE, SIG_IGN);
    const char *service = "8080";
    const char *transport = "tcp";

    int server_fd = passivesock(service,transport, QLEN);

    printf("FD Server: (fd: %d)\n", server_fd);

    int epoll_fd = epoll_create1(0);
	printf("FD Epol: %d\n", epoll_fd);


    struct epoll_event ev = {
        .events = EPOLLIN,
        .data.fd = server_fd
    };

    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev);


    struct epoll_event events[MAX_EVENTS];

    while (1) {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);

        for (int i = 0; i < nfds; i++) {
            int current_fd = events[i].data.fd;
		printf("currentfd %d\n",current_fd);

		if(current_fd == server_fd) {
                	int client_fd = accept4(server_fd, NULL, NULL,SOCK_NONBLOCK);
			if(client_fd<0){
				perror("error en el accept");
			}else{
                		ev.events = EPOLLIN;
                		ev.data.fd = client_fd;
                		epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);

                		printf("Nuevo cliente (fd: %d)\n", client_fd);
            		}
		}

            else {
		printf("Continuacion del cliente\n");
                char buffer[256];
		ssize_t bytes_read = read(current_fd, buffer, sizeof(buffer) - 1);

			if (bytes_read > 0) {
    				buffer[bytes_read] = '\0';
    				printf("Mensaje recibido (fd: %d):\n%s\n", current_fd, buffer);
				response_HTTP(current_fd, buffer);
				epoll_ctl(epoll_fd, EPOLL_CTL_DEL, current_fd, NULL);
                		close(current_fd);
                		printf("Cliente respondido y desconectado (fd: %d)\n\n", current_fd);
			} else {
        	            	printf("Cliente desconectado o error (fd: %d)\n", current_fd);
                	    	epoll_ctl(epoll_fd, EPOLL_CTL_DEL, current_fd, NULL);
                    		close(current_fd);
                }
            }
        }
    }

    close(server_fd);
    close(epoll_fd);
    return 0;
}

int passivesock(const char *service, const char *transport, int qlen){

	struct addrinfo hints;
	struct addrinfo  *rp;
	struct protoent *pe=getprotobyname(transport);
	int s =-1;
	int type = SOCK_STREAM;
	int status;


	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = type;
        hints.ai_protocol = pe->p_proto;
	hints.ai_flags = AI_PASSIVE;

	status = getaddrinfo(NULL,service,&hints,&rp);

	if (status != 0){
		perror("error en getaddrinfo \n");
		exit(EXIT_FAILURE);
			}
	s = socket(rp->ai_family, rp->ai_socktype | SOCK_NONBLOCK, rp->ai_protocol);
	if (s < 0 ){
		perror("error en fd del socket\n");
		exit(EXIT_FAILURE);
	}

	int opt = 1;
        setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	if(bind(s, rp->ai_addr, rp->ai_addrlen) < 0){
		perror("error en el bind");
		exit(EXIT_FAILURE);
		}

	freeaddrinfo(rp);

	if(listen(s, qlen)<0){
		perror("error en el listen");
		exit(EXIT_FAILURE);
		}


	return s;

}


