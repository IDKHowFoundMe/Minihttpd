#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "http.h"

#define MAX_EVENTS 10
#define QLEN              32

int passivesock(const  char *service, const char *transport, int qlen);

int server(int puerto) {
    signal(SIGPIPE, SIG_IGN);
    const char *transport = "tcp";
    char service[6];
    snprintf(service, sizeof(service), "%d", puerto);
    int server_fd = passivesock(service,transport, QLEN);

    printf("FD Server (fd: %d)\n", server_fd);

    int epoll_fd = epoll_create1(0);
	if(epoll_fd == -1){
		perror("Error epoll_create1");
               	exit(EXIT_FAILURE);}

	printf("FD Epol %d\n", epoll_fd);


    struct epoll_event ev = {
        .events = EPOLLIN,
        .data.fd = server_fd
    };

    if((epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev)) == -1){
		perror("Error epoll_ctl");
        	exit(EXIT_FAILURE);};


    struct epoll_event events[MAX_EVENTS];

    while (1) {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
	if(nfds == -1){
			if (errno == EINTR) continue;
			perror("Error en el epolwait");
			exit(EXIT_FAILURE);
			}
        for (int i = 0; i < nfds; i++) {
            int current_fd = events[i].data.fd;

		if(current_fd == server_fd) {
                	int client_fd = accept4(server_fd, NULL, NULL,SOCK_NONBLOCK);
			if(client_fd<0){
				perror("Error en el accept");
    				continue;
			}
                		ev.events = EPOLLIN;
                		ev.data.fd = client_fd;
                		if((epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev))<0){
						perror("Error en el ctl cliente no agregado");
							close(client_fd);
							continue;
				}else{
					printf("Nuevo cliente (fd: %d)\n", client_fd);
            			}
		}
            	else {
                char buffer[256];
		ssize_t bytes_read = read(current_fd, buffer, sizeof(buffer) - 1);
			if (bytes_read > 0) {
    				buffer[bytes_read] = '\0';
    				printf("Mensaje recibido (fd: %d):\n%s\n", current_fd, buffer);

				response_HTTP(current_fd, buffer);

				epoll_ctl(epoll_fd, EPOLL_CTL_DEL, current_fd, NULL);
                		close(current_fd);
                		printf("Cliente respondido y desconectado (fd: %d)\n\n", current_fd);
			}else if(bytes_read ==0){
				epoll_ctl(epoll_fd, EPOLL_CTL_DEL, current_fd, NULL);
                                close(current_fd);
                                printf("Cliente desconectado correctamente sin envio de datos(fd: %d)\n\n", current_fd);
			} else {
				if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        		continue;
                    		}else{
				perror("Error en read");
                	    	epoll_ctl(epoll_fd, EPOLL_CTL_DEL, current_fd, NULL);
                    		close(current_fd);
				printf("Cliente desconectado con error (fd: %d)\n", current_fd);}
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
		perror("Error en getaddrinfo");
		exit(EXIT_FAILURE);
			}
	s = socket(rp->ai_family, rp->ai_socktype | SOCK_NONBLOCK, rp->ai_protocol);
	if (s < 0 ){
		perror("Error en fd del socket");
		exit(EXIT_FAILURE);
	}

	int opt = 1;
        setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	if(bind(s, rp->ai_addr, rp->ai_addrlen) < 0){
		perror("Error en el bind");
		exit(EXIT_FAILURE);
		}

	freeaddrinfo(rp);

	if(listen(s, qlen)<0){
		perror("Error en el listen");
		exit(EXIT_FAILURE);
		}
	return s;
}
