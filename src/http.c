#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include "http.h"
#include "files.h"
#include "mime.h"


static const char *not_foundr = "HTTP/1.1 404 Not Found\r\n"
                         	"Content-Length: 13\r\n"
				"Connection: close\r\n"
                         	"\r\n"
                         	"404 Not Found";

static const char *forbiddenr = "HTTP/1.1 403 Forbidden\r\n"
                         	"Content-Length: 13\r\n"
				"Connection: close\r\n"
                         	"\r\n"
                         	"403 Forbidden";

static const char *ok_response ="HTTP/1.1 200 OK\r\n"
                         	"Content-Type: %s\r\n"
                         	"Content-Length: %ld\r\n"
				"Connection: close\r\n"
                         	"\r\n";

static const char *bad_request="HTTP/1.1 400 Bad Request\r\n"
                             	"Content-Length: 15\r\n"
				"Connection: close\r\n"
                             	"\r\n"
                             	"400 Bad Request";

static const char *method_not_allowed = "HTTP/1.1 405 Method Not Allowed\r\n"
                             		"Content-Length: 22\r\n"
					"Connection: close\r\n"
                             		"\r\n"
                             		"405 Method Not Allowed";

static const char *Error = "HTTP/1.1 500 Internal Server Error\r\n"
                           "Content-Length: 25\r\n"
			   "Connection: close\r\n"
                           "\r\n"
                           "500 Internal Server Error";

int parse(const char *buffer, char *path, int sizepath){
	char buffer_copy[256];

	strncpy(buffer_copy, buffer, sizeof(buffer_copy) - 1);
   	buffer_copy[sizeof(buffer_copy) - 1] = '\0';

	char *p = buffer_copy;
        char *method = strsep(&p," ");
	char *path_receive = strsep(&p, " ");
        char *protocol = strsep(&p, " \r\n");

	if (p!=NULL && *method && *path_receive && *protocol){
            		 if (strcmp(method, "GET") != 0) {
                         printf("Método incorrecto %s\n", method);
			return 405;
                    	}
                 	if (strcmp(protocol, "HTTP/1.1") != 0) {
                          printf("Protocolo incorrecto %s\n", protocol);
				return 400;}
                    	 int host=0;
			 char *line;
				while ((line = strsep(&p, "\r\n")) != NULL) {
                                if (strlen(line) > 0) {
                                        if (strncasecmp(line, "Host:", 5) == 0) {
                                                host=1;
                                                break;}
                                        }
                                }
			if(host){
			 strncpy(path, path_receive, sizepath);
                          printf("Método: %s\n", method);
                          printf("Ruta:  %s\n", path);
                          printf("Protocol %s\n", protocol);
			  printf("Host Validado");
				return 200;
			}else{
				return 400;}
		}else{
			return 400;
		}
}


void response_HTTP(int current_fd, const char *buffer){
		char path[PATH_MAX];
		int parse_result = parse(buffer, path, sizeof(path));
		if(parse_result == 405){
                       printf("Metodo mal implementado");
                        if(write(current_fd, method_not_allowed, strlen(method_not_allowed))<0){
                                perror("error al escribir");
                                        }
                       return;}

		if(parse_result== 400){
			printf("Parse mal implementado");
			if(write(current_fd, bad_request, strlen(bad_request)) < 0){
				perror("Error al escribir");
				}
				return;}

		char verifiedclient_path[PATH_MAX];
                struct stat st;
		int response=transversal_file(path,verifiedclient_path);

		switch(response){
		case 500 :
                           printf("Error en el directorio www\n");
			if(write(current_fd, Error, strlen(Error)) <0){
				perror("error al escribir");
                           break;}
				break;
                case 404:
                         printf("error en la ruta no existe\n\n");
                         if(write(current_fd, not_foundr, strlen(not_foundr)) < 0){
				perror("error al escribir");
                           	break;
				}
				break;

                case 403:
                        printf("error la ruta path transversal (Error Forbidden)\n");
                        if(write(current_fd, forbiddenr, strlen(forbiddenr)) < 0){
                         	perror("error al escribir");
				break;
				}
				break;

		case  200:
                        int file_fd =  file_open(verifiedclient_path, &st);
                        if (file_fd==500){
					if(write(current_fd, Error, strlen(Error)) < 0 ){
					 perror("Error al escribir");
						break;}
						}
                      	const char *mime_type = mime(verifiedclient_path);

			char header[512];
			snprintf(header, sizeof(header), ok_response, mime_type, st.st_size);

			int w=write(current_fd, header, strlen(header));
			if (w <= 0) {
					perror("error");
            				break;}

                        char file_buffer[1024];
                        ssize_t bytes_read_file;

                        while ((bytes_read_file = read(file_fd, file_buffer, sizeof(file_buffer))) > 0) {
				if (bytes_read_file <0){
						perror("error");
						break;}
                            ssize_t bytes_written = write(current_fd, file_buffer, bytes_read_file);
                            if (bytes_written < 0) {
                                     perror("error al escribir");
                                     break;
                                     }
                          }
                        close(file_fd);
			break;
		}
}
