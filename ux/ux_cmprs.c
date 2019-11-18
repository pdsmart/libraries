/******************************************************************************
 * Product:       #     # #     #         #         ###   ######
 *                #     #  #   #          #          #    #     #
 *                #     #   # #           #          #    #     #
 *                #     #    #            #          #    ######
 *                #     #   # #           #          #    #     #
 *                #     #  #   #          #          #    #     #
 *                 #####  #     # ####### #######   ###   ######
 *
 * File:          ux_cmprs.c
 * Description:   Routines to compress/decompress data. The basic code stems
 *                from a LINUX public domain lzw compression/decompression
 *                algorithm, basically tidied up a little as it looked awful and
 *                enhanced to allow embedding within programs. Eventually, a more
 *                hi-tech algorithm will be implemented, but for now, this
 *                lzw appears to have very high compression ratio's
 *                on text.
 *
 * Version:       %I%
 * Dated:         %D%
 * Copyright:     P.D. Smart, 1994-2019.
 *
 * History:       1.0  - Initial Release.
 *
 ******************************************************************************
 * This source file is free software: you can redistribute it and#or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This source file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

/* Bring in system header files.
*/
#include    <stdio.h>
#include    <stdlib.h>
#include    <ctype.h>
#include    <malloc.h>
#include    <string.h>
#include    <stdarg.h>
#include    <sys/types.h>
#include    <errno.h>
#include    <sys/timeb.h>
#include    <sys/stat.h>
#include    <fcntl.h>

#if defined(SUNOS) || defined(SOLARIS) || defined(LINUX)
#include    <sys/socket.h>
#include    <sys/time.h>
#include    <string.h>
#endif

#if defined(SOLARIS)
#include    <sys/file.h>
#endif

#if defined(LINUX)
#include    <term.h>
#endif

#if defined(_WIN32)
#include    <winsock.h>
#include    <time.h>
#endif

#if defined(SUNOS) || defined(SOLARIS)
#include    <netinet/in.h>
#include    <sys/wait.h>
#endif

/* Indicate that we are a C module for any header specifics.
*/
#define        UX_CMPRS_C

/* Bring in specific header files.
*/
#include    "ux.h"

#define LZSIZE  4096    /* Should stay < 32768 */ 
#define CLEAR   256     /* Clear Code */
#define REPEAT  257
#define START   258

static code Prefix, Prefix0, Index;
static int code_len, new_entry;        /* For repeated strings */
static int bits, off, size;
static code *scode;
static byte *sbyte;
static unsigned int pcode, pbyte, length;
static int nInit = 0;
static int nNdx;
static unsigned short nEndian = 0xff00;
static unsigned char *pEndian = (unsigned char *)&nEndian;
static unsigned char *pC, cTmp;

code *PTable, *NTable;
byte *CTable;

/******************************************************************************
 * Function:    Compress
 * Description: A generic function to compress a buffer of text. 
 * Returns:     NULL - Memory problems.
 *              Memory buffer containing compressed copy of input.
 ******************************************************************************/
UCHAR *Compress( UCHAR    *spInBuf,      /* I: Buffer to be compressed. */
                 UINT     *nLen )        /* IO: Length of dec/compressed buffer. */
{
    /* Local variables.
    */
    int            nSize;
    UINT        nInLen = *nLen;
    UINT        nOutLen = nInLen+10;
    UCHAR        *spReturn = spInBuf;
    UCHAR        *spOut;
    char        *szFunc = "Compress";

    /* If the input buffer is smaller than a given threshold then dont
     * waste CPU trying to compress it.
    */
    if(nInLen < MIN_COMPRESSLEN)
    {
        return(spInBuf);
    }

    /* Allocate a buffer to hold the compressed data.
    */
    if((spOut=(UCHAR *)malloc(sizeof(USHRT) * nOutLen)) == NULL)
    {
        Lgr(LOG_DEBUG, szFunc, "Couldnt malloc (%d) bytes", nOutLen);
        Errno = E_NOMEM;
        return(NULL);
    }

    /* Compress the buffer.
    */
    nSize=WLZW(spInBuf, (unsigned short *)&spOut[6], nInLen, nOutLen-4);

    /* Could we compress it?
    */
    if(nSize > 0 && (UINT)nSize < nInLen)
    {
        /* OK, add in the compressed id-byte and the original buffer size.
        */
        spOut[0] = spOut[1] =0xff;
        PutCharFromLong( &spOut[2], (ULNG)nInLen );
        spReturn = spOut;
        if(nLen != NULL) *nLen = nSize + 6;

/* Debugging code.
*/
#if defined(UX_DEBUG)
        printf("Compressed from (%d) to (%d) bytes\n", nInLen, nSize);
#endif
    } else
     {
        Lgr(LOG_DEBUG, szFunc, "Couldnt compress data (%d)", nSize);
        if(nLen != NULL) *nLen = nInLen;
        free(spOut);
    }

    /* Return buffer or NULL to caller.
    */
    return(spReturn);
}

/******************************************************************************
 * Function:    Decompress
 * Description: A generic function to de-compress a buffer to text. 
 * Returns:     NULL - Memory problems.
 *              Memory buffer containing decompressed copy of input.
 ******************************************************************************/
UCHAR *Decompress( UCHAR    *spInBuf,         /* I: Buffer to be decompressed. */
                   UINT     *nCmpLen )        /* IO: Length of comp/dec buffer */
{
    /* Local variables.
    */
    UINT        nSize;
    UINT        nOutLen;
    UINT        nShift;
    UCHAR        *spTmp;
    UCHAR        *spReturn = spInBuf;
    char        *szFunc = "Decompress";

    /* Is the input buffer in compressed format..? The first byte should 
     * contain the value 0xff if its compressed.
    */
    if(spInBuf[0] == 0xff && spInBuf[1] == 0xff)
    {
        /* Extract the expanded size from the buffer.
        */
        nOutLen = GetLongFromChar(&spInBuf[2]);

        /* Allocate a buffer to hold the de-compressed data.
        */
        if((spTmp=(UCHAR *)malloc(nOutLen+2)) == NULL)
        {
            Lgr(LOG_DEBUG, szFunc, "Couldnt malloc (%d) bytes", nOutLen);
            Errno = E_NOMEM;
            return(NULL);
        }

        /* Ensure that the data beings on a LONG boundary as the compression/
         * Decompression works with integers/shorts.
        */
        nShift=(int)((long)(&spInBuf[6]) % sizeof(long));

        /* Shift the memory block onto a long boundary.
        */
        memcpy(&spInBuf[6-nShift], &spInBuf[6], (*nCmpLen)-(6-nShift));

        /* Decompress data.
        */
        if((nSize=RLZW((unsigned short *)&spInBuf[6-nShift], spTmp,
                        (*nCmpLen)-6, nOutLen)) <= 0)
        {
            Lgr(LOG_WARNING,szFunc, "Couldnt Decompress data (%s)",nSize);
            free(spTmp);
            return(NULL);
        }

        /* Terminate string as this may just be a character string.
        */
        spTmp[nOutLen] = '\0';

        /* Setup pointer to new buffer containing decompressed data.
        */
        spReturn = spTmp;

        /* Update the callers length parameter to indicate buffers new
         * length.
        */
        *nCmpLen = nOutLen;
    } else
     {
        /* The buffer is not compressed, terminate it in case its a character
         * string. No need to update the callers lenght parameter as it
         * has not changed.
        */
        spInBuf[*nCmpLen] = '\0';

/* Debugging code.
*/
#if defined(UX_DEBUG)
        printf("Buffer not compressed (%c, %x)\n", spInBuf[0], spInBuf[1]);
#endif
    }

    /* Return buffer or NULL to caller.
    */
    return(spReturn);
}

/*
 * LZW_init - Initialise compression routines.
*/
void LZW_init()
{
    PTable = (code *) malloc(LZSIZE*sizeof(code));
    NTable = (code *) malloc(LZSIZE*sizeof(code));
    CTable = (byte *) malloc(LZSIZE*sizeof(byte));
}

/* Write Next Code */
static int WCode(code wcode)
{
    /* Local variables.
    */
    int todo;

    pcode+=bits;

    /* Quickly check output buffer size for insufficient compression.
    */
    if((pcode>>3) > length)
        return 0;

    if((todo = bits+off-16)>=0)
    {
        *scode++ |= wcode>>todo;
        *scode = wcode<<(16-todo);
        off = todo;
    } else
     {
        *scode |= wcode<<(-todo);
        off += bits;
    }

    /* Finished, get out.
    */
    return(1);
}

/* Read Next Code */
static short RCode(void)
{
    /* Local variables.
    */
    code rcode; /* 15 bits maximum, with LZSIZE=32768; never negative */
    short todo;

    if((todo = bits+off-16)>=0)
    {
        rcode = (*scode++)<<todo;
        rcode |= (*scode)>>(16-todo);
        off = todo;
    } else
     {
        rcode = (*scode)>>(-todo);
        off += bits;
    }

    rcode&=size-1;
    return((short)rcode);
}

/* Initialization (R/W) */
static void InitTable(void)
{
    bits = 8;
    size = 256;
    pbyte = pcode = off = 0;

    memset(PTable, -1, LZSIZE*sizeof(code));
    Index = START;
}

/* Initialization for decompression */
static void RInitTable(void)
{
    /* Local variables.
    */
    int i;
    byte *p;

    InitTable();
    for(i = 0, p = CTable; i < CLEAR; i++)
        *p++ = i;
}

/* Lookup Table */
static short LookUp(byte Car)
{
    /* Local variables.
    */
    code pi;

    pi = PTable[Prefix];
    while(((short)pi) != -1)
    {
        if(CTable[pi] == Car) return pi;
        pi = NTable[pi];
    }

    /* No hit.
    */
    return(-1);
}

/* byte is added to table and becomes prefix */
static int WAddPrefix(void)
{
    /* Local variables.
    */
    code pi;

    pi = PTable[Prefix];
    PTable[Prefix] = Index;        /* Next entry */
    NTable[Index] = pi;
    CTable[Index++] = Prefix = Prefix0 = *sbyte;

    /* Table full.
    */
    if(Index == LZSIZE)
    {
        if(!WCode(CLEAR)) return 0; 
        Index = START;
        memset(PTable, -1, LZSIZE*sizeof(code));
        bits = 8;
        size = 256;
    } else
    if(Index > size)
    {
        bits++;
        size <<= 1;
    }

    /* Finished, get out.
    */
    return(1);
}

static int Expand(code val)
{
    /* Local variables.
    */
    code p,q,r;

    if(val > Index)
        return -2;
    q = -1;
    PTable[Index] = Prefix;
    do {
        p = val;

        /* Get previous
        */
        while((p = PTable[(r = p)]) != q);

        *sbyte++ = CTable[r];

        /* Done; skip useless stuff
        */
        if(++pbyte >= length)
            return 0;

        if(((short)q) == -1 && new_entry)
        {
            if(Index == LZSIZE) return -3;
            CTable[Index++] = CTable[r];
            new_entry = 0;
        }
        q = r;
    } while(q != val);
    Prefix = val;

    /* Finished, get out.
    */
    return(1);
}

int check_repeat(int len)
{
    /* Local variables.
    */
    int        n = 0;
    char    *s;
    char    *s0;

    /* Current string.
    */
    s = sbyte;

    /* Reference string.
    */
    s0 = s - code_len;

    while(code_len <= len && (n+1) < size)
    {
        if(memcmp(s0, s, code_len))
            break;

        /* Number of bytes left.
        */
        len -= code_len;

        /* Number of repeats.
        */
        n++;
        s += code_len;
    }

    /* Finished, get out!
    */
    return(n);
}

/******************************************************************************
 * Function:    WLZW
 * Description: Write or compress data in LZW format.
 * Returns:     0  = Worthless CPU waste (No compression)
 *              -1 = General error
 *              -2 = Logical error
 *              -3 = Expand error
 *              >0 = OK/total length
 ******************************************************************************/
int WLZW( byte       *si,        /* I: Data for compression */
          code       *so,        /* O: Compressed data */
          int        len,        /* I: Length of data for compression */
          int        maxlen )    /* I: Maximum length of compressed data */
{
    /* Local variables.
    */
    code    val;
    int        Repeat;

    if(nInit == 0)
    {
        LZW_init();
        nInit = 1;
    }

    scode = so;
    *scode = 0;
    sbyte = si;
    length = maxlen;
    InitTable();

    Prefix = Prefix0 = *sbyte;
    code_len = 0;
    while(++pbyte<len)
    {
        /* Length of current string.
        */
        code_len++;

        /* at least 2 bytes.
        */
        if(*(++sbyte) == Prefix0 && code_len > 1)
        {
            /* Check for string repeat. If positive, we are going to write
             * 3 codes.  Therefore, we need to filter small repeats.
            */
            Repeat = check_repeat(len-pbyte);

            /* Could do better.
             */
            if(Repeat > 1)
            {
                /* At least 3 times (1+>2).
                */
                WCode(REPEAT);
                WCode(Repeat);
                if(!WCode(Prefix))
                    return(0);

                /* Total length of repeat.
                */
                pbyte += code_len*Repeat;
                if(pbyte >= len)
                    break;

                /* Position of next byte.
                */
                sbyte += code_len*Repeat;
            }
        }

        /* Break in sequence.
        */
        if(((short)(val=LookUp(*sbyte)))==-1)
        {
            /* Write to buffer.
            */
            if(!WCode(Prefix))
                return(0);

            /* Car is new prefix.
            */
            if(!WAddPrefix())
                return(0);
            code_len = 0;
        } else
            Prefix = val;
    }

    if(!WCode(Prefix))
        return(0);

    len = pcode/sizeof(code)/8;
    if(pcode&(sizeof(code)*8-1))
        len++;
    len *= sizeof(code);

    /* Portability issue, will change the code to be insensitive to 
     * little/big endian eventually.
    */
    if(*pEndian == 0x00)
    {
        pC = (unsigned char *)so;
        for(nNdx=0; nNdx < len; nNdx += 2, pC += 2)
        {
            cTmp = *pC;
            *pC = *(pC+1);
            *(pC+1) = cTmp;
        }
    }
    return(len);
}

/* CLEAR Code Read */
static int RClear(void)
{
    bits = 8;
    size = 256;
    Prefix = RCode();
    *sbyte++ = Prefix;
    if(++pbyte >= length)
        return(0);
    new_entry = 1;
    Index = START;
    bits++;
    size = 512;

    /* Finished, get out!
    */
    return(1);
}

/******************************************************************************
 * Function:    RLZW
 * Description: Read or de-compress data from LZW format.
 * Returns:     0  = Worthless CPU waste (No compression)
 *              -1 = General error
 *              -2 = Logical error
 *              -3 = Expand error
 *              >0 = OK/total length
 ******************************************************************************/
int RLZW( code       *si,    /* I: Data to be decompressed */
          byte       *so,    /* O: Decompressed data */
          int        silen,  /* I: Compressed length */
          int        len )   /* I: Expected length of decompressed data. */
{
    /* Local variables.
    */
    code    val;
    int        n;

    /* Portability issue, will change the code to be insensitive to 
     * little/big endian eventually.
    */
    if(*pEndian == 0x00)
    {
        pC = (unsigned char *)si;
        for(nNdx=0; nNdx < silen; nNdx += 2, pC += 2)
        {
            cTmp = *pC;
            *pC = *(pC+1);
            *(pC+1) = cTmp;
        }
    }

    if(nInit == 0)
    {
        LZW_init();
        nInit = 1;
    }

    sbyte  = so;
    scode  = si;
    length = len;
    RInitTable();

    RClear();
    for(;;)
    {
        val = RCode();
        if(val == CLEAR)
        {
            if(!RClear())
                break;
            continue;
        }

        if(val == REPEAT)
        {
            /* # of repeats.
            */
            len = RCode();
            val = RCode();    /* code to expand */
            do {
                n = Expand(val);
            } while(--len && n > 0);
        } else
         {
            n = Expand(val);
            if(Index >= size)
            {
                bits++;
                size <<= 1;
            }

            /* Add new entry at next code.
            */
            new_entry = 1;
        }
        if(n < 0)
            return(n);
        if(n == 0)
            break;
    } 

    /* Finished, get out!
    */
    return(1);
}
