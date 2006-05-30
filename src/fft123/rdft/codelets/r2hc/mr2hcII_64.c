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
/* Generated on Sat Jul  5 21:59:51 EDT 2003 */

#include "codelet-rdft.h"

/* Generated by: /homee/stevenj/cvs/fftw3.0.1/genfft/gen_r2hc_noinline -compact -variables 4 -n 64 -name mr2hcII_64 -dft-II -include r2hcII.h */

/*
 * This function contains 434 FP additions, 206 FP multiplications,
 * (or, 342 additions, 114 multiplications, 92 fused multiply/add),
 * 117 stack variables, and 128 memory accesses
 */
/*
 * Generator Id's : 
 * $Id: mr2hcII_64.c,v 1.1 2005/07/26 17:37:08 glass Exp $
 * $Id: mr2hcII_64.c,v 1.1 2005/07/26 17:37:08 glass Exp $
 * $Id: mr2hcII_64.c,v 1.1 2005/07/26 17:37:08 glass Exp $
 */

#include "r2hcII.h"

static void mr2hcII_64_0(const R *I, R *ro, R *io, stride is, stride ros, stride ios)
{
     DK(KP242980179, +0.242980179903263889948274162077471118320990783);
     DK(KP970031253, +0.970031253194543992603984207286100251456865962);
     DK(KP857728610, +0.857728610000272069902269984284770137042490799);
     DK(KP514102744, +0.514102744193221726593693838968815772608049120);
     DK(KP471396736, +0.471396736825997648556387625905254377657460319);
     DK(KP881921264, +0.881921264348355029712756863660388349508442621);
     DK(KP427555093, +0.427555093430282094320966856888798534304578629);
     DK(KP903989293, +0.903989293123443331586200297230537048710132025);
     DK(KP336889853, +0.336889853392220050689253212619147570477766780);
     DK(KP941544065, +0.941544065183020778412509402599502357185589796);
     DK(KP773010453, +0.773010453362736960810906609758469800971041293);
     DK(KP634393284, +0.634393284163645498215171613225493370675687095);
     DK(KP595699304, +0.595699304492433343467036528829969889511926338);
     DK(KP803207531, +0.803207531480644909806676512963141923879569427);
     DK(KP146730474, +0.146730474455361751658850129646717819706215317);
     DK(KP989176509, +0.989176509964780973451673738016243063983689533);
     DK(KP956940335, +0.956940335732208864935797886980269969482849206);
     DK(KP290284677, +0.290284677254462367636192375817395274691476278);
     DK(KP049067674, +0.049067674327418014254954976942682658314745363);
     DK(KP998795456, +0.998795456205172392714771604759100694443203615);
     DK(KP671558954, +0.671558954847018400625376850427421803228750632);
     DK(KP740951125, +0.740951125354959091175616897495162729728955309);
     DK(KP098017140, +0.098017140329560601994195563888641845861136673);
     DK(KP995184726, +0.995184726672196886244836953109479921575474869);
     DK(KP382683432, +0.382683432365089771728459984030398866761344562);
     DK(KP923879532, +0.923879532511286756128183189396788286822416626);
     DK(KP555570233, +0.555570233019602224742830813948532874374937191);
     DK(KP831469612, +0.831469612302545237078788377617905756738560812);
     DK(KP195090322, +0.195090322016128267848284868477022240927691618);
     DK(KP980785280, +0.980785280403230449126182236134239036973933731);
     DK(KP707106781, +0.707106781186547524400844362104849039284835938);
     {
	  E Tm, T34, T3Z, T5g, Tv, T35, T3W, T5h, Td, T33, T6B, T6Q, T3T, T5f, T68;
	  E T6m, T2b, T3n, T4O, T5D, T2F, T3r, T4K, T5z, TK, T3c, T47, T5n, TR, T3b;
	  E T44, T5o, T15, T38, T4e, T5l, T1c, T39, T4b, T5k, T1s, T3g, T4v, T5w, T1W;
	  E T3k, T4k, T5s, T2u, T3q, T4R, T5A, T2y, T3o, T4H, T5C, T1L, T3j, T4y, T5t;
	  E T1P, T3h, T4r, T5v;
	  {
	       E Te, Tk, Th, Tj, Tf, Tg;
	       Te = I[WS(is, 4)];
	       Tk = I[WS(is, 36)];
	       Tf = I[WS(is, 20)];
	       Tg = I[WS(is, 52)];
	       Th = KP707106781 * (Tf - Tg);
	       Tj = KP707106781 * (Tf + Tg);
	       {
		    E Ti, Tl, T3X, T3Y;
		    Ti = Te + Th;
		    Tl = Tj + Tk;
		    Tm = FNMS(KP195090322, Tl, KP980785280 * Ti);
		    T34 = FMA(KP195090322, Ti, KP980785280 * Tl);
		    T3X = Tk - Tj;
		    T3Y = Te - Th;
		    T3Z = FNMS(KP555570233, T3Y, KP831469612 * T3X);
		    T5g = FMA(KP831469612, T3Y, KP555570233 * T3X);
	       }
	  }
	  {
	       E Tq, Tt, Tp, Ts, Tn, To;
	       Tq = I[WS(is, 60)];
	       Tt = I[WS(is, 28)];
	       Tn = I[WS(is, 12)];
	       To = I[WS(is, 44)];
	       Tp = KP707106781 * (Tn - To);
	       Ts = KP707106781 * (Tn + To);
	       {
		    E Tr, Tu, T3U, T3V;
		    Tr = Tp - Tq;
		    Tu = Ts + Tt;
		    Tv = FMA(KP980785280, Tr, KP195090322 * Tu);
		    T35 = FNMS(KP980785280, Tu, KP195090322 * Tr);
		    T3U = Tt - Ts;
		    T3V = Tp + Tq;
		    T3W = FNMS(KP555570233, T3V, KP831469612 * T3U);
		    T5h = FMA(KP831469612, T3V, KP555570233 * T3U);
	       }
	  }
	  {
	       E T1, T66, T4, T65, T8, T3Q, Tb, T3R, T2, T3;
	       T1 = I[0];
	       T66 = I[WS(is, 32)];
	       T2 = I[WS(is, 16)];
	       T3 = I[WS(is, 48)];
	       T4 = KP707106781 * (T2 - T3);
	       T65 = KP707106781 * (T2 + T3);
	       {
		    E T6, T7, T9, Ta;
		    T6 = I[WS(is, 8)];
		    T7 = I[WS(is, 40)];
		    T8 = FNMS(KP382683432, T7, KP923879532 * T6);
		    T3Q = FMA(KP382683432, T6, KP923879532 * T7);
		    T9 = I[WS(is, 24)];
		    Ta = I[WS(is, 56)];
		    Tb = FNMS(KP923879532, Ta, KP382683432 * T9);
		    T3R = FMA(KP923879532, T9, KP382683432 * Ta);
	       }
	       {
		    E T5, Tc, T6z, T6A;
		    T5 = T1 + T4;
		    Tc = T8 + Tb;
		    Td = T5 + Tc;
		    T33 = T5 - Tc;
		    T6z = Tb - T8;
		    T6A = T66 - T65;
		    T6B = T6z - T6A;
		    T6Q = T6z + T6A;
	       }
	       {
		    E T3P, T3S, T64, T67;
		    T3P = T1 - T4;
		    T3S = T3Q - T3R;
		    T3T = T3P - T3S;
		    T5f = T3P + T3S;
		    T64 = T3Q + T3R;
		    T67 = T65 + T66;
		    T68 = T64 + T67;
		    T6m = T67 - T64;
	       }
	  }
	  {
	       E T22, T2D, T21, T2C, T26, T2z, T29, T2A, T1Z, T20;
	       T22 = I[WS(is, 63)];
	       T2D = I[WS(is, 31)];
	       T1Z = I[WS(is, 15)];
	       T20 = I[WS(is, 47)];
	       T21 = KP707106781 * (T1Z - T20);
	       T2C = KP707106781 * (T1Z + T20);
	       {
		    E T24, T25, T27, T28;
		    T24 = I[WS(is, 7)];
		    T25 = I[WS(is, 39)];
		    T26 = FNMS(KP382683432, T25, KP923879532 * T24);
		    T2z = FMA(KP382683432, T24, KP923879532 * T25);
		    T27 = I[WS(is, 23)];
		    T28 = I[WS(is, 55)];
		    T29 = FNMS(KP923879532, T28, KP382683432 * T27);
		    T2A = FMA(KP923879532, T27, KP382683432 * T28);
	       }
	       {
		    E T23, T2a, T4M, T4N;
		    T23 = T21 - T22;
		    T2a = T26 + T29;
		    T2b = T23 + T2a;
		    T3n = T23 - T2a;
		    T4M = T29 - T26;
		    T4N = T2D - T2C;
		    T4O = T4M - T4N;
		    T5D = T4M + T4N;
	       }
	       {
		    E T2B, T2E, T4I, T4J;
		    T2B = T2z + T2A;
		    T2E = T2C + T2D;
		    T2F = T2B + T2E;
		    T3r = T2E - T2B;
		    T4I = T21 + T22;
		    T4J = T2z - T2A;
		    T4K = T4I + T4J;
		    T5z = T4J - T4I;
	       }
	  }
	  {
	       E Ty, TP, TB, TO, TF, TL, TI, TM, Tz, TA;
	       Ty = I[WS(is, 2)];
	       TP = I[WS(is, 34)];
	       Tz = I[WS(is, 18)];
	       TA = I[WS(is, 50)];
	       TB = KP707106781 * (Tz - TA);
	       TO = KP707106781 * (Tz + TA);
	       {
		    E TD, TE, TG, TH;
		    TD = I[WS(is, 10)];
		    TE = I[WS(is, 42)];
		    TF = FNMS(KP382683432, TE, KP923879532 * TD);
		    TL = FMA(KP382683432, TD, KP923879532 * TE);
		    TG = I[WS(is, 26)];
		    TH = I[WS(is, 58)];
		    TI = FNMS(KP923879532, TH, KP382683432 * TG);
		    TM = FMA(KP923879532, TG, KP382683432 * TH);
	       }
	       {
		    E TC, TJ, T45, T46;
		    TC = Ty + TB;
		    TJ = TF + TI;
		    TK = TC + TJ;
		    T3c = TC - TJ;
		    T45 = TI - TF;
		    T46 = TP - TO;
		    T47 = T45 - T46;
		    T5n = T45 + T46;
	       }
	       {
		    E TN, TQ, T42, T43;
		    TN = TL + TM;
		    TQ = TO + TP;
		    TR = TN + TQ;
		    T3b = TQ - TN;
		    T42 = Ty - TB;
		    T43 = TL - TM;
		    T44 = T42 - T43;
		    T5o = T42 + T43;
	       }
	  }
	  {
	       E TW, T1a, TV, T19, T10, T16, T13, T17, TT, TU;
	       TW = I[WS(is, 62)];
	       T1a = I[WS(is, 30)];
	       TT = I[WS(is, 14)];
	       TU = I[WS(is, 46)];
	       TV = KP707106781 * (TT - TU);
	       T19 = KP707106781 * (TT + TU);
	       {
		    E TY, TZ, T11, T12;
		    TY = I[WS(is, 6)];
		    TZ = I[WS(is, 38)];
		    T10 = FNMS(KP382683432, TZ, KP923879532 * TY);
		    T16 = FMA(KP382683432, TY, KP923879532 * TZ);
		    T11 = I[WS(is, 22)];
		    T12 = I[WS(is, 54)];
		    T13 = FNMS(KP923879532, T12, KP382683432 * T11);
		    T17 = FMA(KP923879532, T11, KP382683432 * T12);
	       }
	       {
		    E TX, T14, T4c, T4d;
		    TX = TV - TW;
		    T14 = T10 + T13;
		    T15 = TX + T14;
		    T38 = TX - T14;
		    T4c = T13 - T10;
		    T4d = T1a - T19;
		    T4e = T4c - T4d;
		    T5l = T4c + T4d;
	       }
	       {
		    E T18, T1b, T49, T4a;
		    T18 = T16 + T17;
		    T1b = T19 + T1a;
		    T1c = T18 + T1b;
		    T39 = T1b - T18;
		    T49 = TV + TW;
		    T4a = T16 - T17;
		    T4b = T49 + T4a;
		    T5k = T4a - T49;
	       }
	  }
	  {
	       E T1g, T1U, T1j, T1T, T1n, T1Q, T1q, T1R, T1h, T1i;
	       T1g = I[WS(is, 1)];
	       T1U = I[WS(is, 33)];
	       T1h = I[WS(is, 17)];
	       T1i = I[WS(is, 49)];
	       T1j = KP707106781 * (T1h - T1i);
	       T1T = KP707106781 * (T1h + T1i);
	       {
		    E T1l, T1m, T1o, T1p;
		    T1l = I[WS(is, 9)];
		    T1m = I[WS(is, 41)];
		    T1n = FNMS(KP382683432, T1m, KP923879532 * T1l);
		    T1Q = FMA(KP382683432, T1l, KP923879532 * T1m);
		    T1o = I[WS(is, 25)];
		    T1p = I[WS(is, 57)];
		    T1q = FNMS(KP923879532, T1p, KP382683432 * T1o);
		    T1R = FMA(KP923879532, T1o, KP382683432 * T1p);
	       }
	       {
		    E T1k, T1r, T4t, T4u;
		    T1k = T1g + T1j;
		    T1r = T1n + T1q;
		    T1s = T1k + T1r;
		    T3g = T1k - T1r;
		    T4t = T1q - T1n;
		    T4u = T1U - T1T;
		    T4v = T4t - T4u;
		    T5w = T4t + T4u;
	       }
	       {
		    E T1S, T1V, T4i, T4j;
		    T1S = T1Q + T1R;
		    T1V = T1T + T1U;
		    T1W = T1S + T1V;
		    T3k = T1V - T1S;
		    T4i = T1g - T1j;
		    T4j = T1Q - T1R;
		    T4k = T4i - T4j;
		    T5s = T4i + T4j;
	       }
	  }
	  {
	       E T2g, T4F, T2j, T4E, T2p, T4C, T2s, T4B;
	       {
		    E T2c, T2i, T2f, T2h, T2d, T2e;
		    T2c = I[WS(is, 3)];
		    T2i = I[WS(is, 35)];
		    T2d = I[WS(is, 19)];
		    T2e = I[WS(is, 51)];
		    T2f = KP707106781 * (T2d - T2e);
		    T2h = KP707106781 * (T2d + T2e);
		    T2g = T2c + T2f;
		    T4F = T2c - T2f;
		    T2j = T2h + T2i;
		    T4E = T2i - T2h;
	       }
	       {
		    E T2o, T2r, T2n, T2q, T2l, T2m;
		    T2o = I[WS(is, 59)];
		    T2r = I[WS(is, 27)];
		    T2l = I[WS(is, 11)];
		    T2m = I[WS(is, 43)];
		    T2n = KP707106781 * (T2l - T2m);
		    T2q = KP707106781 * (T2l + T2m);
		    T2p = T2n - T2o;
		    T4C = T2n + T2o;
		    T2s = T2q + T2r;
		    T4B = T2r - T2q;
	       }
	       {
		    E T2k, T2t, T4P, T4Q;
		    T2k = FNMS(KP195090322, T2j, KP980785280 * T2g);
		    T2t = FMA(KP980785280, T2p, KP195090322 * T2s);
		    T2u = T2k + T2t;
		    T3q = T2t - T2k;
		    T4P = FMA(KP831469612, T4F, KP555570233 * T4E);
		    T4Q = FMA(KP831469612, T4C, KP555570233 * T4B);
		    T4R = T4P + T4Q;
		    T5A = T4P - T4Q;
	       }
	       {
		    E T2w, T2x, T4D, T4G;
		    T2w = FNMS(KP980785280, T2s, KP195090322 * T2p);
		    T2x = FMA(KP195090322, T2g, KP980785280 * T2j);
		    T2y = T2w - T2x;
		    T3o = T2x + T2w;
		    T4D = FNMS(KP555570233, T4C, KP831469612 * T4B);
		    T4G = FNMS(KP555570233, T4F, KP831469612 * T4E);
		    T4H = T4D - T4G;
		    T5C = T4G + T4D;
	       }
	  }
	  {
	       E T1x, T4p, T1A, T4o, T1G, T4m, T1J, T4l;
	       {
		    E T1t, T1z, T1w, T1y, T1u, T1v;
		    T1t = I[WS(is, 5)];
		    T1z = I[WS(is, 37)];
		    T1u = I[WS(is, 21)];
		    T1v = I[WS(is, 53)];
		    T1w = KP707106781 * (T1u - T1v);
		    T1y = KP707106781 * (T1u + T1v);
		    T1x = T1t + T1w;
		    T4p = T1t - T1w;
		    T1A = T1y + T1z;
		    T4o = T1z - T1y;
	       }
	       {
		    E T1F, T1I, T1E, T1H, T1C, T1D;
		    T1F = I[WS(is, 61)];
		    T1I = I[WS(is, 29)];
		    T1C = I[WS(is, 13)];
		    T1D = I[WS(is, 45)];
		    T1E = KP707106781 * (T1C - T1D);
		    T1H = KP707106781 * (T1C + T1D);
		    T1G = T1E - T1F;
		    T4m = T1E + T1F;
		    T1J = T1H + T1I;
		    T4l = T1I - T1H;
	       }
	       {
		    E T1B, T1K, T4w, T4x;
		    T1B = FNMS(KP195090322, T1A, KP980785280 * T1x);
		    T1K = FMA(KP980785280, T1G, KP195090322 * T1J);
		    T1L = T1B + T1K;
		    T3j = T1K - T1B;
		    T4w = FMA(KP831469612, T4p, KP555570233 * T4o);
		    T4x = FMA(KP831469612, T4m, KP555570233 * T4l);
		    T4y = T4w + T4x;
		    T5t = T4w - T4x;
	       }
	       {
		    E T1N, T1O, T4n, T4q;
		    T1N = FNMS(KP980785280, T1J, KP195090322 * T1G);
		    T1O = FMA(KP195090322, T1x, KP980785280 * T1A);
		    T1P = T1N - T1O;
		    T3h = T1O + T1N;
		    T4n = FNMS(KP555570233, T4m, KP831469612 * T4l);
		    T4q = FNMS(KP555570233, T4p, KP831469612 * T4o);
		    T4r = T4n - T4q;
		    T5v = T4q + T4n;
	       }
	  }
	  {
	       E Tx, T2N, T69, T6f, T1e, T6e, T2X, T30, T1Y, T2L, T2Q, T62, T2U, T31, T2H;
	       E T2K, Tw, T63;
	       Tw = Tm + Tv;
	       Tx = Td + Tw;
	       T2N = Td - Tw;
	       T63 = T35 - T34;
	       T69 = T63 - T68;
	       T6f = T63 + T68;
	       {
		    E TS, T1d, T2V, T2W;
		    TS = FNMS(KP098017140, TR, KP995184726 * TK);
		    T1d = FMA(KP995184726, T15, KP098017140 * T1c);
		    T1e = TS + T1d;
		    T6e = T1d - TS;
		    T2V = T2b - T2u;
		    T2W = T2y + T2F;
		    T2X = FNMS(KP671558954, T2W, KP740951125 * T2V);
		    T30 = FMA(KP671558954, T2V, KP740951125 * T2W);
	       }
	       {
		    E T1M, T1X, T2O, T2P;
		    T1M = T1s + T1L;
		    T1X = T1P - T1W;
		    T1Y = FMA(KP998795456, T1M, KP049067674 * T1X);
		    T2L = FNMS(KP049067674, T1M, KP998795456 * T1X);
		    T2O = FMA(KP098017140, TK, KP995184726 * TR);
		    T2P = FNMS(KP995184726, T1c, KP098017140 * T15);
		    T2Q = T2O + T2P;
		    T62 = T2P - T2O;
	       }
	       {
		    E T2S, T2T, T2v, T2G;
		    T2S = T1s - T1L;
		    T2T = T1P + T1W;
		    T2U = FMA(KP740951125, T2S, KP671558954 * T2T);
		    T31 = FNMS(KP671558954, T2S, KP740951125 * T2T);
		    T2v = T2b + T2u;
		    T2G = T2y - T2F;
		    T2H = FNMS(KP049067674, T2G, KP998795456 * T2v);
		    T2K = FMA(KP049067674, T2v, KP998795456 * T2G);
	       }
	       {
		    E T1f, T2I, T6b, T6c;
		    T1f = Tx + T1e;
		    T2I = T1Y + T2H;
		    ro[WS(ros, 31)] = T1f - T2I;
		    ro[0] = T1f + T2I;
		    T6b = T2L + T2K;
		    T6c = T62 + T69;
		    io[WS(ios, 31)] = T6b - T6c;
		    io[0] = T6b + T6c;
	       }
	       {
		    E T2J, T2M, T61, T6a;
		    T2J = Tx - T1e;
		    T2M = T2K - T2L;
		    ro[WS(ros, 16)] = T2J - T2M;
		    ro[WS(ros, 15)] = T2J + T2M;
		    T61 = T2H - T1Y;
		    T6a = T62 - T69;
		    io[WS(ios, 16)] = T61 - T6a;
		    io[WS(ios, 15)] = T61 + T6a;
	       }
	       {
		    E T2R, T2Y, T6h, T6i;
		    T2R = T2N + T2Q;
		    T2Y = T2U + T2X;
		    ro[WS(ros, 24)] = T2R - T2Y;
		    ro[WS(ros, 7)] = T2R + T2Y;
		    T6h = T31 + T30;
		    T6i = T6e + T6f;
		    io[WS(ios, 24)] = T6h - T6i;
		    io[WS(ios, 7)] = T6h + T6i;
	       }
	       {
		    E T2Z, T32, T6d, T6g;
		    T2Z = T2N - T2Q;
		    T32 = T30 - T31;
		    ro[WS(ros, 23)] = T2Z - T32;
		    ro[WS(ros, 8)] = T2Z + T32;
		    T6d = T2X - T2U;
		    T6g = T6e - T6f;
		    io[WS(ios, 23)] = T6d - T6g;
		    io[WS(ios, 8)] = T6d + T6g;
	       }
	  }
	  {
	       E T5j, T5L, T6R, T6X, T5q, T6W, T5V, T5Y, T5y, T5J, T5O, T6O, T5S, T5Z, T5F;
	       E T5I, T5i, T6P;
	       T5i = T5g - T5h;
	       T5j = T5f - T5i;
	       T5L = T5f + T5i;
	       T6P = T3Z + T3W;
	       T6R = T6P - T6Q;
	       T6X = T6P + T6Q;
	       {
		    E T5m, T5p, T5T, T5U;
		    T5m = FMA(KP290284677, T5k, KP956940335 * T5l);
		    T5p = FNMS(KP290284677, T5o, KP956940335 * T5n);
		    T5q = T5m - T5p;
		    T6W = T5p + T5m;
		    T5T = T5z + T5A;
		    T5U = T5C + T5D;
		    T5V = FNMS(KP146730474, T5U, KP989176509 * T5T);
		    T5Y = FMA(KP146730474, T5T, KP989176509 * T5U);
	       }
	       {
		    E T5u, T5x, T5M, T5N;
		    T5u = T5s - T5t;
		    T5x = T5v - T5w;
		    T5y = FMA(KP803207531, T5u, KP595699304 * T5x);
		    T5J = FNMS(KP595699304, T5u, KP803207531 * T5x);
		    T5M = FMA(KP956940335, T5o, KP290284677 * T5n);
		    T5N = FNMS(KP290284677, T5l, KP956940335 * T5k);
		    T5O = T5M + T5N;
		    T6O = T5N - T5M;
	       }
	       {
		    E T5Q, T5R, T5B, T5E;
		    T5Q = T5s + T5t;
		    T5R = T5v + T5w;
		    T5S = FMA(KP989176509, T5Q, KP146730474 * T5R);
		    T5Z = FNMS(KP146730474, T5Q, KP989176509 * T5R);
		    T5B = T5z - T5A;
		    T5E = T5C - T5D;
		    T5F = FNMS(KP595699304, T5E, KP803207531 * T5B);
		    T5I = FMA(KP595699304, T5B, KP803207531 * T5E);
	       }
	       {
		    E T5r, T5G, T6T, T6U;
		    T5r = T5j + T5q;
		    T5G = T5y + T5F;
		    ro[WS(ros, 25)] = T5r - T5G;
		    ro[WS(ros, 6)] = T5r + T5G;
		    T6T = T5J + T5I;
		    T6U = T6O + T6R;
		    io[WS(ios, 25)] = T6T - T6U;
		    io[WS(ios, 6)] = T6T + T6U;
	       }
	       {
		    E T5H, T5K, T6N, T6S;
		    T5H = T5j - T5q;
		    T5K = T5I - T5J;
		    ro[WS(ros, 22)] = T5H - T5K;
		    ro[WS(ros, 9)] = T5H + T5K;
		    T6N = T5F - T5y;
		    T6S = T6O - T6R;
		    io[WS(ios, 22)] = T6N - T6S;
		    io[WS(ios, 9)] = T6N + T6S;
	       }
	       {
		    E T5P, T5W, T6Z, T70;
		    T5P = T5L + T5O;
		    T5W = T5S + T5V;
		    ro[WS(ros, 30)] = T5P - T5W;
		    ro[WS(ros, 1)] = T5P + T5W;
		    T6Z = T5Z + T5Y;
		    T70 = T6W + T6X;
		    io[WS(ios, 30)] = T6Z - T70;
		    io[WS(ios, 1)] = T6Z + T70;
	       }
	       {
		    E T5X, T60, T6V, T6Y;
		    T5X = T5L - T5O;
		    T60 = T5Y - T5Z;
		    ro[WS(ros, 17)] = T5X - T60;
		    ro[WS(ros, 14)] = T5X + T60;
		    T6V = T5V - T5S;
		    T6Y = T6W - T6X;
		    io[WS(ios, 17)] = T6V - T6Y;
		    io[WS(ios, 14)] = T6V + T6Y;
	       }
	  }
	  {
	       E T37, T3z, T6n, T6t, T3e, T6s, T3J, T3M, T3m, T3x, T3C, T6k, T3G, T3N, T3t;
	       E T3w, T36, T6l;
	       T36 = T34 + T35;
	       T37 = T33 - T36;
	       T3z = T33 + T36;
	       T6l = Tv - Tm;
	       T6n = T6l - T6m;
	       T6t = T6l + T6m;
	       {
		    E T3a, T3d, T3H, T3I;
		    T3a = FMA(KP634393284, T38, KP773010453 * T39);
		    T3d = FNMS(KP634393284, T3c, KP773010453 * T3b);
		    T3e = T3a - T3d;
		    T6s = T3d + T3a;
		    T3H = T3n + T3o;
		    T3I = T3q + T3r;
		    T3J = FNMS(KP336889853, T3I, KP941544065 * T3H);
		    T3M = FMA(KP336889853, T3H, KP941544065 * T3I);
	       }
	       {
		    E T3i, T3l, T3A, T3B;
		    T3i = T3g - T3h;
		    T3l = T3j - T3k;
		    T3m = FMA(KP903989293, T3i, KP427555093 * T3l);
		    T3x = FNMS(KP427555093, T3i, KP903989293 * T3l);
		    T3A = FMA(KP773010453, T3c, KP634393284 * T3b);
		    T3B = FNMS(KP634393284, T39, KP773010453 * T38);
		    T3C = T3A + T3B;
		    T6k = T3B - T3A;
	       }
	       {
		    E T3E, T3F, T3p, T3s;
		    T3E = T3g + T3h;
		    T3F = T3j + T3k;
		    T3G = FMA(KP941544065, T3E, KP336889853 * T3F);
		    T3N = FNMS(KP336889853, T3E, KP941544065 * T3F);
		    T3p = T3n - T3o;
		    T3s = T3q - T3r;
		    T3t = FNMS(KP427555093, T3s, KP903989293 * T3p);
		    T3w = FMA(KP427555093, T3p, KP903989293 * T3s);
	       }
	       {
		    E T3f, T3u, T6p, T6q;
		    T3f = T37 + T3e;
		    T3u = T3m + T3t;
		    ro[WS(ros, 27)] = T3f - T3u;
		    ro[WS(ros, 4)] = T3f + T3u;
		    T6p = T3x + T3w;
		    T6q = T6k + T6n;
		    io[WS(ios, 27)] = T6p - T6q;
		    io[WS(ios, 4)] = T6p + T6q;
	       }
	       {
		    E T3v, T3y, T6j, T6o;
		    T3v = T37 - T3e;
		    T3y = T3w - T3x;
		    ro[WS(ros, 20)] = T3v - T3y;
		    ro[WS(ros, 11)] = T3v + T3y;
		    T6j = T3t - T3m;
		    T6o = T6k - T6n;
		    io[WS(ios, 20)] = T6j - T6o;
		    io[WS(ios, 11)] = T6j + T6o;
	       }
	       {
		    E T3D, T3K, T6v, T6w;
		    T3D = T3z + T3C;
		    T3K = T3G + T3J;
		    ro[WS(ros, 28)] = T3D - T3K;
		    ro[WS(ros, 3)] = T3D + T3K;
		    T6v = T3N + T3M;
		    T6w = T6s + T6t;
		    io[WS(ios, 28)] = T6v - T6w;
		    io[WS(ios, 3)] = T6v + T6w;
	       }
	       {
		    E T3L, T3O, T6r, T6u;
		    T3L = T3z - T3C;
		    T3O = T3M - T3N;
		    ro[WS(ros, 19)] = T3L - T3O;
		    ro[WS(ros, 12)] = T3L + T3O;
		    T6r = T3J - T3G;
		    T6u = T6s - T6t;
		    io[WS(ios, 19)] = T6r - T6u;
		    io[WS(ios, 12)] = T6r + T6u;
	       }
	  }
	  {
	       E T41, T4Z, T6D, T6J, T4g, T6I, T59, T5d, T4A, T4X, T52, T6y, T56, T5c, T4T;
	       E T4W, T40, T6C;
	       T40 = T3W - T3Z;
	       T41 = T3T + T40;
	       T4Z = T3T - T40;
	       T6C = T5g + T5h;
	       T6D = T6B - T6C;
	       T6J = T6C + T6B;
	       {
		    E T48, T4f, T57, T58;
		    T48 = FMA(KP881921264, T44, KP471396736 * T47);
		    T4f = FMA(KP881921264, T4b, KP471396736 * T4e);
		    T4g = T48 - T4f;
		    T6I = T48 + T4f;
		    T57 = T4K + T4H;
		    T58 = T4R + T4O;
		    T59 = FMA(KP514102744, T57, KP857728610 * T58);
		    T5d = FNMS(KP857728610, T57, KP514102744 * T58);
	       }
	       {
		    E T4s, T4z, T50, T51;
		    T4s = T4k + T4r;
		    T4z = T4v - T4y;
		    T4A = FMA(KP970031253, T4s, KP242980179 * T4z);
		    T4X = FNMS(KP242980179, T4s, KP970031253 * T4z);
		    T50 = FNMS(KP471396736, T4b, KP881921264 * T4e);
		    T51 = FNMS(KP471396736, T44, KP881921264 * T47);
		    T52 = T50 - T51;
		    T6y = T51 + T50;
	       }
	       {
		    E T54, T55, T4L, T4S;
		    T54 = T4k - T4r;
		    T55 = T4y + T4v;
		    T56 = FMA(KP514102744, T54, KP857728610 * T55);
		    T5c = FNMS(KP514102744, T55, KP857728610 * T54);
		    T4L = T4H - T4K;
		    T4S = T4O - T4R;
		    T4T = FNMS(KP242980179, T4S, KP970031253 * T4L);
		    T4W = FMA(KP242980179, T4L, KP970031253 * T4S);
	       }
	       {
		    E T4h, T4U, T6F, T6G;
		    T4h = T41 + T4g;
		    T4U = T4A + T4T;
		    ro[WS(ros, 29)] = T4h - T4U;
		    ro[WS(ros, 2)] = T4h + T4U;
		    T6F = T4X + T4W;
		    T6G = T6y + T6D;
		    io[WS(ios, 29)] = T6F - T6G;
		    io[WS(ios, 2)] = T6F + T6G;
	       }
	       {
		    E T4V, T4Y, T6x, T6E;
		    T4V = T41 - T4g;
		    T4Y = T4W - T4X;
		    ro[WS(ros, 18)] = T4V - T4Y;
		    ro[WS(ros, 13)] = T4V + T4Y;
		    T6x = T4T - T4A;
		    T6E = T6y - T6D;
		    io[WS(ios, 18)] = T6x - T6E;
		    io[WS(ios, 13)] = T6x + T6E;
	       }
	       {
		    E T53, T5a, T6L, T6M;
		    T53 = T4Z - T52;
		    T5a = T56 - T59;
		    ro[WS(ros, 21)] = T53 - T5a;
		    ro[WS(ros, 10)] = T53 + T5a;
		    T6L = T5d - T5c;
		    T6M = T6J - T6I;
		    io[WS(ios, 21)] = T6L - T6M;
		    io[WS(ios, 10)] = T6L + T6M;
	       }
	       {
		    E T5b, T5e, T6H, T6K;
		    T5b = T4Z + T52;
		    T5e = T5c + T5d;
		    ro[WS(ros, 26)] = T5b - T5e;
		    ro[WS(ros, 5)] = T5b + T5e;
		    T6H = T56 + T59;
		    T6K = T6I + T6J;
		    io[WS(ios, 5)] = -(T6H + T6K);
		    io[WS(ios, 26)] = T6K - T6H;
	       }
	  }
     }
}

static void mr2hcII_64(const R *I, R *ro, R *io, stride is, stride ros, stride ios, int v, int ivs, int ovs)
{
     int i;
     for (i = v; i > 0; --i) {
	  mr2hcII_64_0(I, ro, io, is, ros, ios);
	  I += ivs;
	  ro += ovs;
	  io += ovs;
     }
}

static const kr2hc_desc desc = { 64, "mr2hcII_64", {342, 114, 92, 0}, &GENUS, 0, 0, 0, 0, 0 };

void X(codelet_mr2hcII_64) (planner *p) {
     X(kr2hcII_register) (p, mr2hcII_64, &desc);
}
