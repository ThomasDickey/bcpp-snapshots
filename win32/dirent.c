#include <windows.h>
#include <stdio.h>

#include <dirent.h>

#define is_slashc(c) ((c) == '/' || (c) == '\\')
#define WILDCARD "*"

DIR *
opendir(const char * fname)
{
	char buf[MAX_PATH];
	DIR *od;

	(void)strcpy(buf, fname);

	if (!strcmp(buf, ".")) /* if its just a '.', replace with '*.*' */
		(void)strcpy(buf, WILDCARD);
	else {
		/* If the name ends with a slash, append '*.*' otherwise '\*.*' */
		if (!is_slashc(buf[strlen(buf)-1]))
			(void)strcat(buf, "\\");
		(void)strcat(buf, WILDCARD);
	}

	/* allocate the structure to maintain currency */
	if ((od = (DIR *)malloc(sizeof(DIR))) == 0)
		return NULL;

	/* Let's try to find a file matching the given name */
	if ((od->hFindFile = FindFirstFile(buf, &od->ffd))
		== INVALID_HANDLE_VALUE) {
		free(od);
		return NULL;
	}
	od->first = 1;
	return od;
}

DIRENT *
readdir(DIR *dirp)
{
 if (dirp->first)
  dirp->first = 0;
 else if (!FindNextFile(dirp->hFindFile, &dirp->ffd))
  return NULL;
 dirp->de.d_name = dirp->ffd.cFileName;
 return &dirp->de;
}

int
closedir(DIR *dirp)
{
 FindClose(dirp->hFindFile);
 free(dirp);
 return 0;
}
