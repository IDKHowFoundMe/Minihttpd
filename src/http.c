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
                         "\r\n"
                         "404 Not Found";

static const char *forbiddenr = "HTTP/1.1 403 Forbidden\r\n"
                         "Content-Length: 13\r\n"
                         "\r\n"
                         "403 Forbidden";

static const char *ok_response ="HTTP/1.1 200 OK\r\n"
                         "Content-Type: %s\r\n"
                         "Content-Length: %ld\r\n"
                         "\r\n";

static const char *bad_request="HTTP/1.1 400 Bad Request\r\n"
                             	"Content-Length: 15\r\n"
                             	"\r\n"
                             	"400 Bad Request";

static const char *method_not_allowed = "HTTP/1.1 405 Method Not Allowed\r\n"
                             		"Content-Length: 22\r\n"
                             		"\r\n"
                             		"405 Method Not Allowed";

static const char *Error = "HTTP/1.1 500 Internal Server Error\r\n"
                             "Content-Length: 25\r\n"
                             "\r\n"
                             "500 Internal Server Error";

char *parse(const char *buffer){
	char buffer_copy[256];

	strncpy(buffer_copy, buffer, sizeof(buffer_copy) - 1);
   	buffer_copy[sizeof(buffer_copy) - 1] = '\0';

	char *p = buffer_copy;
        char *method = strsep(&p," ");
        char *errorparse = "400";
	char *path = strsep(&p, " ");
        char *protocol = strsep(&p, " \r\n");

	if (p!=NULL && *method && *path && *protocol){
            		 if (strcmp(method, "GET") != 0) {
                         printf("Método incorrecto %s\n", method);
			return NULL;
                    	}
                 	else if (strcmp(protocol, "HTTP/1.1") != 0) {
                          printf("Protocolo incorrecto %s\n", protocol);
				return NULL;
                    	}else{
                          printf("Método: %s\n", method);
                          printf("Ruta:  %s\n", path);
                          printf("Protocol %s\n", protocol);

				return path;
			}
	}
	return errorparse;
}


void response_HTTP(int current_fd, const char *buffer){

		char *path  = parse(buffer);

		if(path == NULL){
                       printf("Protocolo o metodo mal implementado");
                        if(write(current_fd, method_not_allowed, strlen(method_not_allowed))<0){
                                perror("error al escribir");
                                        }
                       return;}

		if(strcmp(path, "400") == 0){
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
                case 404:
                         printf("error en la ruta no existe\n\n");
                         if(write(current_fd, not_foundr, strlen(not_foundr)) < 0){
				perror("error al escribir");
                           	break;
				}

                case 403:
                        printf("error la ruta path transversal (Error Forbidden)\n");
                        if(write(current_fd, forbiddenr, strlen(forbiddenr)) < 0){
                         	perror("error al escribir");
				break;
				}

		case  200:
                        int file_fd =  file_open(verifiedclient_path, &st);
                        if (file_fd==500){
					if(write(current_fd, Error, strlen(Error)) < 0 ){
					 perror("Error al escribir");
						break;}
						}
                      	const char *mime_type = mime(verifiedclient_path);

                        dprintf(current_fd, ok_response, mime_type, st.st_size);

                        char file_buffer[1024];
                        ssize_t bytes_read_file;

                        while ((bytes_read_file = read(file_fd, file_buffer, sizeof(file_buffer))) > 0) {
                            ssize_t bytes_written = write(current_fd, file_buffer, bytes_read_file);
                            if (bytes_written == -1) {
                                     perror("error al escribir");
                                     break;
                                     }
                          }
                        close(file_fd);
			break;
		}
}
