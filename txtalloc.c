/******************************************************************************
 * Copyright 1995-2004,2010 by Thomas E. Dickey.  All Rights Reserved.        *
 *                                                                            *
 * Permission to use, copy, modify, and distribute this software and its      *
 * documentation for any purpose and without fee is hereby granted, provided  *
 * that the above copyright notice appear in all copies and that both that    *
 * copyright notice and this permission notice appear in supporting           *
 * documentation, and that the name of the above listed copyright holder(s)   *
 * not be used in advertising or publicity pertaining to distribution of the  *
 * software without specific, written prior permission.                       *
 *                                                                            *
 * THE ABOVE LISTED COPYRIGHT HOLDER(S) DISCLAIM ALL WARRANTIES WITH REGARD   *
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND  *
 * FITNESS, IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT HOLDER(S) BE LIABLE  *
 * FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES          *
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN      *
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR *
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.                *
 ******************************************************************************/

#ifndef	NO_IDENT
static const char Id[] = "$Id: txtalloc.c,v 6.5 2010/06/27 21:58:28 tom Exp $";
#endif

/*
 * Title:	txtalloc.c (text-allocator)
 * Author:	T.E.Dickey
 * Created:	29 Apr 1988
 *
 * Function:	Maintains a balanced binary tree of character strings.
 *		The algorithm is taken from "The Art of Computer Programming
 *		-- Volume 3 -- Sorting and Searching", by Donald Knuth.
 */

#include "conflict.h"

typedef struct _node {
    struct _node *links[2];
    char balance;		/* holds 0, -1, +1 */
    char value[1];		/* 1 for the EOS... */
} TNODE;

#define	llink	links[0]
#define	rlink	links[1]

			/* definitions to make this simple, like Knuth */
#define	LLINK(p)	p->llink
#define	RLINK(p)	p->rlink
#define	KEY(p)		p->value
#define	B(p)		p->balance

#define	LINK(a,p)	p->links[(a)>0]

static TNODE head;

static
TNODE *
new_TNODE(char *text)
{
    register TNODE *p = TypeAllocN(TNODE, strlen(text));
    (void) strcpy(KEY(p), text);
    LLINK(p) = 0;
    RLINK(p) = 0;
    B(p) = 0;
    return (p);
}

char *
txtalloc(char *text)
{
    /* (A1:Initialize) */
    register
    TNODE *t = &head,		/* 't' points to the father of 's'      */
     *s = RLINK(t),		/* 's' points to rebalancing point      */
     *p = RLINK(t),		/* 'p' moves down the tree              */
     *q, *r;
    register
    int a;
    char *value;

    if (p == 0) {
	RLINK(t) = p = new_TNODE(text);
	return (KEY(p));
    }
    /* (A2:Compare) */
    while ((a = strcmp(text, value = KEY(p))) != 0) {
	/* (A3,A4: move left/right accordingly) */
	if ((q = LINK(a, p)) != NULL) {
	    if (B(q)) {
		t = p;
		s = q;
	    }
	    p = q;
	    /* ...continue comparing */
	} else {
	    /* (A5:Insert) */
	    LINK(a, p) = q = new_TNODE(text);
	    value = KEY(q);

	    /* (A6:Adjust balance factors) */
	    /*
	     * Now the balance factors on nodes between 's' and 'q'
	     * need to be changed from zero to +/- 1.
	     */
	    if (strcmp(text, KEY(s)) < 0)
		r = p = LLINK(s);
	    else
		r = p = RLINK(s);

	    while (p != q) {
		if ((a = strcmp(text, KEY(p))) != 0) {
		    B(p) = (a < 0) ? -1 : 1;
		    p = LINK(a, p);
		}
	    }

	    /* (A7:Balancing act) */
	    a = (strcmp(text, KEY(s)) < 0) ? -1 : 1;

	    if (B(s) == 0) {
		/* ...the tree has grown higher */
		B(s) = (char) a;
		head.llink++;
	    } else if (B(s) == -a) {
		/* ...the tree has gotten more balanced */
		B(s) = 0;
	    } else if (B(s) == a) {
		/* ...the tree has gotten out of balance */
		if (B(r) == a) {
		    /* (A8:Single rotation) */
		    p = r;
		    LINK(a, s) = LINK(-a, r);
		    LINK(-a, r) = s;

		    B(s) = B(r) = 0;
		} else if (B(r) == -a) {
		    /* (A9: Double rotation) */
		    p = LINK(-a, r);
		    LINK(-a, r) = LINK(a, p);
		    LINK(a, p) = r;
		    LINK(a, s) = LINK(-a, p);
		    LINK(-a, p) = s;

		    if (B(p) == a) {
			B(s) = (char) -a;
			B(r) = 0;
		    } else if (B(p) == 0) {
			B(s) = 0;
			B(r) = 0;
		    } else if (B(p) == -a) {
			B(s) = 0;
			B(r) = (char) a;
		    }

		    B(p) = 0;
		}
		/* A10:Finishing touch */
		t->links[(s == RLINK(t))] = p;
	    }
	    break;
	}
    }
    return (value);
}

/*
 * Dummy entry for consistency
 */
void
txtfree(char *text)
{
    (void) text;
}

/******************************************************************************/
#if NO_LEAKS || defined(TEST)
static
void
free_TNODE(TNODE * p)
{
    if (p != 0) {
	free_TNODE(p->llink);
	free_TNODE(p->rlink);
	free((char *) p);
    }
}

void
free_txtalloc(void)
{
    free_TNODE(head.rlink);
}
#endif /* NO_LEAKS */

/******************************************************************************/
#ifdef	TEST
void
txtdump(TNODE * p, int level)
{
    register int j;

    if (p) {
	txtdump(LLINK(p), level + 1);
	for (j = 0; j < level; j++)
	    printf(". ");
	printf("%s (%d)\n", KEY(p), B(p));
	txtdump(RLINK(p), level + 1);
    }
}

int
main(int argc, char *argv[])
{
    register int j;
    for (j = 1; j < argc; j++)
	(void) txtalloc(argv[j]);
    txtdump(head.rlink, 0);
    free_txtalloc();
    exit(SUCCESS);
    /*NOTREACHED */
}
#endif
