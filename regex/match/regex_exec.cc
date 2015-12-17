#include "regex.h"

int  tagstk[MAXTAG];             /* subpat tag stack..*/
CHAR nfa[MAXNFA];		/* automaton..       */

CHAR bittab[BITBLK];		/* bit table for CCL */
/* pre-set bits...   */
CHAR bitarr[] = {1,2,4,8,16,32,64,128};


static int pmatch_nr(char *, CHAR *, int &, int &,  int *, int *,int);
int re_exec(char lp[2048] , CHAR nfa[1024]);

#define NUM_MATCHES 1
void re_exec_par(char lp[2048], CHAR nfa[NUM_MATCHES*1024], int out_match[NUM_MATCHES])
{
	re_exec_par_label2:for (int i = 0 ; i < NUM_MATCHES; i++) {

		out_match[i] = re_exec(lp,&nfa[i*1024]);
	}
}

/*
 * re_exec:
 * 	execute nfa to find a match.
 *
 *	special cases: (nfa[0])	
 *		BOL
 *			Match only once, starting from the
 *			beginning.
 *		CHR
 *			First locate the character without
 *			calling pmatch, and if found, call
 *			pmatch for the remaining string.
 *		END
 *			re_comp failed, poor luser did not
 *			check for it. Fail fast.
 *
 *	If a match is found, bopat[0] and eopat[0] are set
 *	to the beginning and the end of the matched fragment,
 *	respectively.
 *
 */

int re_exec(char lp[2048] , CHAR nfa[1024]) {

	//#pragma HLS INTERFACE bram port=lp  depth=2048
	//#pragma HLS INTERFACE bram port=nfa depth=1024

	int bopat[MAXTAG];
	int eopat[MAXTAG];
	int bol;
	register CHAR c;
	register int ep = 0;
	register CHAR *ap = nfa;
	int lpi = 0, api = 0;
	bol = lpi;

	bopat[0] = 0;
	bopat[1] = 0;
	bopat[2] = 0;
	bopat[3] = 0;
	bopat[4] = 0;
	bopat[5] = 0;
	bopat[6] = 0;
	bopat[7] = 0;
	bopat[8] = 0;
	bopat[9] = 0;

	switch(ap[api]) {

	case BOL:			/* anchored: match from BOL only */
		ep = pmatch_nr(lp,nfa,lpi, api,bopat,eopat,bol);
		break;

	case CHR:			/* ordinary char: locate it fast */
		c = *(ap+1);
		while (*lp && *lp != c)
			lpi++;
		if (!lp[lpi])		/* if EOS, fail, else fall thru. */
			return 0;
	default:			/* regular matching all the way. */
		if ((ep = pmatch_nr(lp,nfa, lpi, api, bopat,eopat,bol)))
			break;
		return 0;
	case END:			/* munged automaton. fail always */
		return 0;
	}
	if (!ep)
		return 0;

	bopat[0] = lpi;
	eopat[0] = ep;

	return 1;
}

/* 
 * pmatch_nr: internal routine for the hard part
 *
 * 	This code is partly snarfed from an early grep written by
 *	David Conroy. The backref and tag stuff, and various other
 *	innovations are by oz.
 *
 *	special case optimizations: (nfa[n], nfa[n+1])
 *		CLO ANY
 *			We KNOW .* will match everything upto the
 *			end of line. Thus, directly go to the end of
 *			line, without recursive pmatch calls. As in
 *			the other closure cases, the remaining pattern
 *			must be matched by moving backwards on the
 *			string recursively, to find a match for xy
 *			(x is ".*" and y is the remaining pattern)
 *			where the match satisfies the LONGEST match for
 *			x followed by a match for y.
 *		CLO CHR
 *			We can again scan the string forward for the
 *			single char and at the point of failure, we
 *			execute the remaining nfa recursively, same as
 *			above.
 *
 *	At the end of a successful match, bopat[n] and eopat[n]
 *	are set to the beginning and end of subpatterns matched
 *	by tagged expressions (n = 1 to 9).	
 *
 */
#define re_fail(...)
#ifndef re_fail
extern void re_fail(char *, CHAR);
#endif


/* non-recursive version of the above routine */
static int  re_match_basic(char *lp, CHAR *ap, int & lpi, int api, int *bopat, int *eopat, int bol)
{
	register int op, c, n;
	register int bp;		/* beginning of subpat.. */
	register int ep;		/* ending of subpat..	 */
	int sapi = api;
	op = ap [api++];
	while (op != END && (op != CLO)) {
		switch(op) {
			
		case CHR:
			if (lp[lpi++] != ap[api++]) {
				lpi--;
				return -1;
			}
			break;
		case ANY:
			if (!lp[lpi++]) {
				lpi--;
				return -1;
			}
			break;
		case CCL:
			c = lp[lpi++];
			if (!isinset(&ap[api],c)) {
				lpi--;
				return -1;
			}
			api += BITBLK;
			break;
		case BOL:
			if (lpi != bol)
				return -1;
			break;
		case EOL:
			if (lp[lpi])
				return -1;
			break;

		case BOT:
			bopat[ap[api++]] = lpi;
			break;
		case EOT:
			eopat[ap[api++]] = lpi;
			break;

		case BOW:
			if (lpi!=bol && (iswordc(lp[lpi-1]) || !iswordc(lp[lpi])))
				return -1;
			break;
		case EOW:
			if (lpi==bol || !iswordc(lp[lpi-1]) || iswordc(lp[lpi]))
				return -1;
			break;
		case REF:
			n = ap[api++];
			bp = bopat[n];
			ep = eopat[n];
			while (bp < ep)
				if (lp[bp++] != lp[lpi++]) {
					lpi--;
					return -1;
				}
			break;
		}
		op = ap[api++];
	}
	api--;
	return api - sapi;
}

static int pmatch_nr(char *lp, CHAR *ap, int & lpi, int &api, int *bopat, int *eopat, int bol)
{
	register int op, c, n;
	register char *e;		/* extra pointer for CLO */
	register char *bp;		/* beginning of subpat.. */
	register char *ep;		/* ending of subpat..	 */
	int are;			/* to save the line ptr. */

	while ((op = ap[api++]) != END) {
		if (op != CLO ) {
			api--;
			int rv = re_match_basic(lp,ap,lpi, api, bopat,eopat,bol);
			if (rv < 0) return 0;
			api += rv;
		} else {
			int rv;
			are = lpi;
			switch(ap[api]) {
				
			case ANY:
				while (lp[lpi])
					lpi++;
				n = ANYSKIP;
				break;
			case CHR:
				c = ap[api+1];
				while (lp[lpi] && c == lp[lpi])
					lpi++;
				n = CHRSKIP;
				break;
			case CCL:
				while ((c = lp[lpi]) && isinset(&ap[api+1],c))
					lpi++;
				n = CCLSKIP;
				break;
			default:
				re_fail("closure: bad nfa.", *ap);
				return 0;
			}
			api += n;
			while (lpi >= are) {
				int o_lpi = lpi;
				rv = re_match_basic(lp,ap,lpi,api,bopat,eopat,bol);
				if (rv == 0) break;
				if (rv > 0) {
					api += rv;
					break;
				}
				lpi = --o_lpi;
			}
		}
	}
	return lpi;
}
