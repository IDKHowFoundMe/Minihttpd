#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include "files.h"

int transversal_file(const char *path, char *verifiedclient_path){
		char filepath[256];
			if(strcmp(path, "/") == 0) {
				snprintf(filepath, sizeof(filepath), "www/index.html");
                        }else{
				snprintf(filepath, sizeof(filepath), "www%s", path);
                        }

                char rootdir_path[PATH_MAX];

		if (realpath("www",rootdir_path)==NULL) {
			return error;
		}

		if(realpath(filepath, verifiedclient_path)==NULL){
			return notFound;
		}else if (strncmp(verifiedclient_path, rootdir_path, strlen(rootdir_path)) != 0) {
			return forbidden;
		}else{
			return fileOk;
			}

}


int  file_open(const char *verifiedclient_path, struct stat *st){

		stat(verifiedclient_path, st);
		int file_fd = open(verifiedclient_path, O_RDONLY);
                if (file_fd == -1) {
                	perror("error en el open\n");
			return error;
                }else{
			return file_fd;
		}
	}
