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

/**

 * @file asterixhdlcparsing.cxx
 *
 * @brief Reading anf parsing of HDLC packets.
 *        HDLC data is in following format:
 *        1 byte - 0x01 (addr)
 *        1 byte - 0x03 (ctrl)
 *        N bytes - Asterix packet
 *        2 bytes - FCS
 *
 *
 * Based on :
 *      AtxRecorder.cpp
 *      Verzija 1.2 (3.10.2007.)
 *      (P) HK, HKZP d.o.o.
 *
 */

#include "asterixhdlcparsing.h"

//
// Podaci za proracun FCS
//
#define FCS_INIT  0xffff
#define FCS_GOOD  0xf0b8
unsigned short FCStab[256] = {
        0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
        0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
        0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
        0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
        0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
        0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
        0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
        0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
        0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
        0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
        0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
        0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
        0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
        0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
        0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
        0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
        0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
        0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
        0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
        0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
        0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
        0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
        0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
        0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
        0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
        0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
        0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
        0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
        0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
        0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
        0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
        0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};
//
// Podaci za detekciju pocetka HDLC paketa
//
#define MIN_BLK 10        // najmanja prihvatljiva duljina HDLC paketa
#define MSK_LEN    4        // broj pocetnih okteta koji se testiraju
unsigned char Msk1[MSK_LEN] = {0xFF, 0xFF, 0xF8, 0xFF};
unsigned char Msk2[MSK_LEN] = {0x01, 0x03, 0x00, 0x00};
//
// Bufferi
//
unsigned char RxBuf[MAX_RXBUF];    // buffer za ucitavanje podatka s COM
unsigned char cBuf[MAX_CBUF];    // cirkularni buffer za ulazne podatke
unsigned char Frame[MAX_FRM];        // HDLC paket

//
// Indeksi i brojaci
//
int iBuf = 0;    // index za upis slijedeceg okteta u cirkularni buffer
int nAtx = 0;    // trenutna duljina ASTERIX bloka

int iF = -1;    // index pocetka HDLC paketa (ili -1)
int iC = -1;    // index pocetka HDLC paketa u cirkularnom bufferu

int iNorthOK = 0; // 1 - ako treba a�urirati tablicu

unsigned long nByteIn = 0;
unsigned long nFrmOK = 0;
unsigned long nFrmErr = 0;
unsigned long nBytesErr = 0;
unsigned long Cat001 = 0;
unsigned long Cat002 = 0;
unsigned long Cat008 = 0;
unsigned long Cat016 = 0;
unsigned long CatErr = 0;

unsigned short test_hdlc_fcs(int iF, int iL);

int test_frame_iF(int iS);

int get_hdlc_frame(int iS, int MinLen);

//int get_atx_rec( int iS );
int proc_hdlc_frame(int len);

int GetAndResetFailedBytes() {
    int ret = nBytesErr;
    nBytesErr = 0;
    return ret;
}

#if 0
/*-----------------------------------------------------------------------------------------*/
int _tmain(int argc, _TCHAR* argv[])
/*-----------------------------------------------------------------------------------------*/
{
    int retv = 0;
    int cnt = 0;

    pDEB = fopen( "AtxRecorder.TXT", "wt" );
    pATX = fopen( "AtxRecorder.BIN", "wb" );

// ucitaj podatke iz AtxRecorder.INI
    if( (retv = read_ini()) ) {
        printf( "\nError: read_ini() = %d", retv );
        getch();
        return 0;
    }
// otvori port za ASTERIX
    if( (retv = atx_open()) ) {
        printf( "\nError: atx_open() = %d", retv );
        getch();
        return 0;
    }
// testiraj i ocisti ulazni buffer
    if( !TestFile ) {
        if( atx_read() < 0 )
            hComAtx = INVALID_HANDLE_VALUE;
        if( atx_read() < 0 )
            hComAtx = INVALID_HANDLE_VALUE;
    }
    iBuf = 0;
//
// glavni proces
//
    int iF = -1;	// index pocetka HDLC paketa (ili -1)
    int iC = -1;	// index pocetka HDLC paketa u cirkularnom bufferu

    printf( "\n AtxRecorder _____________________________________________________________\n" );
    printf( "\n Ver 1.20, (P)2006 Hrvoje Kalinovcic" );
    printf( "\n Copyright (C)2006 Croatia Control Ltd. All rights reserved.\n" );
    printf( "\n Port = %s\n", ComAtx );
    printf( "\n -------------------------------------------------------------------------" );
    printf( "\n       Byte   Frames   Frames" );
    printf( "\n      Count    Valid      Bad   Cat001   Cat002   Cat008   Cat016   CatErr" );
    printf( "\n -------------------------------------------------------------------------\n" );

    // beskonacna petlja
    while( hComAtx != INVALID_HANDLE_VALUE ) {
        if( (retv = atx_read()) < 0 )
            hComAtx = INVALID_HANDLE_VALUE;
        nByteIn += retv;

        // bilo koja tipka prekida program
        while( kbhit() ) {
            getch();
            hComAtx = INVALID_HANDLE_VALUE;
        }

        // da li se u primljenim podacima nalazi kraj prethodnog paketa?
        if( iF != -1 ) {
            int len = get_hdlc_frame( iF, MIN_BLK );	// duljina HDLC paketa
            if( len ) {
                if( proc_hdlc_frame( len ) )
                    iF = -1;
            }
        }

        // da li se u primljenim podacima nalazi pocetak novog HDLC paketa
        int i = (iC+1) % MAX_CBUF;
        while( i != iBuf ) {
            if( test_frame_iF( i ) ) {
                if( iF == -1 ) nFrmOK++;
                else           nFrmErr++;
                iF = iC = i;						// pocetak HDLC paketa
                break;
            }
            i = (i + 1) % MAX_CBUF;
        }

        printf( "%c%11lu%9lu%9lu%9lu%9lu%9lu%9lu%9lu",
            0x0D, nByteIn, nFrmOK, nFrmErr, Cat001, Cat002, Cat008, Cat016, CatErr );
    }
    printf( "\n\nAtxRecorder Stopped\n" );
    getch();
}
#endif


/*-----------------------------------------------------------------------------------------*/
int copy_to_cbuf(unsigned char *RxBuf, int Cnt)
//
// Copy Cnt bytes from RxBuf to cBuf
//
/*-----------------------------------------------------------------------------------------*/
{
    int j = 0;
    if (Cnt) {
        for (j = 0; j < (int) Cnt; j++) {    // kopiraj u CBuf
            cBuf[iBuf++] = RxBuf[j];
            iBuf %= MAX_CBUF;
        }
    }
    return Cnt;    // ucitano Cnt novih podataka (moguce je i Cnt = 0)
}

unsigned char *get_next_hdlc_frame(int *pRetLen) {
    // da li se u primljenim podacima nalazi kraj prethodnog paketa?
    if (iF != -1) {
        int len = get_hdlc_frame(iF, MIN_BLK);        // duljina HDLC paketa
        if (len) {
            if (proc_hdlc_frame(len)) {
                iF = -1;
                *pRetLen = len - 4; // minus HDLC header and checksum
                return &Frame[2];
            }
        }
    }

    // da li se u primljenim podacima nalazi pocetak novog HDLC paketa
    int i = (iC + 1) % MAX_CBUF;
    while (i != iBuf) {
        if (test_frame_iF(i)) {
            if (iF == -1) {
                nFrmOK++;
            } else {
                nBytesErr += i - iF;
                nFrmErr++;
            }
            iF = iC = i;                                            // pocetak HDLC paketa
            break;
        }
        i = (i + 1) % MAX_CBUF;
    }

    // da li se u primljenim podacima nalazi kraj prethodnog paketa?
    if (iF != -1) {
        int len = get_hdlc_frame(iF, MIN_BLK);        // duljina HDLC paketa
        if (len) {
            if (proc_hdlc_frame(len)) {
                iF = -1;
                *pRetLen = len - 4; // minus HDLC header and checksum
                return &Frame[2];
            }
        }
    }

    return 0;
}

/*-----------------------------------------------------------------------------------------*/
int test_frame_iF(int iS)
//
// provjerava da li cBuf[iS] moze biti pocetak novog HDLC paketa
//
/*-----------------------------------------------------------------------------------------*/
{
    int i = iS;
    int m = 0;
    for (m = 0; m < MSK_LEN; m++) {
        if (i == iBuf)
            return 0;                    // NE, kraj cirkularnog buffera
        if ((cBuf[i++] & Msk1[m]) ^ Msk2[m])
            return 0;                    // NE, ovaj byte nije prosao test
        i %= MAX_CBUF;
    }
    return 1; // DA, zadani "iS" moze biti pocetak novog HDLC paketa
}

/*-----------------------------------------------------------------------------------------*/
int get_hdlc_frame(int iF, int MinLen)
//
// vadi HDLC paket iz cirkularnog buffera i vraca duljinu HDLC paketa
// po kriteriju ispravnog HDLC FCS
// iF		- pocetak paketa u cBuf
// MinLen	- najmanja duljina HDLC paketa
//
/*-----------------------------------------------------------------------------------------*/
{
    int i = iF;
    int j = 0;
    while (i != iBuf) {                // kopiraj podatke u buffer
        Frame[j++] = cBuf[i++];
        i %= MAX_CBUF;
        if (j >= MAX_FRM)
            break;
    }
    if (MinLen < MSK_LEN)                // ovo je besmisleno
        return 0;
    if (j < MinLen)
        return 0;                        // nema jos dovoljno novih podataka

    for (i = MinLen - 1; i < j; i++) {
        if (!test_hdlc_fcs(0, i))
            return i + 1;                    // i je moguci zavrsetak HDLC paketa
    }
    return 0; // nema FCS_GOOD za ovaj IF
}

/*-----------------------------------------------------------------------------------------*/
unsigned short test_hdlc_fcs(int iF, int iL)
//
// provjerava HDLC FCS za dio buffera od &Frame[iF] do &Frame[iL]
// ako zadnja dva okteta daju ispravan HDLC FCS, rutina ce vratiti 0x0000
//
/*-----------------------------------------------------------------------------------------*/
{
    int len = iL - iF + 1;
    unsigned short fcs = FCS_INIT;
    unsigned char *cp = &Frame[iF];

    while (len--)
        fcs = (fcs >> 8) ^ FCStab[0xFF & (fcs ^ (*cp++))];

    return (fcs ^ FCS_GOOD);    // ispravan paket vraca 0x0000
}


/*-----------------------------------------------------------------------------------------*/
int proc_hdlc_frame(int len)
/*-----------------------------------------------------------------------------------------*/
{
/* Check if there are correct Asterix packet(s) in HDLC frame
 * HDLC frame format:
 * ----------------- ---------Asterix record 1-------    ---------Asterix record N-------       -----HDLC checksum---
 * [HLEN_H] [HLEN_L] [CAT] [ALEN_H] [ALEN_L] [payload] { [CAT] [ALEN_H] [ALEN_L] [payload]} ... [CHKSUM_H] [CHKSUM_L]
 */

    int len_to_check = len - 4; // len - [HLEN_H] [HLEN_L] [CHKSUM_H] [CHKSUM_L]
    int offset = 2;

    while (len_to_check > 0) {
        int cat = (int) Frame[offset]; // [CAT]
        int packet_len = (int) Frame[offset + 1]; // [ALEN_H]
        packet_len <<= 8;
        packet_len |= (int) Frame[offset + 2]; // [ALEN_L]

        switch (cat) {
            case 1:
                Cat001++;
                break;
            case 2:
                Cat002++;
                break;
            case 8:
                Cat008++;
                break;
            case 16:
                Cat016++;
                break;
            default:
                CatErr++;
        }

        offset += packet_len;
        len_to_check -= packet_len;
    }

    return (len_to_check == 0);
}

