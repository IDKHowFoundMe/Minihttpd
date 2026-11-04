#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define MAX_EVENTS 6

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in address = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(PORT)
    };
    
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, SOMAXCONN);

    int epoll_fd = epoll_create1(0);

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

         
            if (current_fd == server_fd) {
                int client_fd = accept(server_fd, NULL, NULL);
                
         
                ev.events = EPOLLIN;
                ev.data.fd = client_fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
                
                printf("-> Nuevo cliente (fd: %d)\n", client_fd);
            } 
           
            else {
                char buffer[256];
                int bytes = read(current_fd, buffer, sizeof(buffer) - 1);

                if (bytes <= 0) {
                    printf("<- Cliente desconectado (fd: %d)\n", current_fd);
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, current_fd, NULL);
                    close(current_fd);
                } else {
                    buffer[bytes] = '\0';
                    printf("Mensaje recibido del cliente (fd: %d):\n%s\n", current_fd, buffer);
                   
                }
            }
        }
    }

    close(server_fd);
    close(epoll_fd);
    return 0;
}
