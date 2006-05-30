/*
 * Copyright (c) 2003 Matteo Frigo
 * Copyright (c) 2003 Massachusetts Institute of Technology
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/* This file was automatically generated --- DO NOT EDIT */
/* Generated on Sat Jul  5 21:41:14 EDT 2003 */

#include "codelet-dft.h"

/* Generated by: /homee/stevenj/cvs/fftw3.0.1/genfft/gen_notw_noinline_c -simd -compact -variables 4 -n 64 -name m2fv_64 -with-ostride 2 -include n2f.h */

/*
 * This function contains 456 FP additions, 124 FP multiplications,
 * (or, 404 additions, 72 multiplications, 52 fused multiply/add),
 * 105 stack variables, and 128 memory accesses
 */
/*
 * Generator Id's : 
 * $Id: m2fv_64.c,v 1.1 2005/07/26 17:36:59 glass Exp $
 * $Id: m2fv_64.c,v 1.1 2005/07/26 17:36:59 glass Exp $
 * $Id: m2fv_64.c,v 1.1 2005/07/26 17:36:59 glass Exp $
 */

#include "n2f.h"

static void m2fv_64_0(const R *xi, R *xo, stride is, int ivs, int ovs)
{
     DVK(KP471396736, +0.471396736825997648556387625905254377657460319);
     DVK(KP881921264, +0.881921264348355029712756863660388349508442621);
     DVK(KP098017140, +0.098017140329560601994195563888641845861136673);
     DVK(KP995184726, +0.995184726672196886244836953109479921575474869);
     DVK(KP290284677, +0.290284677254462367636192375817395274691476278);
     DVK(KP956940335, +0.956940335732208864935797886980269969482849206);
     DVK(KP634393284, +0.634393284163645498215171613225493370675687095);
     DVK(KP773010453, +0.773010453362736960810906609758469800971041293);
     DVK(KP555570233, +0.555570233019602224742830813948532874374937191);
     DVK(KP831469612, +0.831469612302545237078788377617905756738560812);
     DVK(KP980785280, +0.980785280403230449126182236134239036973933731);
     DVK(KP195090322, +0.195090322016128267848284868477022240927691618);
     DVK(KP382683432, +0.382683432365089771728459984030398866761344562);
     DVK(KP923879532, +0.923879532511286756128183189396788286822416626);
     DVK(KP707106781, +0.707106781186547524400844362104849039284835938);
     {
	  V T4p, T5q, Tb, T39, T2n, T3A, T6f, T6T, Tq, T3B, T6i, T76, T2i, T3a, T4w;
	  V T5r, TI, T2p, T6C, T6V, T3h, T3E, T4L, T5u, TZ, T2q, T6F, T6U, T3e, T3D;
	  V T4E, T5t, T23, T2N, T6t, T71, T6w, T72, T2c, T2O, T3t, T41, T5f, T5R, T5k;
	  V T5S, T3w, T42, T1s, T2K, T6m, T6Y, T6p, T6Z, T1B, T2L, T3m, T3Y, T4Y, T5O;
	  V T53, T5P, T3p, T3Z;
	  {
	       V T3, T4n, T2m, T4o, T6, T5p, T9, T5o;
	       {
		    V T1, T2, T2k, T2l;
		    T1 = LD(&(xi[0]), ivs, &(xi[0]));
		    T2 = LD(&(xi[WS(is, 32)]), ivs, &(xi[0]));
		    T3 = VSUB(T1, T2);
		    T4n = VADD(T1, T2);
		    T2k = LD(&(xi[WS(is, 16)]), ivs, &(xi[0]));
		    T2l = LD(&(xi[WS(is, 48)]), ivs, &(xi[0]));
		    T2m = VSUB(T2k, T2l);
		    T4o = VADD(T2k, T2l);
	       }
	       {
		    V T4, T5, T7, T8;
		    T4 = LD(&(xi[WS(is, 8)]), ivs, &(xi[0]));
		    T5 = LD(&(xi[WS(is, 40)]), ivs, &(xi[0]));
		    T6 = VSUB(T4, T5);
		    T5p = VADD(T4, T5);
		    T7 = LD(&(xi[WS(is, 56)]), ivs, &(xi[0]));
		    T8 = LD(&(xi[WS(is, 24)]), ivs, &(xi[0]));
		    T9 = VSUB(T7, T8);
		    T5o = VADD(T7, T8);
	       }
	       T4p = VSUB(T4n, T4o);
	       T5q = VSUB(T5o, T5p);
	       {
		    V Ta, T2j, T6d, T6e;
		    Ta = VMUL(LDK(KP707106781), VADD(T6, T9));
		    Tb = VADD(T3, Ta);
		    T39 = VSUB(T3, Ta);
		    T2j = VMUL(LDK(KP707106781), VSUB(T9, T6));
		    T2n = VSUB(T2j, T2m);
		    T3A = VADD(T2m, T2j);
		    T6d = VADD(T4n, T4o);
		    T6e = VADD(T5p, T5o);
		    T6f = VADD(T6d, T6e);
		    T6T = VSUB(T6d, T6e);
	       }
	  }
	  {
	       V Te, T4q, To, T4u, Th, T4r, Tl, T4t;
	       {
		    V Tc, Td, Tm, Tn;
		    Tc = LD(&(xi[WS(is, 4)]), ivs, &(xi[0]));
		    Td = LD(&(xi[WS(is, 36)]), ivs, &(xi[0]));
		    Te = VSUB(Tc, Td);
		    T4q = VADD(Tc, Td);
		    Tm = LD(&(xi[WS(is, 12)]), ivs, &(xi[0]));
		    Tn = LD(&(xi[WS(is, 44)]), ivs, &(xi[0]));
		    To = VSUB(Tm, Tn);
		    T4u = VADD(Tm, Tn);
	       }
	       {
		    V Tf, Tg, Tj, Tk;
		    Tf = LD(&(xi[WS(is, 20)]), ivs, &(xi[0]));
		    Tg = LD(&(xi[WS(is, 52)]), ivs, &(xi[0]));
		    Th = VSUB(Tf, Tg);
		    T4r = VADD(Tf, Tg);
		    Tj = LD(&(xi[WS(is, 60)]), ivs, &(xi[0]));
		    Tk = LD(&(xi[WS(is, 28)]), ivs, &(xi[0]));
		    Tl = VSUB(Tj, Tk);
		    T4t = VADD(Tj, Tk);
	       }
	       {
		    V Ti, Tp, T6g, T6h;
		    Ti = VFNMS(LDK(KP382683432), Th, VMUL(LDK(KP923879532), Te));
		    Tp = VFMA(LDK(KP923879532), Tl, VMUL(LDK(KP382683432), To));
		    Tq = VADD(Ti, Tp);
		    T3B = VSUB(Tp, Ti);
		    T6g = VADD(T4q, T4r);
		    T6h = VADD(T4t, T4u);
		    T6i = VADD(T6g, T6h);
		    T76 = VSUB(T6h, T6g);
	       }
	       {
		    V T2g, T2h, T4s, T4v;
		    T2g = VFNMS(LDK(KP923879532), To, VMUL(LDK(KP382683432), Tl));
		    T2h = VFMA(LDK(KP382683432), Te, VMUL(LDK(KP923879532), Th));
		    T2i = VSUB(T2g, T2h);
		    T3a = VADD(T2h, T2g);
		    T4s = VSUB(T4q, T4r);
		    T4v = VSUB(T4t, T4u);
		    T4w = VMUL(LDK(KP707106781), VADD(T4s, T4v));
		    T5r = VMUL(LDK(KP707106781), VSUB(T4v, T4s));
	       }
	  }
	  {
	       V Tu, T4F, TG, T4G, TB, T4J, TD, T4I;
	       {
		    V Ts, Tt, TE, TF;
		    Ts = LD(&(xi[WS(is, 62)]), ivs, &(xi[0]));
		    Tt = LD(&(xi[WS(is, 30)]), ivs, &(xi[0]));
		    Tu = VSUB(Ts, Tt);
		    T4F = VADD(Ts, Tt);
		    TE = LD(&(xi[WS(is, 14)]), ivs, &(xi[0]));
		    TF = LD(&(xi[WS(is, 46)]), ivs, &(xi[0]));
		    TG = VSUB(TE, TF);
		    T4G = VADD(TE, TF);
		    {
			 V Tv, Tw, Tx, Ty, Tz, TA;
			 Tv = LD(&(xi[WS(is, 6)]), ivs, &(xi[0]));
			 Tw = LD(&(xi[WS(is, 38)]), ivs, &(xi[0]));
			 Tx = VSUB(Tv, Tw);
			 Ty = LD(&(xi[WS(is, 54)]), ivs, &(xi[0]));
			 Tz = LD(&(xi[WS(is, 22)]), ivs, &(xi[0]));
			 TA = VSUB(Ty, Tz);
			 TB = VMUL(LDK(KP707106781), VADD(Tx, TA));
			 T4J = VADD(Tv, Tw);
			 TD = VMUL(LDK(KP707106781), VSUB(TA, Tx));
			 T4I = VADD(Ty, Tz);
		    }
	       }
	       {
		    V TC, TH, T6A, T6B;
		    TC = VADD(Tu, TB);
		    TH = VSUB(TD, TG);
		    TI = VFMA(LDK(KP195090322), TC, VMUL(LDK(KP980785280), TH));
		    T2p = VFNMS(LDK(KP195090322), TH, VMUL(LDK(KP980785280), TC));
		    T6A = VADD(T4F, T4G);
		    T6B = VADD(T4J, T4I);
		    T6C = VADD(T6A, T6B);
		    T6V = VSUB(T6A, T6B);
	       }
	       {
		    V T3f, T3g, T4H, T4K;
		    T3f = VSUB(Tu, TB);
		    T3g = VADD(TG, TD);
		    T3h = VFNMS(LDK(KP555570233), T3g, VMUL(LDK(KP831469612), T3f));
		    T3E = VFMA(LDK(KP555570233), T3f, VMUL(LDK(KP831469612), T3g));
		    T4H = VSUB(T4F, T4G);
		    T4K = VSUB(T4I, T4J);
		    T4L = VFNMS(LDK(KP382683432), T4K, VMUL(LDK(KP923879532), T4H));
		    T5u = VFMA(LDK(KP382683432), T4H, VMUL(LDK(KP923879532), T4K));
	       }
	  }
	  {
	       V TS, T4z, TW, T4y, TP, T4C, TX, T4B;
	       {
		    V TQ, TR, TU, TV;
		    TQ = LD(&(xi[WS(is, 18)]), ivs, &(xi[0]));
		    TR = LD(&(xi[WS(is, 50)]), ivs, &(xi[0]));
		    TS = VSUB(TQ, TR);
		    T4z = VADD(TQ, TR);
		    TU = LD(&(xi[WS(is, 2)]), ivs, &(xi[0]));
		    TV = LD(&(xi[WS(is, 34)]), ivs, &(xi[0]));
		    TW = VSUB(TU, TV);
		    T4y = VADD(TU, TV);
		    {
			 V TJ, TK, TL, TM, TN, TO;
			 TJ = LD(&(xi[WS(is, 58)]), ivs, &(xi[0]));
			 TK = LD(&(xi[WS(is, 26)]), ivs, &(xi[0]));
			 TL = VSUB(TJ, TK);
			 TM = LD(&(xi[WS(is, 10)]), ivs, &(xi[0]));
			 TN = LD(&(xi[WS(is, 42)]), ivs, &(xi[0]));
			 TO = VSUB(TM, TN);
			 TP = VMUL(LDK(KP707106781), VSUB(TL, TO));
			 T4C = VADD(TM, TN);
			 TX = VMUL(LDK(KP707106781), VADD(TO, TL));
			 T4B = VADD(TJ, TK);
		    }
	       }
	       {
		    V TT, TY, T6D, T6E;
		    TT = VSUB(TP, TS);
		    TY = VADD(TW, TX);
		    TZ = VFNMS(LDK(KP195090322), TY, VMUL(LDK(KP980785280), TT));
		    T2q = VFMA(LDK(KP980785280), TY, VMUL(LDK(KP195090322), TT));
		    T6D = VADD(T4y, T4z);
		    T6E = VADD(T4C, T4B);
		    T6F = VADD(T6D, T6E);
		    T6U = VSUB(T6D, T6E);
	       }
	       {
		    V T3c, T3d, T4A, T4D;
		    T3c = VSUB(TW, TX);
		    T3d = VADD(TS, TP);
		    T3e = VFMA(LDK(KP831469612), T3c, VMUL(LDK(KP555570233), T3d));
		    T3D = VFNMS(LDK(KP555570233), T3c, VMUL(LDK(KP831469612), T3d));
		    T4A = VSUB(T4y, T4z);
		    T4D = VSUB(T4B, T4C);
		    T4E = VFMA(LDK(KP923879532), T4A, VMUL(LDK(KP382683432), T4D));
		    T5t = VFNMS(LDK(KP382683432), T4A, VMUL(LDK(KP923879532), T4D));
	       }
	  }
	  {
	       V T1F, T55, T2a, T56, T1M, T5h, T27, T5g, T58, T59, T1U, T5a, T25, T5b, T5c;
	       V T21, T5d, T24;
	       {
		    V T1D, T1E, T28, T29;
		    T1D = LD(&(xi[WS(is, 63)]), ivs, &(xi[WS(is, 1)]));
		    T1E = LD(&(xi[WS(is, 31)]), ivs, &(xi[WS(is, 1)]));
		    T1F = VSUB(T1D, T1E);
		    T55 = VADD(T1D, T1E);
		    T28 = LD(&(xi[WS(is, 15)]), ivs, &(xi[WS(is, 1)]));
		    T29 = LD(&(xi[WS(is, 47)]), ivs, &(xi[WS(is, 1)]));
		    T2a = VSUB(T28, T29);
		    T56 = VADD(T28, T29);
	       }
	       {
		    V T1G, T1H, T1I, T1J, T1K, T1L;
		    T1G = LD(&(xi[WS(is, 7)]), ivs, &(xi[WS(is, 1)]));
		    T1H = LD(&(xi[WS(is, 39)]), ivs, &(xi[WS(is, 1)]));
		    T1I = VSUB(T1G, T1H);
		    T1J = LD(&(xi[WS(is, 55)]), ivs, &(xi[WS(is, 1)]));
		    T1K = LD(&(xi[WS(is, 23)]), ivs, &(xi[WS(is, 1)]));
		    T1L = VSUB(T1J, T1K);
		    T1M = VMUL(LDK(KP707106781), VADD(T1I, T1L));
		    T5h = VADD(T1G, T1H);
		    T27 = VMUL(LDK(KP707106781), VSUB(T1L, T1I));
		    T5g = VADD(T1J, T1K);
	       }
	       {
		    V T1Q, T1T, T1X, T20;
		    {
			 V T1O, T1P, T1R, T1S;
			 T1O = LD(&(xi[WS(is, 3)]), ivs, &(xi[WS(is, 1)]));
			 T1P = LD(&(xi[WS(is, 35)]), ivs, &(xi[WS(is, 1)]));
			 T1Q = VSUB(T1O, T1P);
			 T58 = VADD(T1O, T1P);
			 T1R = LD(&(xi[WS(is, 19)]), ivs, &(xi[WS(is, 1)]));
			 T1S = LD(&(xi[WS(is, 51)]), ivs, &(xi[WS(is, 1)]));
			 T1T = VSUB(T1R, T1S);
			 T59 = VADD(T1R, T1S);
		    }
		    T1U = VFNMS(LDK(KP382683432), T1T, VMUL(LDK(KP923879532), T1Q));
		    T5a = VSUB(T58, T59);
		    T25 = VFMA(LDK(KP382683432), T1Q, VMUL(LDK(KP923879532), T1T));
		    {
			 V T1V, T1W, T1Y, T1Z;
			 T1V = LD(&(xi[WS(is, 59)]), ivs, &(xi[WS(is, 1)]));
			 T1W = LD(&(xi[WS(is, 27)]), ivs, &(xi[WS(is, 1)]));
			 T1X = VSUB(T1V, T1W);
			 T5b = VADD(T1V, T1W);
			 T1Y = LD(&(xi[WS(is, 11)]), ivs, &(xi[WS(is, 1)]));
			 T1Z = LD(&(xi[WS(is, 43)]), ivs, &(xi[WS(is, 1)]));
			 T20 = VSUB(T1Y, T1Z);
			 T5c = VADD(T1Y, T1Z);
		    }
		    T21 = VFMA(LDK(KP923879532), T1X, VMUL(LDK(KP382683432), T20));
		    T5d = VSUB(T5b, T5c);
		    T24 = VFNMS(LDK(KP923879532), T20, VMUL(LDK(KP382683432), T1X));
	       }
	       {
		    V T1N, T22, T6r, T6s;
		    T1N = VADD(T1F, T1M);
		    T22 = VADD(T1U, T21);
		    T23 = VSUB(T1N, T22);
		    T2N = VADD(T1N, T22);
		    T6r = VADD(T55, T56);
		    T6s = VADD(T5h, T5g);
		    T6t = VADD(T6r, T6s);
		    T71 = VSUB(T6r, T6s);
	       }
	       {
		    V T6u, T6v, T26, T2b;
		    T6u = VADD(T58, T59);
		    T6v = VADD(T5b, T5c);
		    T6w = VADD(T6u, T6v);
		    T72 = VSUB(T6v, T6u);
		    T26 = VSUB(T24, T25);
		    T2b = VSUB(T27, T2a);
		    T2c = VSUB(T26, T2b);
		    T2O = VADD(T2b, T26);
	       }
	       {
		    V T3r, T3s, T57, T5e;
		    T3r = VSUB(T1F, T1M);
		    T3s = VADD(T25, T24);
		    T3t = VADD(T3r, T3s);
		    T41 = VSUB(T3r, T3s);
		    T57 = VSUB(T55, T56);
		    T5e = VMUL(LDK(KP707106781), VADD(T5a, T5d));
		    T5f = VADD(T57, T5e);
		    T5R = VSUB(T57, T5e);
	       }
	       {
		    V T5i, T5j, T3u, T3v;
		    T5i = VSUB(T5g, T5h);
		    T5j = VMUL(LDK(KP707106781), VSUB(T5d, T5a));
		    T5k = VADD(T5i, T5j);
		    T5S = VSUB(T5j, T5i);
		    T3u = VADD(T2a, T27);
		    T3v = VSUB(T21, T1U);
		    T3w = VADD(T3u, T3v);
		    T42 = VSUB(T3v, T3u);
	       }
	  }
	  {
	       V T1q, T4P, T1v, T4O, T1n, T50, T1w, T4Z, T4U, T4V, T18, T4W, T1z, T4R, T4S;
	       V T1f, T4T, T1y;
	       {
		    V T1o, T1p, T1t, T1u;
		    T1o = LD(&(xi[WS(is, 17)]), ivs, &(xi[WS(is, 1)]));
		    T1p = LD(&(xi[WS(is, 49)]), ivs, &(xi[WS(is, 1)]));
		    T1q = VSUB(T1o, T1p);
		    T4P = VADD(T1o, T1p);
		    T1t = LD(&(xi[WS(is, 1)]), ivs, &(xi[WS(is, 1)]));
		    T1u = LD(&(xi[WS(is, 33)]), ivs, &(xi[WS(is, 1)]));
		    T1v = VSUB(T1t, T1u);
		    T4O = VADD(T1t, T1u);
	       }
	       {
		    V T1h, T1i, T1j, T1k, T1l, T1m;
		    T1h = LD(&(xi[WS(is, 57)]), ivs, &(xi[WS(is, 1)]));
		    T1i = LD(&(xi[WS(is, 25)]), ivs, &(xi[WS(is, 1)]));
		    T1j = VSUB(T1h, T1i);
		    T1k = LD(&(xi[WS(is, 9)]), ivs, &(xi[WS(is, 1)]));
		    T1l = LD(&(xi[WS(is, 41)]), ivs, &(xi[WS(is, 1)]));
		    T1m = VSUB(T1k, T1l);
		    T1n = VMUL(LDK(KP707106781), VSUB(T1j, T1m));
		    T50 = VADD(T1k, T1l);
		    T1w = VMUL(LDK(KP707106781), VADD(T1m, T1j));
		    T4Z = VADD(T1h, T1i);
	       }
	       {
		    V T14, T17, T1b, T1e;
		    {
			 V T12, T13, T15, T16;
			 T12 = LD(&(xi[WS(is, 61)]), ivs, &(xi[WS(is, 1)]));
			 T13 = LD(&(xi[WS(is, 29)]), ivs, &(xi[WS(is, 1)]));
			 T14 = VSUB(T12, T13);
			 T4U = VADD(T12, T13);
			 T15 = LD(&(xi[WS(is, 13)]), ivs, &(xi[WS(is, 1)]));
			 T16 = LD(&(xi[WS(is, 45)]), ivs, &(xi[WS(is, 1)]));
			 T17 = VSUB(T15, T16);
			 T4V = VADD(T15, T16);
		    }
		    T18 = VFNMS(LDK(KP923879532), T17, VMUL(LDK(KP382683432), T14));
		    T4W = VSUB(T4U, T4V);
		    T1z = VFMA(LDK(KP923879532), T14, VMUL(LDK(KP382683432), T17));
		    {
			 V T19, T1a, T1c, T1d;
			 T19 = LD(&(xi[WS(is, 5)]), ivs, &(xi[WS(is, 1)]));
			 T1a = LD(&(xi[WS(is, 37)]), ivs, &(xi[WS(is, 1)]));
			 T1b = VSUB(T19, T1a);
			 T4R = VADD(T19, T1a);
			 T1c = LD(&(xi[WS(is, 21)]), ivs, &(xi[WS(is, 1)]));
			 T1d = LD(&(xi[WS(is, 53)]), ivs, &(xi[WS(is, 1)]));
			 T1e = VSUB(T1c, T1d);
			 T4S = VADD(T1c, T1d);
		    }
		    T1f = VFMA(LDK(KP382683432), T1b, VMUL(LDK(KP923879532), T1e));
		    T4T = VSUB(T4R, T4S);
		    T1y = VFNMS(LDK(KP382683432), T1e, VMUL(LDK(KP923879532), T1b));
	       }
	       {
		    V T1g, T1r, T6k, T6l;
		    T1g = VSUB(T18, T1f);
		    T1r = VSUB(T1n, T1q);
		    T1s = VSUB(T1g, T1r);
		    T2K = VADD(T1r, T1g);
		    T6k = VADD(T4O, T4P);
		    T6l = VADD(T50, T4Z);
		    T6m = VADD(T6k, T6l);
		    T6Y = VSUB(T6k, T6l);
	       }
	       {
		    V T6n, T6o, T1x, T1A;
		    T6n = VADD(T4R, T4S);
		    T6o = VADD(T4U, T4V);
		    T6p = VADD(T6n, T6o);
		    T6Z = VSUB(T6o, T6n);
		    T1x = VADD(T1v, T1w);
		    T1A = VADD(T1y, T1z);
		    T1B = VSUB(T1x, T1A);
		    T2L = VADD(T1x, T1A);
	       }
	       {
		    V T3k, T3l, T4Q, T4X;
		    T3k = VSUB(T1v, T1w);
		    T3l = VADD(T1f, T18);
		    T3m = VADD(T3k, T3l);
		    T3Y = VSUB(T3k, T3l);
		    T4Q = VSUB(T4O, T4P);
		    T4X = VMUL(LDK(KP707106781), VADD(T4T, T4W));
		    T4Y = VADD(T4Q, T4X);
		    T5O = VSUB(T4Q, T4X);
	       }
	       {
		    V T51, T52, T3n, T3o;
		    T51 = VSUB(T4Z, T50);
		    T52 = VMUL(LDK(KP707106781), VSUB(T4W, T4T));
		    T53 = VADD(T51, T52);
		    T5P = VSUB(T52, T51);
		    T3n = VADD(T1q, T1n);
		    T3o = VSUB(T1z, T1y);
		    T3p = VADD(T3n, T3o);
		    T3Z = VSUB(T3o, T3n);
	       }
	  }
	  {
	       V T6N, T6R, T6Q, T6S;
	       {
		    V T6L, T6M, T6O, T6P;
		    T6L = VADD(T6f, T6i);
		    T6M = VADD(T6F, T6C);
		    T6N = VADD(T6L, T6M);
		    T6R = VSUB(T6L, T6M);
		    T6O = VADD(T6m, T6p);
		    T6P = VADD(T6t, T6w);
		    T6Q = VADD(T6O, T6P);
		    T6S = VBYI(VSUB(T6P, T6O));
	       }
	       ST(&(xo[64]), VSUB(T6N, T6Q), ovs, &(xo[0]));
	       ST(&(xo[32]), VADD(T6R, T6S), ovs, &(xo[0]));
	       ST(&(xo[0]), VADD(T6N, T6Q), ovs, &(xo[0]));
	       ST(&(xo[96]), VSUB(T6R, T6S), ovs, &(xo[0]));
	  }
	  {
	       V T6j, T6G, T6y, T6H, T6q, T6x;
	       T6j = VSUB(T6f, T6i);
	       T6G = VSUB(T6C, T6F);
	       T6q = VSUB(T6m, T6p);
	       T6x = VSUB(T6t, T6w);
	       T6y = VMUL(LDK(KP707106781), VADD(T6q, T6x));
	       T6H = VMUL(LDK(KP707106781), VSUB(T6x, T6q));
	       {
		    V T6z, T6I, T6J, T6K;
		    T6z = VADD(T6j, T6y);
		    T6I = VBYI(VADD(T6G, T6H));
		    ST(&(xo[112]), VSUB(T6z, T6I), ovs, &(xo[0]));
		    ST(&(xo[16]), VADD(T6z, T6I), ovs, &(xo[0]));
		    T6J = VSUB(T6j, T6y);
		    T6K = VBYI(VSUB(T6H, T6G));
		    ST(&(xo[80]), VSUB(T6J, T6K), ovs, &(xo[0]));
		    ST(&(xo[48]), VADD(T6J, T6K), ovs, &(xo[0]));
	       }
	  }
	  {
	       V T6X, T7i, T78, T7g, T74, T7f, T7b, T7j, T6W, T77;
	       T6W = VMUL(LDK(KP707106781), VADD(T6U, T6V));
	       T6X = VADD(T6T, T6W);
	       T7i = VSUB(T6T, T6W);
	       T77 = VMUL(LDK(KP707106781), VSUB(T6V, T6U));
	       T78 = VADD(T76, T77);
	       T7g = VSUB(T77, T76);
	       {
		    V T70, T73, T79, T7a;
		    T70 = VFMA(LDK(KP923879532), T6Y, VMUL(LDK(KP382683432), T6Z));
		    T73 = VFNMS(LDK(KP382683432), T72, VMUL(LDK(KP923879532), T71));
		    T74 = VADD(T70, T73);
		    T7f = VSUB(T73, T70);
		    T79 = VFNMS(LDK(KP382683432), T6Y, VMUL(LDK(KP923879532), T6Z));
		    T7a = VFMA(LDK(KP382683432), T71, VMUL(LDK(KP923879532), T72));
		    T7b = VADD(T79, T7a);
		    T7j = VSUB(T7a, T79);
	       }
	       {
		    V T75, T7c, T7l, T7m;
		    T75 = VADD(T6X, T74);
		    T7c = VBYI(VADD(T78, T7b));
		    ST(&(xo[120]), VSUB(T75, T7c), ovs, &(xo[0]));
		    ST(&(xo[8]), VADD(T75, T7c), ovs, &(xo[0]));
		    T7l = VBYI(VADD(T7g, T7f));
		    T7m = VADD(T7i, T7j);
		    ST(&(xo[24]), VADD(T7l, T7m), ovs, &(xo[0]));
		    ST(&(xo[104]), VSUB(T7m, T7l), ovs, &(xo[0]));
	       }
	       {
		    V T7d, T7e, T7h, T7k;
		    T7d = VSUB(T6X, T74);
		    T7e = VBYI(VSUB(T7b, T78));
		    ST(&(xo[72]), VSUB(T7d, T7e), ovs, &(xo[0]));
		    ST(&(xo[56]), VADD(T7d, T7e), ovs, &(xo[0]));
		    T7h = VBYI(VSUB(T7f, T7g));
		    T7k = VSUB(T7i, T7j);
		    ST(&(xo[40]), VADD(T7h, T7k), ovs, &(xo[0]));
		    ST(&(xo[88]), VSUB(T7k, T7h), ovs, &(xo[0]));
	       }
	  }
	  {
	       V T5N, T68, T61, T69, T5U, T65, T5Y, T66;
	       {
		    V T5L, T5M, T5Z, T60;
		    T5L = VSUB(T4p, T4w);
		    T5M = VSUB(T5u, T5t);
		    T5N = VADD(T5L, T5M);
		    T68 = VSUB(T5L, T5M);
		    T5Z = VFNMS(LDK(KP555570233), T5O, VMUL(LDK(KP831469612), T5P));
		    T60 = VFMA(LDK(KP555570233), T5R, VMUL(LDK(KP831469612), T5S));
		    T61 = VADD(T5Z, T60);
		    T69 = VSUB(T60, T5Z);
	       }
	       {
		    V T5Q, T5T, T5W, T5X;
		    T5Q = VFMA(LDK(KP831469612), T5O, VMUL(LDK(KP555570233), T5P));
		    T5T = VFNMS(LDK(KP555570233), T5S, VMUL(LDK(KP831469612), T5R));
		    T5U = VADD(T5Q, T5T);
		    T65 = VSUB(T5T, T5Q);
		    T5W = VSUB(T5r, T5q);
		    T5X = VSUB(T4L, T4E);
		    T5Y = VADD(T5W, T5X);
		    T66 = VSUB(T5X, T5W);
	       }
	       {
		    V T5V, T62, T6b, T6c;
		    T5V = VADD(T5N, T5U);
		    T62 = VBYI(VADD(T5Y, T61));
		    ST(&(xo[116]), VSUB(T5V, T62), ovs, &(xo[0]));
		    ST(&(xo[12]), VADD(T5V, T62), ovs, &(xo[0]));
		    T6b = VBYI(VADD(T66, T65));
		    T6c = VADD(T68, T69);
		    ST(&(xo[20]), VADD(T6b, T6c), ovs, &(xo[0]));
		    ST(&(xo[108]), VSUB(T6c, T6b), ovs, &(xo[0]));
	       }
	       {
		    V T63, T64, T67, T6a;
		    T63 = VSUB(T5N, T5U);
		    T64 = VBYI(VSUB(T61, T5Y));
		    ST(&(xo[76]), VSUB(T63, T64), ovs, &(xo[0]));
		    ST(&(xo[52]), VADD(T63, T64), ovs, &(xo[0]));
		    T67 = VBYI(VSUB(T65, T66));
		    T6a = VSUB(T68, T69);
		    ST(&(xo[44]), VADD(T67, T6a), ovs, &(xo[0]));
		    ST(&(xo[84]), VSUB(T6a, T67), ovs, &(xo[0]));
	       }
	  }
	  {
	       V T11, T2C, T2v, T2D, T2e, T2z, T2s, T2A;
	       {
		    V Tr, T10, T2t, T2u;
		    Tr = VSUB(Tb, Tq);
		    T10 = VSUB(TI, TZ);
		    T11 = VADD(Tr, T10);
		    T2C = VSUB(Tr, T10);
		    T2t = VFNMS(LDK(KP634393284), T1B, VMUL(LDK(KP773010453), T1s));
		    T2u = VFMA(LDK(KP773010453), T2c, VMUL(LDK(KP634393284), T23));
		    T2v = VADD(T2t, T2u);
		    T2D = VSUB(T2u, T2t);
	       }
	       {
		    V T1C, T2d, T2o, T2r;
		    T1C = VFMA(LDK(KP634393284), T1s, VMUL(LDK(KP773010453), T1B));
		    T2d = VFNMS(LDK(KP634393284), T2c, VMUL(LDK(KP773010453), T23));
		    T2e = VADD(T1C, T2d);
		    T2z = VSUB(T2d, T1C);
		    T2o = VSUB(T2i, T2n);
		    T2r = VSUB(T2p, T2q);
		    T2s = VADD(T2o, T2r);
		    T2A = VSUB(T2r, T2o);
	       }
	       {
		    V T2f, T2w, T2F, T2G;
		    T2f = VADD(T11, T2e);
		    T2w = VBYI(VADD(T2s, T2v));
		    ST(&(xo[114]), VSUB(T2f, T2w), ovs, &(xo[2]));
		    ST(&(xo[14]), VADD(T2f, T2w), ovs, &(xo[2]));
		    T2F = VBYI(VADD(T2A, T2z));
		    T2G = VADD(T2C, T2D);
		    ST(&(xo[18]), VADD(T2F, T2G), ovs, &(xo[2]));
		    ST(&(xo[110]), VSUB(T2G, T2F), ovs, &(xo[2]));
	       }
	       {
		    V T2x, T2y, T2B, T2E;
		    T2x = VSUB(T11, T2e);
		    T2y = VBYI(VSUB(T2v, T2s));
		    ST(&(xo[78]), VSUB(T2x, T2y), ovs, &(xo[2]));
		    ST(&(xo[50]), VADD(T2x, T2y), ovs, &(xo[2]));
		    T2B = VBYI(VSUB(T2z, T2A));
		    T2E = VSUB(T2C, T2D);
		    ST(&(xo[46]), VADD(T2B, T2E), ovs, &(xo[2]));
		    ST(&(xo[82]), VSUB(T2E, T2B), ovs, &(xo[2]));
	       }
	  }
	  {
	       V T3j, T3Q, T3J, T3R, T3y, T3N, T3G, T3O;
	       {
		    V T3b, T3i, T3H, T3I;
		    T3b = VADD(T39, T3a);
		    T3i = VADD(T3e, T3h);
		    T3j = VADD(T3b, T3i);
		    T3Q = VSUB(T3b, T3i);
		    T3H = VFNMS(LDK(KP290284677), T3m, VMUL(LDK(KP956940335), T3p));
		    T3I = VFMA(LDK(KP290284677), T3t, VMUL(LDK(KP956940335), T3w));
		    T3J = VADD(T3H, T3I);
		    T3R = VSUB(T3I, T3H);
	       }
	       {
		    V T3q, T3x, T3C, T3F;
		    T3q = VFMA(LDK(KP956940335), T3m, VMUL(LDK(KP290284677), T3p));
		    T3x = VFNMS(LDK(KP290284677), T3w, VMUL(LDK(KP956940335), T3t));
		    T3y = VADD(T3q, T3x);
		    T3N = VSUB(T3x, T3q);
		    T3C = VADD(T3A, T3B);
		    T3F = VADD(T3D, T3E);
		    T3G = VADD(T3C, T3F);
		    T3O = VSUB(T3F, T3C);
	       }
	       {
		    V T3z, T3K, T3T, T3U;
		    T3z = VADD(T3j, T3y);
		    T3K = VBYI(VADD(T3G, T3J));
		    ST(&(xo[122]), VSUB(T3z, T3K), ovs, &(xo[2]));
		    ST(&(xo[6]), VADD(T3z, T3K), ovs, &(xo[2]));
		    T3T = VBYI(VADD(T3O, T3N));
		    T3U = VADD(T3Q, T3R);
		    ST(&(xo[26]), VADD(T3T, T3U), ovs, &(xo[2]));
		    ST(&(xo[102]), VSUB(T3U, T3T), ovs, &(xo[2]));
	       }
	       {
		    V T3L, T3M, T3P, T3S;
		    T3L = VSUB(T3j, T3y);
		    T3M = VBYI(VSUB(T3J, T3G));
		    ST(&(xo[70]), VSUB(T3L, T3M), ovs, &(xo[2]));
		    ST(&(xo[58]), VADD(T3L, T3M), ovs, &(xo[2]));
		    T3P = VBYI(VSUB(T3N, T3O));
		    T3S = VSUB(T3Q, T3R);
		    ST(&(xo[38]), VADD(T3P, T3S), ovs, &(xo[2]));
		    ST(&(xo[90]), VSUB(T3S, T3P), ovs, &(xo[2]));
	       }
	  }
	  {
	       V T4N, T5G, T5z, T5H, T5m, T5D, T5w, T5E;
	       {
		    V T4x, T4M, T5x, T5y;
		    T4x = VADD(T4p, T4w);
		    T4M = VADD(T4E, T4L);
		    T4N = VADD(T4x, T4M);
		    T5G = VSUB(T4x, T4M);
		    T5x = VFNMS(LDK(KP195090322), T4Y, VMUL(LDK(KP980785280), T53));
		    T5y = VFMA(LDK(KP195090322), T5f, VMUL(LDK(KP980785280), T5k));
		    T5z = VADD(T5x, T5y);
		    T5H = VSUB(T5y, T5x);
	       }
	       {
		    V T54, T5l, T5s, T5v;
		    T54 = VFMA(LDK(KP980785280), T4Y, VMUL(LDK(KP195090322), T53));
		    T5l = VFNMS(LDK(KP195090322), T5k, VMUL(LDK(KP980785280), T5f));
		    T5m = VADD(T54, T5l);
		    T5D = VSUB(T5l, T54);
		    T5s = VADD(T5q, T5r);
		    T5v = VADD(T5t, T5u);
		    T5w = VADD(T5s, T5v);
		    T5E = VSUB(T5v, T5s);
	       }
	       {
		    V T5n, T5A, T5J, T5K;
		    T5n = VADD(T4N, T5m);
		    T5A = VBYI(VADD(T5w, T5z));
		    ST(&(xo[124]), VSUB(T5n, T5A), ovs, &(xo[0]));
		    ST(&(xo[4]), VADD(T5n, T5A), ovs, &(xo[0]));
		    T5J = VBYI(VADD(T5E, T5D));
		    T5K = VADD(T5G, T5H);
		    ST(&(xo[28]), VADD(T5J, T5K), ovs, &(xo[0]));
		    ST(&(xo[100]), VSUB(T5K, T5J), ovs, &(xo[0]));
	       }
	       {
		    V T5B, T5C, T5F, T5I;
		    T5B = VSUB(T4N, T5m);
		    T5C = VBYI(VSUB(T5z, T5w));
		    ST(&(xo[68]), VSUB(T5B, T5C), ovs, &(xo[0]));
		    ST(&(xo[60]), VADD(T5B, T5C), ovs, &(xo[0]));
		    T5F = VBYI(VSUB(T5D, T5E));
		    T5I = VSUB(T5G, T5H);
		    ST(&(xo[36]), VADD(T5F, T5I), ovs, &(xo[0]));
		    ST(&(xo[92]), VSUB(T5I, T5F), ovs, &(xo[0]));
	       }
	  }
	  {
	       V T2J, T34, T2X, T35, T2Q, T31, T2U, T32;
	       {
		    V T2H, T2I, T2V, T2W;
		    T2H = VADD(Tb, Tq);
		    T2I = VADD(T2q, T2p);
		    T2J = VADD(T2H, T2I);
		    T34 = VSUB(T2H, T2I);
		    T2V = VFNMS(LDK(KP098017140), T2L, VMUL(LDK(KP995184726), T2K));
		    T2W = VFMA(LDK(KP995184726), T2O, VMUL(LDK(KP098017140), T2N));
		    T2X = VADD(T2V, T2W);
		    T35 = VSUB(T2W, T2V);
	       }
	       {
		    V T2M, T2P, T2S, T2T;
		    T2M = VFMA(LDK(KP098017140), T2K, VMUL(LDK(KP995184726), T2L));
		    T2P = VFNMS(LDK(KP098017140), T2O, VMUL(LDK(KP995184726), T2N));
		    T2Q = VADD(T2M, T2P);
		    T31 = VSUB(T2P, T2M);
		    T2S = VADD(T2n, T2i);
		    T2T = VADD(TZ, TI);
		    T2U = VADD(T2S, T2T);
		    T32 = VSUB(T2T, T2S);
	       }
	       {
		    V T2R, T2Y, T37, T38;
		    T2R = VADD(T2J, T2Q);
		    T2Y = VBYI(VADD(T2U, T2X));
		    ST(&(xo[126]), VSUB(T2R, T2Y), ovs, &(xo[2]));
		    ST(&(xo[2]), VADD(T2R, T2Y), ovs, &(xo[2]));
		    T37 = VBYI(VADD(T32, T31));
		    T38 = VADD(T34, T35);
		    ST(&(xo[30]), VADD(T37, T38), ovs, &(xo[2]));
		    ST(&(xo[98]), VSUB(T38, T37), ovs, &(xo[2]));
	       }
	       {
		    V T2Z, T30, T33, T36;
		    T2Z = VSUB(T2J, T2Q);
		    T30 = VBYI(VSUB(T2X, T2U));
		    ST(&(xo[66]), VSUB(T2Z, T30), ovs, &(xo[2]));
		    ST(&(xo[62]), VADD(T2Z, T30), ovs, &(xo[2]));
		    T33 = VBYI(VSUB(T31, T32));
		    T36 = VSUB(T34, T35);
		    ST(&(xo[34]), VADD(T33, T36), ovs, &(xo[2]));
		    ST(&(xo[94]), VSUB(T36, T33), ovs, &(xo[2]));
	       }
	  }
	  {
	       V T3X, T4i, T4b, T4j, T44, T4f, T48, T4g;
	       {
		    V T3V, T3W, T49, T4a;
		    T3V = VSUB(T39, T3a);
		    T3W = VSUB(T3E, T3D);
		    T3X = VADD(T3V, T3W);
		    T4i = VSUB(T3V, T3W);
		    T49 = VFNMS(LDK(KP471396736), T3Y, VMUL(LDK(KP881921264), T3Z));
		    T4a = VFMA(LDK(KP471396736), T41, VMUL(LDK(KP881921264), T42));
		    T4b = VADD(T49, T4a);
		    T4j = VSUB(T4a, T49);
	       }
	       {
		    V T40, T43, T46, T47;
		    T40 = VFMA(LDK(KP881921264), T3Y, VMUL(LDK(KP471396736), T3Z));
		    T43 = VFNMS(LDK(KP471396736), T42, VMUL(LDK(KP881921264), T41));
		    T44 = VADD(T40, T43);
		    T4f = VSUB(T43, T40);
		    T46 = VSUB(T3B, T3A);
		    T47 = VSUB(T3h, T3e);
		    T48 = VADD(T46, T47);
		    T4g = VSUB(T47, T46);
	       }
	       {
		    V T45, T4c, T4l, T4m;
		    T45 = VADD(T3X, T44);
		    T4c = VBYI(VADD(T48, T4b));
		    ST(&(xo[118]), VSUB(T45, T4c), ovs, &(xo[2]));
		    ST(&(xo[10]), VADD(T45, T4c), ovs, &(xo[2]));
		    T4l = VBYI(VADD(T4g, T4f));
		    T4m = VADD(T4i, T4j);
		    ST(&(xo[22]), VADD(T4l, T4m), ovs, &(xo[2]));
		    ST(&(xo[106]), VSUB(T4m, T4l), ovs, &(xo[2]));
	       }
	       {
		    V T4d, T4e, T4h, T4k;
		    T4d = VSUB(T3X, T44);
		    T4e = VBYI(VSUB(T4b, T48));
		    ST(&(xo[74]), VSUB(T4d, T4e), ovs, &(xo[2]));
		    ST(&(xo[54]), VADD(T4d, T4e), ovs, &(xo[2]));
		    T4h = VBYI(VSUB(T4f, T4g));
		    T4k = VSUB(T4i, T4j);
		    ST(&(xo[42]), VADD(T4h, T4k), ovs, &(xo[2]));
		    ST(&(xo[86]), VSUB(T4k, T4h), ovs, &(xo[2]));
	       }
	  }
     }
}

static void m2fv_64(const R *ri, const R *ii, R *ro, R *io, stride is, stride os, int v, int ivs, int ovs)
{
     int i;
     BEGIN_SIMD();
     for (i = 0; i < v; i += VL) {
	  m2fv_64_0(ri, ro, is, ivs, ovs);
	  ri += VL * ivs;
	  ro += VL * ovs;
     }
     END_SIMD();
}

static const kdft_desc desc = { 64, "m2fv_64", {404, 72, 52, 0}, &GENUS, 0, 2, 0, 0 };
void X(codelet_m2fv_64) (planner *p) {
     X(kdft_register) (p, m2fv_64, &desc);
}
