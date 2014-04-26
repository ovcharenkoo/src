/*        Generated by TAPENADE     (INRIA, Tropics team)
    Tapenade 3.7 (r4786) - 21 Feb 2013 15:53
*/
#include "cstd.h"

/*
  Differentiation of acd_2d_4_d in reverse (adjoint) mode:
   gradient     of useful results: **ucd **upd **uc **up
   with respect to varying inputs: **ucd **upd **csq **uc **up
   RW status of diff variables: **ucd:incr **upd:in-out **csq:out
                **uc:incr **up:in-out
   Plus diff mem management of: ucd:in *ucd:in upd:in *upd:in
                csq:in *csq:in uc:in *uc:in up:in *up:in


  Differentiation of acd_2d_4 in forward (tangent) mode:
   variations   of useful results: **up
   with respect to varying inputs: **csq **uc **up
   RW status of diff variables: **csq:in **uc:in **up:in-out
   Plus diff mem management of: csq:in *csq:in uc:in *uc:in up:in
                *up:in
*/
void acd_2d_4_d_b(float **uc, float **ucb, float **ucd, float **ucdb, float **
        up, float **upb, float **upd, float **updb, float **csq, float **csqb,
        float **csqd, int *s, int *e, float c0, float *c1, float *c2, int *lbc
        , int *rbc) {
    int i0, i1;
    int branch;
    float tempb1;
    float tempb0;
    float tmpb;
    float tmp0b;
    float tempb;
    float tmp2b;
    float tmp1b;
    /* boundary conditions - note that uc[-1][i]=0 etc. */
    if (lbc[1])
        pushcontrol1b(0);
    else
        pushcontrol1b(1);
    if (rbc[1])
        pushcontrol1b(0);
    else
        pushcontrol1b(1);
    if (lbc[0])
        pushcontrol1b(0);
    else
        pushcontrol1b(1);
    if (rbc[0])
        for (i1 = e[1]; i1 > s[1]-1; --i1) {
            upb[i1][e[0]] = upb[i1][e[0]] - upb[i1][e[0] + 2];
            upb[i1][e[0] + 2] = 0.0;
            updb[i1][e[0]] = updb[i1][e[0]] - updb[i1][e[0] + 2];
            updb[i1][e[0] + 2] = 0.0;
        }
    popcontrol1b(&branch);
    if (branch == 0)
        for (i1 = e[1]; i1 > s[1]-1; --i1) {
            upb[i1][s[0]] = upb[i1][s[0]] - upb[i1][s[0] - 2];
            upb[i1][s[0] - 2] = 0.0;
            updb[i1][s[0]] = updb[i1][s[0]] - updb[i1][s[0] - 2];
            updb[i1][s[0] - 2] = 0.0;
        }
    popcontrol1b(&branch);
    if (branch == 0)
        for (i0 = e[0]; i0 > s[0]-1; --i0) {
            tmp2b = upb[e[1] + 2][i0];
            upb[e[1] + 2][i0] = 0.0;
            upb[e[1]][i0] = upb[e[1]][i0] - tmp2b;
            tmp1b = updb[e[1] + 2][i0];
            updb[e[1] + 2][i0] = 0.0;
            updb[e[1]][i0] = updb[e[1]][i0] - tmp1b;
        }
    popcontrol1b(&branch);
    if (branch == 0)
        for (i0 = e[0]; i0 > s[0]-1; --i0) {
            tmp0b = upb[s[1] - 2][i0];
            upb[s[1] - 2][i0] = 0.0;
            upb[s[1]][i0] = upb[s[1]][i0] - tmp0b;
            tmpb = updb[s[1] - 2][i0];
            updb[s[1] - 2][i0] = 0.0;
            updb[s[1]][i0] = updb[s[1]][i0] - tmpb;
        }
    //**csqb = 0.0;
    for (i1 = e[1]; i1 > s[1]-1; --i1)
        for (i0 = e[0]; i0 > s[0]-1; --i0) {
            tempb = csq[i1][i0]*upb[i1][i0];
            ucb[i1][i0] = ucb[i1][i0] + c0*tempb + 2.0*upb[i1][i0];
            csqb[i1][i0] = csqb[i1][i0] + (c0*uc[i1][i0]+c1[0]*(uc[i1][i0+1]+
                uc[i1][i0-1])+c1[1]*(uc[i1+1][i0]+uc[i1-1][i0])+c2[0]*(uc[i1][
                i0+2]+uc[i1][i0-2])+c2[1]*(uc[i1+2][i0]+uc[i1-2][i0]))*upb[i1]
                [i0];
            ucb[i1][i0 + 1] = ucb[i1][i0 + 1] + c1[0]*tempb;
            ucb[i1][i0 - 1] = ucb[i1][i0 - 1] + c1[0]*tempb;
            ucb[i1 + 1][i0] = ucb[i1 + 1][i0] + c1[1]*tempb;
            ucb[i1 - 1][i0] = ucb[i1 - 1][i0] + c1[1]*tempb;
            ucb[i1][i0 + 2] = ucb[i1][i0 + 2] + c2[0]*tempb;
            ucb[i1][i0 - 2] = ucb[i1][i0 - 2] + c2[0]*tempb;
            ucb[i1 + 2][i0] = ucb[i1 + 2][i0] + c2[1]*tempb;
            ucb[i1 - 2][i0] = ucb[i1 - 2][i0] + c2[1]*tempb;
            upb[i1][i0] = -upb[i1][i0];
            tempb0 = csqd[i1][i0]*updb[i1][i0];
            tempb1 = csq[i1][i0]*updb[i1][i0];
            ucdb[i1][i0] = ucdb[i1][i0] + c0*tempb1 + 2.0*updb[i1][i0];
            ucb[i1][i0] = ucb[i1][i0] + c0*tempb0;
            ucb[i1][i0 + 1] = ucb[i1][i0 + 1] + c1[0]*tempb0;
            ucb[i1][i0 - 1] = ucb[i1][i0 - 1] + c1[0]*tempb0;
            ucb[i1 + 1][i0] = ucb[i1 + 1][i0] + c1[1]*tempb0;
            ucb[i1 - 1][i0] = ucb[i1 - 1][i0] + c1[1]*tempb0;
            ucb[i1][i0 + 2] = ucb[i1][i0 + 2] + c2[0]*tempb0;
            ucb[i1][i0 - 2] = ucb[i1][i0 - 2] + c2[0]*tempb0;
            ucb[i1 + 2][i0] = ucb[i1 + 2][i0] + c2[1]*tempb0;
            ucb[i1 - 2][i0] = ucb[i1 - 2][i0] + c2[1]*tempb0;
            csqb[i1][i0] = csqb[i1][i0] + (c0*ucd[i1][i0]+c1[0]*(ucd[i1][i0+1]
                +ucd[i1][i0-1])+c1[1]*(ucd[i1+1][i0]+ucd[i1-1][i0])+c2[0]*(ucd
                [i1][i0+2]+ucd[i1][i0-2])+c2[1]*(ucd[i1+2][i0]+ucd[i1-2][i0]))
                *updb[i1][i0];
            ucdb[i1][i0 + 1] = ucdb[i1][i0 + 1] + c1[0]*tempb1;
            ucdb[i1][i0 - 1] = ucdb[i1][i0 - 1] + c1[0]*tempb1;
            ucdb[i1 + 1][i0] = ucdb[i1 + 1][i0] + c1[1]*tempb1;
            ucdb[i1 - 1][i0] = ucdb[i1 - 1][i0] + c1[1]*tempb1;
            ucdb[i1][i0 + 2] = ucdb[i1][i0 + 2] + c2[0]*tempb1;
            ucdb[i1][i0 - 2] = ucdb[i1][i0 - 2] + c2[0]*tempb1;
            ucdb[i1 + 2][i0] = ucdb[i1 + 2][i0] + c2[1]*tempb1;
            ucdb[i1 - 2][i0] = ucdb[i1 - 2][i0] + c2[1]*tempb1;
            updb[i1][i0] = -updb[i1][i0];
        }
}
