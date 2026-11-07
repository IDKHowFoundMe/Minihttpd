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

            if (current_fd == server_fd) {
                int client_fd = accept(server_fd, NULL, NULL);

                ev.events = EPOLLIN;
                ev.data.fd = client_fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);

                printf("Nuevo cliente (fd: %d)\n", client_fd);
            }

            else {
                char buffer[256];
		ssize_t bytes_read = read(current_fd, buffer, sizeof(buffer) - 1);

		if (bytes_read > 0) {
    			buffer[bytes_read] = '\0';
    			printf("Mensaje recibido (fd: %d):\n%s\n", current_fd, buffer);

	                char *p = buffer;
        	        char *method = strsep(&p," ");
			char *path = strsep(&p, " ");
			char *protocol = strsep(&p, " \r\n");

			if (p!=NULL && *method && *path && *protocol){

				if (strcmp(method, "GET") != 0) {
					printf("Método incorrecto %s\n", method);
                            		}
                            	else if (strcmp(protocol, "HTTP/1.1") != 0) {
                                	printf("Protocolo incorrecto %s\n", protocol);
                            	}else{
					printf("Método: %s\n", method);
        	                	printf("Ruta:  %s\n", path);
					printf("Protocol %s\n", protocol);

					char filepath[256];
					if(strcmp(path, "/") == 0) {
    						snprintf(filepath, sizeof(filepath), "www/index.html");
					}else{

						snprintf(filepath, sizeof(filepath), "www%s", path);
						}


					struct stat st;
					if (stat(filepath, &st) == -1) {

    						const char *not_found = "HTTP/1.1 404 Not Found\r\n"
									"Content-Length: 13\r\n"
									"\r\n"
									"404 Not Found";
    						write(current_fd, not_found, strlen(not_found));
					}else{

    						int file_fd = open(filepath, O_RDONLY);
    						if (file_fd == -1) {
							printf("error en el open");
						}else{

						const char *response_head =
        					                "HTTP/1.1 200 OK\r\n"
                        					"Content-Type: text/html; charset=UTF-8\r\n"
                					        "Content-Length: %ld\r\n"
                        					"\r\n";
						dprintf(current_fd, response_head, st.st_size);

						char file_buffer[1024];
        					ssize_t bytes_read_file;

						while ((bytes_read_file = read(file_fd, file_buffer, sizeof(file_buffer))) > 0) {
						            ssize_t bytes_written = write(current_fd, file_buffer, bytes_read_file);
								if (bytes_written == -1) {
    									printf("Error al escribir");
    									break;
									}
        						}
							close(file_fd);
						}
						}
					}
					} else {
                        	printf("Error\n");
                    	}

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
	int s =-1;
	int type = SOCK_STREAM;
	int status;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = type;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	status = getaddrinfo(NULL,service,&hints,&rp);

	if (status != 0){
		printf("error en getaddrinfo \n");
			}
	s = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
	if (s < 0 ){
		printf("error en fd del socket\n");
	}

	int opt = 1;
        setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	bind(s, rp->ai_addr, rp->ai_addrlen);
	freeaddrinfo(rp);
	listen(s, qlen);


	return s;

}
