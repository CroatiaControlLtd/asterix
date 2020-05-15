/*
 *  Copyright (c) 2013 Croatia Control Ltd. (www.crocontrol.hr)
 *
 *  This file is part of Asterix.
 *
 *  Asterix is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Asterix is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Asterix.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * AUTHORS: Damir Salantic, Croatia Control Ltd.
 *
 */

#ifndef ASTERIXHDLCPARSING_H_
#define ASTERIXHDLCPARSING_H_

// defines
#define MAX_RXBUF 0x1000                        // 4K
//#define MAX_RXBUF 90 //test
#define MAX_CBUF  0x10000                       // 64K
#define MAX_FRM   0x200                         // 2*256 byte

// data
#ifdef __cplusplus
extern "C" {
#endif

extern unsigned char RxBuf[];       // buffer za ucitavanje podatka s COM
extern int copy_to_cbuf(unsigned char *RxBuf, int Cnt);

extern int get_hdlc_frame(int iF, int MinLen);

extern unsigned short test_hdlc_fcs(int iF, int iL);

extern unsigned char *get_next_hdlc_frame(int *len);

extern int GetAndResetFailedBytes();

#ifdef __cplusplus
}
#endif

// functions

#endif /* ASTERIXHDLCPARSING_H_ */
