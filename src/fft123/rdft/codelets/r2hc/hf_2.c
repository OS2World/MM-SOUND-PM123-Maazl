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
/* Generated on Sat Jul  5 21:56:51 EDT 2003 */

#include "codelet-rdft.h"

/* Generated by: /homee/stevenj/cvs/fftw3.0.1/genfft/gen_hc2hc -compact -variables 4 -n 2 -dit -name hf_2 -include hf.h */

/*
 * This function contains 6 FP additions, 4 FP multiplications,
 * (or, 4 additions, 2 multiplications, 2 fused multiply/add),
 * 9 stack variables, and 8 memory accesses
 */
/*
 * Generator Id's : 
 * $Id: hf_2.c,v 1.1 2005/07/26 17:37:08 glass Exp $
 * $Id: hf_2.c,v 1.1 2005/07/26 17:37:08 glass Exp $
 * $Id: hf_2.c,v 1.1 2005/07/26 17:37:08 glass Exp $
 */

#include "hf.h"

static const R *hf_2(R *rio, R *iio, const R *W, stride ios, int m, int dist)
{
     int i;
     for (i = m - 2; i > 0; i = i - 2, rio = rio + dist, iio = iio - dist, W = W + 2) {
	  E T1, T8, T6, T7;
	  T1 = rio[0];
	  T8 = iio[-WS(ios, 1)];
	  {
	       E T3, T5, T2, T4;
	       T3 = rio[WS(ios, 1)];
	       T5 = iio[0];
	       T2 = W[0];
	       T4 = W[1];
	       T6 = FMA(T2, T3, T4 * T5);
	       T7 = FNMS(T4, T3, T2 * T5);
	  }
	  iio[-WS(ios, 1)] = T1 - T6;
	  rio[WS(ios, 1)] = T7 - T8;
	  rio[0] = T1 + T6;
	  iio[0] = T7 + T8;
     }
     return W;
}

static const tw_instr twinstr[] = {
     {TW_FULL, 0, 2},
     {TW_NEXT, 1, 0}
};

static const hc2hc_desc desc = { 2, "hf_2", twinstr, {4, 2, 2, 0}, &GENUS, 0, 0, 0 };

void X(codelet_hf_2) (planner *p) {
     X(khc2hc_dit_register) (p, hf_2, &desc);
}
