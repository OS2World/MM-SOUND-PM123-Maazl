/*
 * Copyright 1997-2003 Samuel Audet <guardia@step.polymtl.ca>
 *                     Taneli Lepp� <rosmo@sektori.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 *    3. The name of the author may not be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _REALEQ_H
#define _REALEQ_H

#define ID_EQ     1006 /* = IDH_EQUALIZER */

#define EQ_ENABLED  20
#define ID_LOCKLR   21

#define EQ_SAVE     30
#define EQ_LOAD     31
#define EQ_ZERO     32
#define EQ_FILE     33

#define ID_FIRORDER 40
#define ID_PLANSIZE 41

#define ID_GAIN     50
#define ID_GROUPDELAY 51

#define ID_UTXTL    60
#define ID_CTXTL    61
#define ID_BTXTL    62
#define ID_UTXTR    65
#define ID_CTXTR    66
#define ID_BTXTR    67

#define ID_MASTERL  100
#define ID_BANDL    101 // .. 132
#define ID_MASTERR  150
#define ID_BANDR    151 // .. 182
#define ID_BANDEND  199  

#define ID_MUTEALLL 200
#define ID_MUTEL    201 // .. 232
#define ID_MUTEALLR 250
#define ID_MUTER    251 // .. 282
#define ID_MUTEEND  299

#endif /* _REALEQ_H */

