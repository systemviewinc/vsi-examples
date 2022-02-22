#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "regex.h"
int  sta = NOP;               	/* status of lastpat */
int  tagstk[MAXTAG];             /* subpat tag stack..*/
NFA_t bittab[BITBLK];		/* bit table for CCL */
NFA_t nfa[MAXNFA];		/* automaton..       */

NFA_t get_token(char **p)
{
	char *sp = *p;
	if (strncmp(sp,"SMA",3) == 0) {
		sp += 3;
		*p = sp;
		return IP_SMA;
	}
	if (strncmp(sp,"DMA",3) == 0) {
		sp += 3;
		*p = sp;
		return IP_DMA;
	}
	if (strncmp(sp,"TYPE",4) == 0) {
		sp += 4;
		*p = sp;
		return IP_TYPE;
	}
	if (strncmp(sp,"SA",2) == 0) {
		sp += 2;
		*p = sp;
		return IP_SA;
	}
	if (strncmp(sp,"DA",2) == 0) {
		sp += 2;
		*p = sp;
		return IP_DA;
	}
	if (strncmp(sp,"SP",2) == 0) {
		sp += 2;
		*p = sp;
		return IP_SP;
	}
	if (strncmp(sp,"DP",2) == 0) {
		sp += 2;
		*p = sp;
		return IP_DP;
	}
	if (strncmp(sp,"IF",2) == 0) {
		sp += 2;
		*p = sp;
		return IP_FLAG;
	}
	if (strncmp(sp,"PROTO",5) == 0) {
		sp += 5;
		*p = sp;
		return IP_PROTO;
	}
	return 0;
}

void re_comp(char pat[512], NFA_t o_pat[1024])
{
        NFA_t *c_pat = (NFA_t*) &o_pat[0];
	register char *p;               /* pattern pointer   */
	register NFA_t *mp=c_pat;          /* nfa pointer       */
	register NFA_t *lp;              /* saved pointer..   */
	register NFA_t *sp=c_pat;          /* another one..     */

	register int tagi = 0;          /* tag stack index   */
	register int tagc = 1;          /* actual tag count  */
	unsigned int skip_n;
	register int n;
	register NFA_t mask;		/* xor mask -CCL/NCL */
	int c1, c2;
	if (!pat || !*pat)
		if (sta)
			return 0;
		else
			return badpat("No previous regular expression");
	sta = NOP;

	for (p = pat; *p; p++) {
		lp = mp;
		switch(*p) {
		case '{' :		/* Ethernet header */
			store(IP_HDR);
			p++;
			while (*p != '}') {
				while (isspace(*p)) p++;
				if (*p == '/') {
					NFA_t tok;
					int k;
					unsigned short ip_t;
					p++;
					tok = get_token(&p);
					if (*p == ':') {
						p++;
						switch (tok) {
						case IP_SMA:
						case IP_DMA:
							store(tok);
							// format xx:xx:xx:xx:xx:xx
							for (k = 0 ; k < 6; k++) {
								unsigned char c = (*p > '9') ? (*p &~ 0x20) - 'A' + 10 : (*p - '0');
								c <<= 4; p++;
								c |= (*p > '9') ? (*p &~ 0x20) - 'A' + 10 : (*p - '0');
								store(c);
								p++;
								if (*p != ':') break;
							}
							break;
						case IP_TYPE:
							store(tok);
							ip_t = strtoul(p,&p,0);
							store(ip_t & 0xff);
							store((ip_t>>8) & 0xff);
							break;
						case IP_SA:
						case IP_DA:
							store(tok);
							for (k = 0 ; k < 4 ; k++) {
								unsigned char a = strtoul(p,&p,0);
								store(a);
								if (*p != '.') break;
								p++;
							}
							break;
						case IP_SP:
						case IP_DP:
							store(tok);
							ip_t = strtoul(p,&p,0);
							store((ip_t>>8) & 0xff);
							store(ip_t & 0xff);
							break;
						case IP_FLAG:
							store(tok);
							ip_t = strtoul(p,&p,0);
							store(ip_t & 0xff);
							break;
						case IP_PROTO:
							store(tok);
							ip_t = strtoul(p,&p,0);
							store(ip_t & 0xff);
							break; 
						default:
							break;
						}
					};
				} else p++;
				while (isspace(*p)) p++;
			}
			break;
		case '.':               /* match any char..  */
			store(ANY);
			break;

		case '^':               /* match beginning.. */
			if (p == pat)
				store(BOL);
			else {
				store(CHR);
				store(*p);
			}
			break;

		case '$':               /* match endofline.. */
			if (!*(p+1))
				store(EOL);
			else {
				store(CHR);
				store(*p);
			}
			break;

		case '[':               /* match char class..*/
			store(CCL);

			if (*++p == '^') {
				mask = 0377;	
				p++;
			}
			else
				mask = 0;

			if (*p == '-')		/* real dash */
				chset(*p++);
			if (*p == ']')		/* real brac */
				chset(*p++);
			while (*p && *p != ']') {
				if (*p == '-' && *(p+1) && *(p+1) != ']') {
					p++;
					c1 = *(p-2) + 1;
					c2 = *p++;
					while (c1 <= c2)
						chset((NFA_t)c1++);
				}
				else if (*p == '\\' && *(p+1)) {
					p++;
					chset(*p++);
				}
				else
					chset(*p++);
			}
			if (!*p)
				return badpat("Missing ]");

			for (n = 0; n < BITBLK; bittab[n++] = (char) 0)
				store(mask ^ bittab[n]);
	
			break;

		case '*':               /* match 0 or more.. */
		case '+':               /* match 1 or more.. */
			if (p == pat)
				return badpat("Empty closure");
			lp = sp;		/* previous opcode */
			if (*lp == CLO)		/* equivalence..   */
				break;
			switch(*lp) {

			case BOL:
			case BOT:
			case EOT:
			case BOW:
			case EOW:
			case REF:
				return badpat("Illegal closure");
			default:
				break;
			}

			if (*p == '+')
				for (sp = mp; lp < sp; lp++)
					store(*lp);

			store(END);
			store(END);
			sp = mp;
			while (--mp > lp)
				*mp = mp[-1];
			store(CLO);
			mp = sp;
			break;

		case '\\':              /* tags, backrefs .. */
			switch(*++p) {

			case '(':
				if (tagc < MAXTAG) {
					tagstk[++tagi] = tagc;
					store(BOT);
					store(tagc++);
				}
				else
					return badpat("Too many \\(\\) pairs");
				break;
			case ')':
				if (*sp == BOT)
					return badpat("Null pattern inside \\(\\)");
				if (tagi > 0) {
					store(EOT);
					store(tagstk[tagi--]);
				}
				else
					return badpat("Unmatched \\)");
				break;
			case '<':
				store(BOW);
				break;
			case '>':
				if (*sp == BOW)
					return badpat("Null pattern inside \\<\\>");
				store(EOW);
				break;
			case '#':
				store(SKIP);
				p++;
				skip_n = strtoul(p,&p,0);
				store(skip_n);
				p--;
				break;
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				n = *p-'0';
				if (tagi > 0 && tagstk[tagi] == n)
					return badpat("Cyclical reference");
				if (tagc > n) {
					store(REF);
					store(n);
				}
				else
					return badpat("Undetermined reference");
				break;
			case 'b':
				store(CHR);
				store('\b');
				break;
			case 'n':
				store(CHR);
				store('\n');
				break;
			case 'f':
				store(CHR);
				store('\f');
				break;
			case 'r':
				store(CHR);
				store('\r');
				break;
			case 't':
				store(CHR);
				store('\t');
				break;
			default:
				store(CHR);
				store(*p);
			}
			break;

		default :               /* an ordinary char  */
			store(CHR);
			store(*p);
			break;
		}
		sp = lp;
	}
	if (tagi > 0)
		return badpat("Unmatched \\(");
	store(END);
	sta = OKP;
	
	return 0;
}


