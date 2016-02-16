#include "regex.h"
//#include <string.h>


static int pmatch_nr    (char *, NFA_t *, int &, int &,  int *, int *,int);
static int pmatch_ip_hdr(char *, NFA_t *, int &, int &);
int re_exec(char lp[2048] , NFA_t nfa[1024]);

static int get_packet_len(char lp[2048]) 
{
	struct ip_header  *iph;
	iph = (struct ip_header *)(lp + sizeof(struct eth_header));
	return ((iph->ip_len[0] << 8) | iph->ip_len[1]) + sizeof(struct eth_header);
}

void re_match_packet(char lp[2048], NFA_t nfa_i[1024], char op[2048])
{
	if (re_exec(lp,nfa_i)) {
		int len = get_packet_len(lp) ;
		for (int i = 0 ; i < len; i++) {
#pragma HLS PIPELINE II=1			
			op[i] = lp[i];
		}
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

int re_exec(char lp[2048] , NFA_t nfa[1024]) {

	int bopat[MAXTAG];
	int eopat[MAXTAG];
	int bol;
	register NFA_t c;
	register int ep = 0;
	register NFA_t *ap = nfa;
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
		c = ap[api+1];
		while (lp[lpi] && lp[lpi] != c)
			lpi++;
		if (!lp[lpi])		/* if EOS, fail, else fall thru. */
			return 0;
	case IP_HDR:
		api++;
		if (!pmatch_ip_hdr(lp, ap, lpi, api)) return 0;
		if (ap[api] == END) return 1; // header match only
		else ep = lpi; // fall thru
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
extern void re_fail(char *, NFA_t);
#endif


/* non-recursive version of the above routine */
static int  re_match_basic(char *lp, NFA_t *ap, int & lpi, int api, int *bopat, int *eopat, int bol)
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
		case SKIP:
			lpi += ap[api++];
			break;
		}
		op = ap[api++];
	}
	api--;
	return api - sapi;
}

static int pmatch_nr(char *lp, NFA_t *ap, int & lpi, int &api, int *bopat, int *eopat, int bol)
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

static int pmatch_ip_hdr(char *lp, NFA_t *ap, int &lpi, int &api)
{
	struct eth_header *eth;
	struct ip_header  *iph;
	struct ports      *prt;
	int olpi          = lpi;

	eth = (struct eth_header *)lp;
	iph = (struct ip_header *)(lp + sizeof(struct eth_header));
	prt = (struct ports *)(lp + sizeof(eth_header) + sizeof(ip_header));

	while (1) {
		int i;
		switch (ap[api]) {
		case IP_SMA:
			api++;
			for (i = 0 ; i < 6 ; i++) {
				if (ap[api] != 255 && ap[api] != eth->src_addr[i]) 
					return 0;
				api++;
			}
			break;
		case IP_DMA:
			api++;
			for (i = 0 ; i < 6 ; i++) {
				if (ap[api] != 255 && ap[api] != eth->src_addr[i]) 
					return 0;
				api++;
			}
			break;
		case IP_TYPE:
			api++;
			if (eth->ip_type[0] != ap[api] ||
			    eth->ip_type[1] != ap[api+1]) return 0;
			api += 2;
			break;
		case IP_SA:
			api++;
			for (i = 0 ; i < 4; i++) {
				// -1 then allow all
				if (ap[api] != 255 && ap[api] != iph->ip_src.addr[i])
					return 0;
				api++;
			}
			break;
		case IP_DA:
			api++;
			for (i = 0 ; i < 4; i++) {
				if (ap[api] != 255 && ap[api] != iph->ip_dst.addr[i])
					return 0;
				api++;
			}
			break;
		case IP_DP:
			api++;
			if (prt->d_port[0] == ap[api] &&
			    prt->d_port[1] == ap[api+1]) 
				api += 2;
			else
				return 0;
			break;
		case IP_SP:
			api++;
			if (prt->s_port[0] == ap[api] &&
			    prt->s_port[1] == ap[api+1]) 
				api += 2;
			else
				return 0;
			break;
		case IP_FLAG:
			api++;
			if ((iph->ip_flags & ap[api]) == 0) return 0;
			api++;
			break;
		case IP_PROTO:
			api++;
			if (iph->ip_proto != ap[api]) return 0;
			api++;
			break;
		default:
			// increment depending on the packet type
			// so we can compare the data : we understand only a few
			// of the protocols
			switch (iph->ip_proto) {
			case 1: // ICMP
				lpi = olpi + sizeof(struct eth_header) + sizeof(struct ip_header) + 4;
				break;
			case 6: 
				lpi = olpi +
					sizeof(struct eth_header) + 
					sizeof(struct ip_header)  +
					sizeof(struct ports)      +
					sizeof(struct tcp_header);
				break;
			default:
				break;
			}
			return 1; // matched the header
		}
	}
}

//#define DUMP
#ifdef DUMP
/*
 * symbolic - produce a symbolic dump of the nfa
 */
static symbolic(char *s) 
{
	printf("pattern: %s\n", s);
	printf("nfacode:\n");
	nfadump(nfa);
}

static nfadump(NFA_t *ap)
{
	register int n;

	while (*ap != END)
		switch(*ap++) {
		case CLO:
			printf("CLOSURE");
			nfadump(ap);
			switch(*ap) {
			case CHR:
				n = CHRSKIP;
				break;
			case ANY:
				n = ANYSKIP;
				break;
			case CCL:
				n = CCLSKIP;
				break;
			}
			ap += n;
			break;
		case CHR:
			printf("\tCHR %c\n",*ap++);
			break;
		case ANY:
			printf("\tANY .\n");
			break;
		case BOL:
			printf("\tBOL -\n");
			break;
		case EOL:
			printf("\tEOL -\n");
			break;
		case BOT:
			printf("BOT: %d\n",*ap++);
			break;
		case EOT:
			printf("EOT: %d\n",*ap++);
			break;
		case BOW:
			printf("BOW\n");
			break;
		case EOW:
			printf("EOW\n");
			break;
		case REF:
			printf("REF: %d\n",*ap++);
			break;
		case CCL:
			printf("\tCCL [");
			for (n = 0; n < MAXCHR; n++)
				if (isinset(ap,(NFA_t)n)) {
					if (n < ' ')
						printf("^%c", n ^ 0x040);
					else
						printf("%c", n);
				}
			printf("]\n");
			ap += BITBLK;
			break;
		default:
			printf("bad nfa. opcode %o\n", ap[-1]);
			return;
			break;
		}
}
#endif
