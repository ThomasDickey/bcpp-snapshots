/* $Id: conflict.h,v 6.4 2004/03/27 11:32:52 tom Exp $
 *
 * Common/configurable definitions and types for 'conflict'.
 */

#ifndef HAVE_CONFIG_H
# define HAVE_CONFIG_H 0	/* Watcom's cpp isn't ANSI... */
#endif

#if HAVE_CONFIG_H
#include "config.h"
#else
# define HAVE_LINK 0
# define HAVE_REALPATH 0
# define HAVE_SYMLINK 0
# define HAVE_SYS_PARAM_H 0	/* only on real computers */
# ifdef __TURBOC__
#  define SYS_MSDOS 1
#  define HAVE_STDLIB_H 1
#  define HAVE_DIRENT_H 1
#  define HAVE_STRING_H 1
#  define HAVE_GETCWD   1
#  define HAVE_STRRCHR  1
#  define HAVE_STRDUP   1
#  define ino_t short
#  define dev_t short
#  include <dir.h>		/* ...to get MAXPATH */
#  include <io.h>		/* ...to get isatty */
#  if defined(MAXPATH) && !defined(MAXPATHLEN)
#   define MAXPATHLEN MAXPATH
#  endif
# endif
# ifdef __WATCOMC__
#  define HAVE_STDLIB_H 1
#  define HAVE_UNISTD_H 1
#  define HAVE_MALLOC_H 1
#  define HAVE_DIRECT_H 1
#  define HAVE_STRING_H 1
#  define HAVE_GETCWD   1
#  define HAVE_STRRCHR  1
#  define HAVE_STRDUP   1
/*#  ifdef __MSDOS__*/
/*#   define SYS_MSDOS 1*/
/*#  endif*/
#  ifdef __OS2__
#   include <os2/os2.h>
#   define SYS_OS2 1
#   define MAXPATHLEN CCHMAXPATH	/* in <os2/bsedos.h> */
#  endif
# endif
# ifdef WIN32
#  define HAVE_STDLIB_H 1
#  define HAVE_MALLOC_H 1
#  define HAVE_DIRECT_H 1
#  define HAVE_STRING_H 1
#  define HAVE_GETCWD   1
#  define HAVE_STRRCHR  1
#  define HAVE_STRDUP   1
#  define HAVE_STRING_H 1
#  define MAXPATHLEN    255
#  define SYS_WIN32     1
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#  include <io.h>		/* ...to get isatty */
#  define HAVE_DIRENT_H 1
# endif
#endif

#if defined(__EMX__) || defined(__UNIXOS2__)
# define SYS_OS2_EMX 1		/* makefile.emx, or configure-script */
#endif

/* Some compilers (e.g., Watcom 10.0a) don't treat conditionals properly
 * for undefined tokens.
 */
#ifndef HAVE_DBMALLOC_H
#define HAVE_DBMALLOC_H 0
#endif

#ifndef HAVE_DIRENT_H
#define HAVE_DIRENT_H 0
#endif

#ifndef HAVE_GETOPT_H
#define HAVE_GETOPT_H 0
#endif

#ifndef HAVE_STDLIB_H
#define HAVE_STDLIB_H 0
#endif

#ifndef NO_LEAKS
#define NO_LEAKS HAVE_DBMALLOC_H
#endif

#ifndef STDC_HEADERS
#define STDC_HEADERS 0
#endif

#ifndef SYS_MSDOS
#define SYS_MSDOS 0
#endif

#ifndef SYS_OS2
#define SYS_OS2 0
#endif

#ifndef SYS_OS2_EMX
#define SYS_OS2_EMX 0
#endif

#ifndef SYS_WIN32
#define SYS_WIN32 0
#endif

#ifndef SYS_UNIX
#define SYS_UNIX !(SYS_MSDOS || SYS_OS2 || SYS_WIN32)
#endif

/*
 * Include the stuff that's needed for our configuration.
 */
#include <stdio.h>

#if HAVE_STDLIB_H
# include <stdlib.h>
#endif

#if HAVE_UNISTD_H
# include <unistd.h>
#endif

#if STDC_HEADERS
# include <string.h>
#else
# if HAVE_STRING_H
#  include <string.h>
#  ifndef HAVE_STRRCHR
#   define strrchr rindex
#  endif
# endif
# if HAVE_MALLOC_H
#  include <malloc.h>
# endif
#endif

#include <sys/types.h>
#include <sys/stat.h>

#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#if HAVE_DIRENT_H
# include <dirent.h>
# define NAMLEN(dirent) strlen((dirent)->d_name)
#else
# define dirent direct
# if HAVE_DIRECT_H
#  include <direct.h>
#  define NAMLEN(dirent) strlen((dirent)->d_name)	/* Watcom OS/2 */
# else
#  define NAMLEN(dirent) (dirent)->d_namlen
#  if HAVE_SYS_NDIR_H
#   include <sys/ndir.h>
#  endif
#  if HAVE_SYS_DIR_H
#   include <sys/dir.h>
#  endif
#  if HAVE_NDIR_H
#   include <ndir.h>
#  endif
# endif
#endif

#if HAVE_GETOPT_H
#include <getopt.h>
#else
extern int getopt(int, char *const*, const char *);
extern int optind;
extern char *optarg;
#endif

#if HAVE_GETCWD
#define getwd(buffer) getcwd(buffer, sizeof(buffer))
#endif

#define TypeRealloc(type,p,n) (type *)realloc(p,(n)*sizeof(type))
#define TypeAlloc(type,n)     (type *)calloc(sizeof(type),n)
#define TypeAllocN(type,n)    (type *)calloc(sizeof(type)+n,1)

#undef  SIZEOF
#define SIZEOF(v) (sizeof(v)/sizeof(v[0]))

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define EOS '\0'

#ifndef R_OK
#define R_OK 1
#define W_OK 2
#define X_OK 4
#endif

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#endif

#if SYS_MSDOS || SYS_OS2 || SYS_WIN32
# define PATHNAME_SEP '\\'
# define PATHLIST_SEP ';'
#elif SYS_OS2_EMX
# define PATHNAME_SEP '/'
# define PATHLIST_SEP ';'
#else
# define PATHNAME_SEP '/'
# define PATHLIST_SEP ':'
#endif

typedef	unsigned type_t;

/*
 * If files can be linked together, it's simplest to 'stat()' them to determine
 * if they're identical.  Otherwise, we've got to expand the pathnames to
 * determine if they're the same place.
 */
#if HAVE_LINK || HAVE_SYMLINK
#define USE_INODE 1

#define	SAME_NODE(j,k)		(ip->node[j].device == ip->node[k].device\
			&&	 ip->node[j].inode  == ip->node[k].inode)
#define	SAME_DIRS(j,k)		(dirs[j].device == dirs[k].device\
			&&	 dirs[j].inode  == dirs[k].inode)

typedef	struct	{
		type_t	flags;
		dev_t	device;
		ino_t	inode;
	} NODE;

typedef	struct	{
		char	*name;
		dev_t	device;
		ino_t	inode;
	} DIRS;

#else	/* e.g., MSDOS */
#define USE_INODE 0

#define	SAME_NODE(j,k)		(j == k)
#define	SAME_DIRS(j,k)		SameString(dirs[j].actual, dirs[k].actual)

typedef	struct	{
		type_t	flags;
	} NODE;

typedef	struct	{
		char	*name;
		char	*actual;
	} DIRS;

#endif

#define NODEFLAGS(j)		ip->node[j].flags
#define IS_A_NODE(j)		(NODEFLAGS(j) != 0)

typedef	struct	{
		char	*name;	/* name of executable file */
		NODE	*node;	/* stat-result, for comparing */
	} INPATH;

/*
 * Prototypes for functions defined in this program:
 */
extern	int	main (int argc, char *argv[]);
extern	void	failed (char *s);
extern	char	*fleaf (char *name);
extern	char	*ftype (char *name);
extern	void	blip (int c);

/* MSDOS and OS/2 need a wrapper for 'chdir()' to also switch drives */
#if SYS_MSDOS || SYS_OS2 || SYS_OS2_EMX || SYS_WIN32
extern	int	have_drive (char *name);
extern	int	same_drive (char *a, char *b);
extern	int	set_drive (char *name);
extern	int	set_directory (char *name);
#else
#define set_directory(path) (chdir(path) >= 0)
#endif

#define USE_TXTALLOC 1
#if USE_TXTALLOC
extern	char	*txtalloc (char *s);
extern	void	free_txtalloc (void);
extern	void	txtfree (char *s);

# define MakeString(a)   txtalloc(a)
# define SameString(a,b) ((a) == (b))	/* txtalloc vs strcmp... */
# define FreeString(a)			/* ...actually we don't  */
#else
# define MakeString(a)    strdup(a)
# define SameString(a,b) !strcmp(a,b)
# define FreeString(a)    free(a)
#endif

/*
 * Prototypes for functions that may be defined in this program (if the target
 * system doesn't have an equivalent):
 */
#if !USE_INODE && !HAVE_REALPATH
# define realpath my_realpath
char	*my_realpath (char *given, char *actual);
#endif

#if !HAVE_STRDUP
# define strdup my_strdup
char	*my_strdup (char *s);
#endif

/*
 * Definitions for testing/debugging this program:
 */
#if HAVE_DBMALLOC_H
# include <dbmalloc.h>
#endif
