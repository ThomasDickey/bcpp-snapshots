
#ifndef DIRENT_H
#define DIRENT_H

#include <direct.h>

struct dirent {
	char *d_name;
};

struct _dirdesc {
	HANDLE hFindFile;
	WIN32_FIND_DATA ffd;
	int first;
	struct dirent de;
};

#define DIRENT struct dirent
typedef struct _dirdesc DIR;

#define opendir  my_opendir
#define readdir  my_readdir
#define closedir my_closedir

extern DIR * opendir ( const char *path );
extern DIRENT *readdir ( DIR *dp );
extern int closedir ( DIR *dp );

#endif /* dirent.h */
