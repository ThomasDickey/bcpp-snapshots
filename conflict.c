/******************************************************************************
 * Copyright 1995 by Thomas E. Dickey.  All Rights Reserved.                  *
 *                                                                            *
 * You may freely copy or redistribute this software, so long as there is no  *
 * profit made from its use, sale trade or reproduction. You may not change   *
 * this copyright notice, and it must be included in any copy made.           *
 ******************************************************************************/
#ifndef	NO_IDENT
static	char	Id[] = "$Header: /users/source/archives/conflict.vcs/RCS/conflict.c,v 6.0 1995/03/18 14:05:04 dickey Rel $";
#endif

/*
 * Title:	conflict.c (path-conflict display)
 * Author:	T.E.Dickey
 * Created:	15 Apr 1988
 *
 * Function:	Reports pathname conflicts by making a list of the directories
 *		which are listed in the environment variable PATH, and then
 *		scanning these directories for executable files.  If arguments
 *		given to this program, the test for executable files is limited
 *		to the given names (after stripping directory-prefix).
 *
 *		The report is sent to standard output and always shows the
 *		list of directories.  Conflicts are reported on succeeding
 *		lines.
 */

#include "conflict.h"

#define	CHUNK	0xff		/* (2**n) - 1 chunk for reducing realloc's */

static	INPATH	*inpath;
static	DIRS	*dirs;
static	char	*env_name;	/* e.g., "PATH" */
static	char	*pathlist;	/* e.g., ".:/bin:/usr/bin" */
static	char	**FileTypes;
static	char	*dot;
static	size_t	total,
		path_len,	/* maximum number of items in path */
		num_types;	/* number of file-types to scan */
static	int	acc_mask,	/* permissions we're looking for */
		p_opt,		/* print pathnames (not a table) */
		v_opt,		/* verbose (repeat for different levels) */
		do_blips;
static	char	*w_opt	= "",	/* pads listing */
		*w_opt_text = "--------";

#if SYS_MSDOS || SYS_OS2
#define DOS_upper(s) strupr(s)
#else
#define DOS_upper(s) s
#endif

#if SYS_MSDOS
#define TYPES_PATH ".COM.EXE.BAT.PIF"
#endif

#if SYS_OS2
#define TYPES_PATH ".EXE.CMD.bat.com.sys"
#endif

static	char *	TypesOf ARGS((size_t len, INPATH *ip));
static	int	LookupDirs ARGS((char *name, int n));
static	int	SameTypeless ARGS((char *a, char *b));
static	int	cmp_INPATH ARGS((const void *p1, const void *p2));
static	int	had_conflict ARGS((INPATH *ip));
static	type_t	LookupType ARGS((char *name));
static	void	AddToPath ARGS((char *path));
static	void	ScanConflicts ARGS((char *path, int inx, int argc, char **argv));
static	void	ShowConflicts ARGS((int len, INPATH *ip));
static	void	ShowPathnames ARGS((INPATH *ip));
static	void	compress_list ARGS((void));
static	void	usage ARGS((void));

#if USE_INODE
static	void	node_found ARGS((INPATH *ip, int n, type_t f, struct stat *sb));
#else
static	void	node_found ARGS((INPATH *ip, int n, type_t flags));
#endif

/*
 * comparison procedure used for sorting list of names for display
 */
static
int	cmp_INPATH(p1, p2)
	const	void	*p1;
	const	void	*p2;
{
	return strcmp(((INPATH *)(p1))->name, ((INPATH *)(p2))->name);
}

/*
 * save information from the current stat-block so we can test for repeated
 * instances of the file.
 */
#if USE_INODE
static
void	node_found(ip, n, flags, sb)
	INPATH *	ip;
	int		n;
	type_t		flags;
	struct stat *	sb;
{
	NODEFLAGS(n)      |= flags;
	ip->node[n].device = sb->st_dev;
	ip->node[n].inode  = sb->st_ino;
}
#define	FoundNode(ip,n) node_found(ip,n,ok,&sb)

#else
static
void	node_found(INPATH *ip, int n, type_t flags)
{
	NODEFLAGS(n) |= flags;
}
#define	FoundNode(ip,n) node_found(ip,n,ok)
#endif	/* USE_INODE */

/*
 * Look up the given name and fill-in the n'th entry of the 'dirs[]' array.
 */
static
int	LookupDirs(name, n)
	char		*name;
	int		n;
{
	int	found	= FALSE;
#if USE_INODE
	struct	stat	sb;

	if (stat(name, &sb) >= 0) {
		dirs[n].device = sb.st_dev;
		dirs[n].inode  = sb.st_ino;
		found = TRUE;
	}
#else
	char	resolved[MAXPATHLEN];
	if (realpath(name, resolved) != 0) {
		dirs[n].actual = MakeString(resolved);
		found = TRUE;
	}
#endif
	return found;
}

/*
 * Display the conflicting items
 */
static
void	ShowConflicts(len, ip)
	int		len;
	INPATH *	ip;
{
	register int j;
	int	k = -1;
	char flags[BUFSIZ];

	for (j = 0; j < len; j++) {
		if (FileTypes == 0) {
			register int d = '-';
			if (ip != 0) {
				if (IS_A_NODE(j)) {
					d = '*';
					if (k < 0) {
						k = j;	/* latch first index */
					} else {
						if (!SAME_NODE(j,k))
							d = '+';
					}
				}
			}
			(void)printf("%s%c", w_opt, d);
			continue;
		}

		for (k = 0; k < num_types+2; k++)
			flags[k] = '-';
		flags[k-1] = EOS;
		flags[0] = ':';
		if (ip != 0 && IS_A_NODE(j)) {
			for (k = 0; k < num_types; k++) {
				if ((1 << k) & NODEFLAGS(j)) {
					char *s = FileTypes[k];
					int c = *s++;
					if (c == EOS || *s == EOS)
						c = '.';
					else
						c = *s;
					flags[k+1] = c;
				}
			}
		}
		(void)printf("%s%s", w_opt, flags);
	}
}

static
void	ShowPathnames (ip)
	INPATH	*ip;
{
	int	j, k;
	char	bfr[MAXPATHLEN];

	if (num_types != 0) {
		(void)strcpy(bfr, ip->name);
		*ftype(bfr) = EOS;
	}

	if (p_opt) {
		for (j = 0; j < path_len; j++) {
			if (NODEFLAGS(j) != 0) {
				if (num_types != 0) {
					for (k = 0; k < num_types; k++) {
						if ((1<<k) & NODEFLAGS(j)) {
							(void)printf("%s%c%s%s\n",
								dirs[j].name,
								PATHNAME_SEP,
								bfr,
								FileTypes[k]);
						}
					}
				} else {
					(void)printf("%s%c%s\n",
						dirs[j].name,
						PATHNAME_SEP,
						ip->name);
				}
			}
		}
	} else if (num_types != 0) {
		(void)printf(": %s (%s)\n", bfr, TypesOf(path_len, ip));
	} else {
		(void)printf(": %s\n", ip->name);
	}
}

static
char *	TypesOf(len, ip)
	size_t		len;
	INPATH *	ip;
{
	register int j, mask, n, k;
#if NO_LEAKS
	static	char	result[BUFSIZ];
#else
	static	char	*result;

	if (result == 0)
		result = malloc((num_types*4) + 1);
#endif

	for (j = mask = 0; j < len; j++)
		mask |= NODEFLAGS(j);

	result[0] = EOS;
	for (n = 0, k = 1; n < num_types; n++, k <<= 1) {
		if (k & mask) {
			if (result[0] != EOS)
				(void)strcat(result, ",");
			(void)strcat(result, FileTypes[n]);
		}
	}
	return result;
}

/*
 * Compress the list of paths, removing those which had no conflicts.
 */
static
void	compress_list()
{
	register int	j, k, jj;
	int	compress;
	type_t	flags;

	for (j = 0; j < path_len; j++) {
		for (k = 0, compress = TRUE; compress && (k < total); k++) {
			register INPATH *ip = inpath + k;
			if ((flags = NODEFLAGS(j)) != 0) {
				/* If there's more than one suffix found for
				 * the current entry, we cannot compress it.
				 */
				while (flags != 0) {
					if ((flags & 1) != 0)
						break;
					flags >>= 1;
				}
				if (flags == 1) {
					for (jj = 0; jj < path_len; jj++) {
						if (jj == j)
							continue;
						if (IS_A_NODE(jj)) {
							compress = FALSE;
							break;
						}
					}
				} else {
					compress = FALSE;
				}
			}
		}
		if (compress) {
			if (v_opt) {
				(void)fprintf(stderr, "no conflict:%s\n", dirs[j].name);
				(void)fflush(stderr);
			}
			FreeString(dirs[j].name);
			for (jj = j+1; jj < path_len; jj++)
				dirs[jj-1] = dirs[jj];
			for (k = 0; k < total; k++) {
				register INPATH *ip = inpath + k;
				for (jj = j+1; jj < path_len; jj++)
					ip->node[jj-1] = ip->node[jj];
			}
			j--;
			path_len--;
		}
	}
}

/*
 * returns true iff we have two instances of the same name
 */
static
int	had_conflict(ip)
	INPATH *	ip;
{
	register int	j, k;
	int	first	= TRUE;
	type_t	mask;

	for (j = 0; j < path_len; j++) {
		if ((mask = NODEFLAGS(j)) != 0) {

			if (FileTypes == 0) {
				if (!first)
					return TRUE;
				first = FALSE;
				continue;
			}

			for (k = 0; k < num_types; k++) {
				if (mask & (1<<k)) {
					if (!first)
						return TRUE;
					first = FALSE;
				}
			}
		}
	}
	return (FALSE);
}

/* Returns nonzero if the given filename has an executable-suffix */
static
type_t	LookupType(name)
	char *		name;
{
	register int k;
	char	temp[MAXPATHLEN];
	char	*type = DOS_upper(strcpy(temp, ftype(name)));

	for (k = 0; k < num_types; k++) {
		if (!strcmp(type, FileTypes[k]))
			return (1<<k);
	}
	return 0;
}

/* Compare two leaf-names, ignoring their suffix. */
static
int	SameTypeless(a, b)
	char *		a;
	char *		b;
{
	char	*type_a = ftype(a);
	char	*type_b = ftype(b);
	if (type_a - a == type_b - b) {
		if (!strncmp(a, b, (size_t)(type_a - a)))
			return TRUE;
	}
	return FALSE;
}
#define SameName(a,b) ((FileTypes == 0) ? SameString(a,b) : SameTypeless(a,b))

static
void	ScanConflicts(path, inx, argc, argv)
	char *	path;
	int	inx;
	int	argc;
	char **	argv;
{
	DIR		*dp;
	struct dirent	*de;
	struct stat	sb;
	register int	j;
#if SYS_MSDOS || SYS_OS2
	char	save_wd[MAXPATHLEN];
#endif

	/*
	 * When scanning a directory, we first chdir to it, mostly to make
	 * the scan+stat work faster, but also because some systems don't
	 * scan properly otherwise.
	 *
	 * MSDOS and OS/2 are a little more complicated, because each drive
	 * has its own current directory.
	 */
#if SYS_MSDOS || SYS_OS2
	(void) strcpy(save_wd, dot);
	if (!strcmp(".", path)) {
		path = dot;
	} else if (!same_drive(dot, path)) {
		if (!set_drive(path))
			return;
		getwd(save_wd);
	}
#endif
	if (set_directory(path)
	 && (dp = opendir(path)) != NULL) {

		while ((de = readdir(dp)) != NULL) {
		register
		type_t	ok	= 0;
		int	found	= FALSE;
		char	buffer[MAXPATHLEN];
		char	*the_name;

			if (do_blips)
				blip('.');

			(void)sprintf(buffer, "%.*s", (int)NAMLEN(de), de->d_name);
			the_name = MakeString(DOS_upper(buffer));

			/* If arguments are given, restrict search to them */
			if (argc > optind) {
				for (j = optind; j < argc; j++) {
					if (SameName(argv[j], the_name)) {
						found = TRUE;
						break;
					}
				}
				if (!found)
					continue;
			}

			/* Verify that the name is a file, and executable */
			if (stat(the_name, &sb) < 0)
				continue;
			if ((sb.st_mode & S_IFMT) != S_IFREG)
				continue;

#if SYS_UNIX || SYS_OS2
			if (access(the_name, acc_mask) < 0)
				continue;
			ok = 1;
#endif
			if (FileTypes != 0) {
				if ((ok = LookupType(the_name)) == 0)
					continue;
			}

			/* Find the name in our array of all names */
			found	= FALSE;
			for (j = 0; j < total; j++) {
				if (SameName(inpath[j].name, the_name)) {
					FoundNode(&inpath[j], inx);
					found = TRUE;
					break;
				}
			}

			/* If not there, add it */
			if (!found) {
				if (!(total & CHUNK)) {
					size_t need = ((total*3)/2 | CHUNK) + 1;
					if (inpath != 0)
						inpath = TypeRealloc(INPATH,
								inpath, need);
					else
						inpath = TypeAlloc(INPATH, need);
				}
				j = total++;
				inpath[j].name = the_name;
				inpath[j].node = TypeAlloc(NODE, path_len);
				FoundNode(&inpath[j], inx);
			}
			if (v_opt > 2) {
				(void)printf("%c %s/%s\n",
					found ? '+' : '*',
					path, the_name);
			}
		}
		(void)closedir(dp);
	}
#if SYS_MSDOS || SYS_OS2
	if (strcmp(dot, save_wd)) {
		chdir (save_wd);
	}
#endif
	(void)set_directory(dot);
}

/*
 * Build up the 'pathlist' string in the same form as we expect from an
 * environment variable, from command-line option values.
 */
static
void	AddToPath(path)
	char	*path;
{
	size_t	need = strlen(path) + 1;

	if (pathlist != 0) {
		size_t have = strlen(pathlist);
		pathlist = realloc(pathlist, have + need + 1);
		(void)sprintf(pathlist + have, "%c%s", PATHLIST_SEP, path);
	} else {
		pathlist = malloc(need);
		(void)strcpy(pathlist, path);
	}
}

static
void	usage()
{
	static	char	*tbl[] = {
		 "Usage: conflict [options] [list_of_files]"
		,""
		,"Lists conflicts of executable files in current PATH."
		,"First instance in path (and those linked to it) are marked"
		,"with \"*\", succeeding instances with \"+\" marks."
		,""
		,"Options are:"
		,"  -e name    specify another environment variable than $PATH"
		,"  -p         print pathnames only"
		,"  -r         look for read-able files"
		,"  -t types   look only for given file-types"
		,"  -v         (verbose) show names found in each directory"
		,"             once: shows all directory names"
		,"             twice: shows all filenames"
		,"  -w         look for write-able files"
		,"  -W number  expand width of display by number of columns"
		,"  -x         look for execut-able files (default)"
		,""
		,"Also, C-preprocessor-style -I and -L options"
	};
	register int	j;
	for (j = 0; j < SIZEOF(tbl); j++)
		(void)fprintf(stderr, "%s\n", tbl[j]);
	(void)fflush(stderr);
	(void)exit(EXIT_FAILURE);
}

void	failed(s)
	char	*s;
{
	perror(s);
	exit(EXIT_FAILURE);
}

int	main(argc, argv)
	int	argc;
	char	*argv[];
{
	register int	found, j, k;
	char	*s, *t;
	char	*type_list = 0;
	char	bfr[MAXPATHLEN];

	while ((j = getopt(argc, argv, "D:e:I:prt:U:vwW:x")) != EOF) {
		switch (j) {
		case 'e':	env_name = optarg;	break;
		case 't':	type_list = optarg;	break;
		case 'v':	v_opt++;		break;
		case 'p':	p_opt = TRUE;		break;

		case 'W':	k = (int)strtol(optarg, &t, 0);
				if (*t != EOS || k < 0)
					usage();
				k = strlen(w_opt_text) - k;
				if (k < 0)
					k = 0;
				w_opt = w_opt_text + k;
				break;

		case 'r':	acc_mask |= R_OK;	break;
		case 'w':	acc_mask |= W_OK;	break;
		case 'x':	acc_mask |= X_OK;	break;

			/*
			 * The 'U' and 'D' options are parsed to simplify
			 * using C-preprocessor options to scan for include-
			 * files.
			 */
		case 'U':	/* ignored */		break;
		case 'D':	/* ignored */		break;
		case 'I':	AddToPath(optarg);	break;
		case 'L':	AddToPath(optarg);	break;

		default:	usage();
				/*NOTREACHED*/
		}
	}

	/* The "-v" and "-p" options aren't meaningful in combination */
	if (v_opt && p_opt)
		usage();

	/* The remaining arguments are the programs/symbols that we're looking
	 * for.  Reduce the argument-list to the leaf-names (stripping paths).
	 */
	if (argc > optind) {
		for (j = optind; j < argc; j++) {
			argv[j] = MakeString(DOS_upper(fleaf(argv[j])));
		}
	}

	do_blips = ((v_opt > 1) && isatty(fileno(stderr))); 

	if (acc_mask == 0)
		acc_mask = X_OK;

	/*
	 * Get the current working-directory, so we have a reference point to
	 * go back after scanning directories.
	 */
	if (getwd(bfr) == 0)
		failed("getcwd");
	dot = MakeString(bfr);
	if (!p_opt)
		(void)printf("Current working directory is \"%s\"\n", dot);

	/*
	 * Obtain the list of directories that we'll scan.
	 */
	if (pathlist == 0) {
		if (env_name == 0)
			env_name = "PATH";

		if ((s = getenv(env_name)) != 0)
			pathlist = strdup(s);
		else
			failed(env_name);

#if SYS_MSDOS || SYS_OS2
		if (!strcmp(env_name, "PATH")) {
			/* look in current directory before looking in $PATH */
			s = malloc(strlen(pathlist)+3);
			(void)sprintf(s, ".%c%s", PATHLIST_SEP, pathlist);
			free(pathlist);
			pathlist = s;
		}
#endif
	}
	for (s = DOS_upper(pathlist), path_len = 1; *s != EOS; s++)
		if (*s == PATHLIST_SEP)
			path_len++;
	dirs = TypeAlloc(DIRS, path_len);

	/*
	 * Reconstruct the type-list (if any) as an array to simplify scanning.
	 */
#if SYS_MSDOS || SYS_OS2
	 if (type_list == 0) {
		if (!strcmp(env_name, "PATH"))
			type_list = TYPES_PATH;
	}
#endif
	if (type_list != 0) {
		type_list = DOS_upper(strdup(type_list));
		for (s = type_list, num_types = 0; *s != EOS; s++) {
			if (*s == '.') {
				num_types++;
#if SYS_UNIX			/* "." and "" are different types */
				if ((s[1] == '.') || (s[1] == EOS))
					num_types++;
#endif
			}
		}
		if (num_types == 0 || *type_list != '.') {
			(void)fprintf(stderr, "Type-list must be .-separated\n"); 
			exit(EXIT_FAILURE);
		}
		FileTypes = TypeAlloc(char *, num_types);
		j = num_types;
		do {
			if (*--s == '.') {
				FileTypes[--j] = strdup(s);
#if SYS_UNIX			/* "." and "" are different types */
				if (s[1] == EOS)
					FileTypes[--j] = strdup("");
#endif
				*s = EOS;
			}
		} while (j != 0);
		free(type_list);
	}

	/*
	 * Build a list of the directories in the $PATH variable, excluding any
	 * that happen to be the same inode (e.g., ".", or symbolically linked
	 * directories).
	 */
	for (s = pathlist, j = 0; *s != EOS; s = t) {
		for (k = 0; s[k] != PATHLIST_SEP; k++)
			if ((bfr[k] = s[k]) == EOS)
				break;

		t = (s[k] != EOS) ? s+k+1 : s+k;
		if (k == 0)
			(void)strcpy(bfr, ".");
		else
			bfr[k] = EOS;

		if (LookupDirs(bfr, j)) {
			for (k = 0, found = FALSE; k < j; k++) {
				if (SAME_DIRS(j,k)) {
					found = TRUE;
					break;
				}
			}
			if (!found) {
				dirs[j].name = MakeString(bfr);
				j++;
			}
		}
	}
	path_len = j;

	/*
	 * For each unique directory in $PATH, scan it, looking for executable
	 * files.
	 */
	for (j = 0; j < path_len; j++) {
		if (v_opt > 1) {
			ShowConflicts(j+1, (INPATH *)0);
			(void)printf("> %s\n", dirs[j].name);
		} else if (do_blips) {
			(void)fprintf(stderr, "%s", dirs[j].name);
			(void)fflush(stdout);
			(void)fflush(stderr);
		}
		ScanConflicts(dirs[j].name, j, argc, argv);
		if (do_blips)
			blip('\n');
	}

	/*
	 * The normal (unverbose) listing shows only the conflicting files,
	 * and the directories in which they are found.
	 */
	if (!v_opt)
		compress_list();

	if (!p_opt) {
		for (j = 0; j < path_len; j++) {
			ShowConflicts(j+1, (INPATH *)0);
			(void)printf("> %s\n", dirs[j].name);
		}
	}

	if (total != 0) {
		qsort((char *)inpath, (size_t)total, sizeof(INPATH), cmp_INPATH);
		for (j = 0; j < total; j++) {
			if ((v_opt > 1) || had_conflict(&inpath[j])) {
				if (!p_opt)
					ShowConflicts((int)path_len, &inpath[j]);
				ShowPathnames(&inpath[j]);
			}
		}
	}

	/*
	 * Test/debug: free all memory so we can validate the heap.
	 */
#if NO_LEAKS
	if (FileTypes != 0) {
		for (j = 0; j < num_types; j++)
			free(FileTypes[j]);
		free((char *)FileTypes);
	}
	if (dirs != 0) {
# if !USE_TXTALLOC
		for (j = 0; j < path_len; j++) {
			FreeString(dirs[j].name);
		}
# endif
		free((char *)dirs);
	}
	if (inpath != 0) {
		for (j = 0; j < total; j++) {
			FreeString(inpath[j].name);
			free((char *)(inpath[j].node));
		}
		free((char *)inpath);
	}
	free(pathlist);
	FreeString(dot);
# if USE_TXTALLOC
	free_txtalloc();
# endif
# if HAVE_DBMALLOC_H
	malloc_dump(fileno(stdout));
# endif
#endif
	(void)fflush(stderr);
	(void)fflush(stdout);
	(void)exit(EXIT_SUCCESS);
	/*NOTREACHED*/
	return 0;
}
