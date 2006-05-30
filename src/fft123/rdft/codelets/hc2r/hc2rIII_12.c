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
/* Generated on Sat Jul  5 22:12:07 EDT 2003 */

#include "codelet-rdft.h"

/* Generated by: /homee/stevenj/cvs/fftw3.0.1/genfft/gen_hc2r -compact -variables 4 -sign 1 -n 12 -name hc2rIII_12 -dft-III -include hc2rIII.h */

/*
 * This function contains 42 FP additions, 20 FP multiplications,
 * (or, 38 additions, 16 multiplications, 4 fused multiply/add),
 * 25 stack variables, and 24 memory accesses
 */
/*
 * Generator Id's : 
 * $Id: hc2rIII_12.c,v 1.1 2005/07/26 17:37:06 glass Exp $
 * $Id: hc2rIII_12.c,v 1.1 2005/07/26 17:37:06 glass Exp $
 * $Id: hc2rIII_12.c,v 1.1 2005/07/26 17:37:06 glass Exp $
 */

#include "hc2rIII.h"

static void hc2rIII_12(const R *ri, const R *ii, R *O, stride ris, stride iis, stride os, int v, int ivs, int ovs)
{
     DK(KP1_414213562, +1.414213562373095048801688724209698078569671875);
     DK(KP2_000000000, +2.000000000000000000000000000000000000000000000);
     DK(KP500000000, +0.500000000000000000000000000000000000000000000);
     DK(KP866025403, +0.866025403784438646763723170752936183471402627);
     int i;
     for (i = v; i > 0; i = i - 1, ri = ri + ivs, ii = ii + ivs, O = O + ovs) {
	  E T5, Tw, Tb, Te, Tx, Ts, Ta, TA, Tg, Tj, Tz, Tp, Tt, Tu;
	  {
	       E T1, T2, T3, T4;
	       T1 = ri[WS(ris, 1)];
	       T2 = ri[WS(ris, 5)];
	       T3 = ri[WS(ris, 2)];
	       T4 = T2 + T3;
	       T5 = T1 + T4;
	       Tw = KP866025403 * (T2 - T3);
	       Tb = FNMS(KP500000000, T4, T1);
	  }
	  {
	       E Tq, Tc, Td, Tr;
	       Tq = ii[WS(iis, 1)];
	       Tc = ii[WS(iis, 5)];
	       Td = ii[WS(iis, 2)];
	       Tr = Td - Tc;
	       Te = KP866025403 * (Tc + Td);
	       Tx = FMA(KP500000000, Tr, Tq);
	       Ts = Tq - Tr;
	  }
	  {
	       E T6, T7, T8, T9;
	       T6 = ri[WS(ris, 4)];
	       T7 = ri[0];
	       T8 = ri[WS(ris, 3)];
	       T9 = T7 + T8;
	       Ta = T6 + T9;
	       TA = KP866025403 * (T7 - T8);
	       Tg = FNMS(KP500000000, T9, T6);
	  }
	  {
	       E To, Th, Ti, Tn;
	       To = ii[WS(iis, 4)];
	       Th = ii[0];
	       Ti = ii[WS(iis, 3)];
	       Tn = Ti - Th;
	       Tj = KP866025403 * (Th + Ti);
	       Tz = FMA(KP500000000, Tn, To);
	       Tp = Tn - To;
	  }
	  O[0] = KP2_000000000 * (T5 + Ta);
	  O[WS(os, 6)] = KP2_000000000 * (Ts + Tp);
	  Tt = Tp - Ts;
	  Tu = T5 - Ta;
	  O[WS(os, 3)] = KP1_414213562 * (Tt - Tu);
	  O[WS(os, 9)] = KP1_414213562 * (Tu + Tt);
	  {
	       E Tf, Tk, Tv, Ty, TB, TC;
	       Tf = Tb - Te;
	       Tk = Tg + Tj;
	       Tv = Tf - Tk;
	       Ty = Tw + Tx;
	       TB = Tz - TA;
	       TC = Ty + TB;
	       O[WS(os, 4)] = -(KP2_000000000 * (Tf + Tk));
	       O[WS(os, 10)] = KP2_000000000 * (TB - Ty);
	       O[WS(os, 1)] = KP1_414213562 * (Tv - TC);
	       O[WS(os, 7)] = KP1_414213562 * (Tv + TC);
	  }
	  {
	       E Tl, Tm, TF, TD, TE, TG;
	       Tl = Tb + Te;
	       Tm = Tg - Tj;
	       TF = Tm - Tl;
	       TD = TA + Tz;
	       TE = Tx - Tw;
	       TG = TE + TD;
	       O[WS(os, 8)] = KP2_000000000 * (Tl + Tm);
	       O[WS(os, 5)] = KP1_414213562 * (TF + TG);
	       O[WS(os, 2)] = KP2_000000000 * (TD - TE);
	       O[WS(os, 11)] = KP1_414213562 * (TF - TG);
	  }
     }
}

static const khc2r_desc desc = { 12, "hc2rIII_12", {38, 16, 4, 0}, &GENUS, 0, 0, 0, 0, 0 };

void X(codelet_hc2rIII_12) (planner *p) {
     X(khc2rIII_register) (p, hc2rIII_12, &desc);
}
