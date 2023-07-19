// SPDX-License-Identifier: EPL-2.0
// Copyright 2017 DaSoftver LLC.
// Licensed under Eclipse Public License - v 2.0. See LICENSE file.
// On the web https://vely.dev/ - this file is part of Vely framework.

// 
// UTF8-related module
//

//
// Based on UTF8 standard: https://www.rfc-editor.org/rfc/rfc3629
//

#include "vely.h"

//
// get original unicode from two surrogates. Return that value.
//
num32 vely_make_from_utf8_surrogate (num32 u0, num32 u1)
{
    VV_TRACE("");
    return (u0 << 10) + u1 + (0x10000 - (0xD800 << 10) - 0xDC00);
}

//
// Encode binary UTF8 string 'r' to unicode value 'u'. Return number of bytes of UTF8 picked up (1,2,3 or 4) or -1 if error
// 'e' is error text
//
num vely_encode_utf8 (char *r, num32 *u, char **e)
{
    VV_TRACE("");
    if ((r[0] & 128) == 0) 
    {
        // single byte, ascii
        *u = (num32) r[0];
        return 1;
    }
    // These 3 else if must be in this order, because checking for 192 is true for both 224 and 240
    // so if 192 were first, 224 and 240 would never happen
    else if ((r[0] & 240) == 240)
    {
        // four byte
        *u = (r[0] & 7) << 18;
        if ((r[1] & 128) == 128) *u += ((r[1] & 63)<<12);
        else { *e = VV_UTF8_ERR_SECOND_BYTE; return -1;}
        if ((r[2] & 128) == 128) *u += ((r[2] & 63)<<6);
        else { *e = VV_UTF8_ERR_THIRD_BYTE; return -1;}
        if ((r[3] & 128) == 128) *u += (r[3] & 63);
        else { *e = VV_UTF8_ERR_FOURTH_BYTE; return -1;}
        return 4;
    }
    else if ((r[0] & 224) == 224)
    {
        // three byte
        *u = (r[0] & 15) << 12;
        if ((r[1] & 128) == 128) *u += ((r[1] & 63)<<6);
        else { *e = VV_UTF8_ERR_SECOND_BYTE; return -1;}
        if ((r[2] & 128) == 128) *u += (r[2] & 63);
        else { *e = VV_UTF8_ERR_THIRD_BYTE; return -1;}
        return 3;
    }
    else if ((r[0] & 192) == 192)
    {
        // two byte
        *u = (r[0] & 31) << 6;
        if ((r[1] & 128) == 128) *u += (r[1] & 63);
        else { *e = VV_UTF8_ERR_SECOND_BYTE; return -1;}
        return 2;
    }
    else
    {
        *e = VV_UTF8_INVALID; return -1;
    }
}

//
// Break unicode u into surrogate u0 and u1. 
// 
//
void vely_get_utf8_surrogate (num32 u, num32 *u0, num32 *u1)
{
    VV_TRACE("");
    *u0 = (0xD800 - (0x10000 >> 10)) + (u>>10);
    *u1 = 0xDC00 + (u&0x3FF);
}

//
// Decode 32 bit Unicode to 1,2,3 or 4 UTF8 string
// 'u' is unicode number of a character, range 0 to 10FFFF.
// 'r' is the result string of UTF8 (1,2,3 or 4 bytes), r must point to at least 5 bytes of memory (4+null)
// 'e' is error message. 
// returns -1 on error, number of bytes (excluding 0) if okay
//
num vely_decode_utf8 (num32 u, unsigned char *r, char **e)
{
    VV_TRACE("");
    if (u <= 0x7F) { r[0] = (unsigned char)u; return 1; } // single byte (ASCII)
    else if (u >= 0x80 && u <= 0x7FF) 
    { 
        // 192 = b11000000
        r[0] = 192+(u>>6); // >>6 to get the top 5 (out of 11) bits
        r[1] = 128+(u&63); // 63 is 111111 to extract lower 6 bits
        return 2; 
    }
    else if (u >= 0x800 && u <= 0xFFFF) 
    {
        if (u == 0xFEFF) {*e = VV_UTF8_ERR_ILLEGAL_CHARACTER_FEFF; return -1;}
        // 224 = 11100000
        r[0] = 224+(u>>12); // get top 4 (out of 16) bits
        r[1] = 128+((u>>6)&63); // get middle 6 from 4+6+6 group of bits
        r[2] = 128+(u&63); // get lower 6 bits
        return 3;
    }
    else if (u >= 0x10000 && u <= 0x10FFFF) 
    {
        // 240 = 11110000
        r[0] = 240+(u>>18); // get top 3 (out of 21) bits
        r[1] = 128+((u>>12)&63); // get 6 after 3 from 3+6+6+6 group of bits
        r[2] = 128+((u>>6)&63); // get 6 after second 6 in 3+6+6+6 group of bits
        r[3] = 128+(u&63); // get lower 6 bits
        return 4;
    }
    else { *e = VV_UTF8_ERR_OUT_OF_RANGE; return -1; }
    return -1; // just to satisfy compiler
}

