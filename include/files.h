#ifndef FILES_H
#define FILES_H
#include <sys/stat.h>


	#define internalServerError 500
	#define notFound  404
	#define forbidden 403
	#define fileOk 200


int transversal_file(const char *path, char *verifiedclient_path);
int  file_open(const char *verifiedclient_path, struct stat *st);


#endif
