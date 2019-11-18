/******************************************************************************
 * Product:       #     # #     #         #         ###   ######
 *                #     #  #   #          #          #    #     #
 *                #     #   # #           #          #    #     #
 *                #     #    #            #          #    ######
 *                #     #   # #           #          #    #     #
 *                #     #  #   #          #          #    #     #
 *                 #####  #     # ####### #######   ###   ######
 *
 * File:          ux_str.c
 * Description:   General purpose string processing funtions. Additions to
 *                those which exist within the C libraries.
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
#define        UX_STRINGPROCESSING_C

/* Bring in specific header files.
*/
#include    "ux.h"

/******************************************************************************
 * Function:    PutCharFromLong
 * Description: Place a long type variable into a character buffer in a known
 *              byte order. IE. A long is 32 bit, and is placed into the
 *              char buffer as MSB (3), 2, 1, LSB (0). Where MSB fits into the
 *              first byte of the buffer.
 * Returns:     R_OK   - Cannot fail ... will I be eating my words...?
 ******************************************************************************/
int    PutCharFromLong( UCHAR    *pDestBuf,    /* O: Destination buffer */
                        ULNG     lVar )        /* I: Variable of type long */
{
    /* Local variables.
    */
    int         nReturn = R_OK;
    ULNG        lVal = lVar;

    /* Cant assume any byte ordering, so use arithmetic to break it down
     * into fundamental components. Assume a long is 32bit.
    */
    pDestBuf[0] = (UCHAR)(lVal/16777216L);
    lVal       -= (ULNG)pDestBuf[0] * 16777216L;
    pDestBuf[1] = (UCHAR)(lVal/65536L);
    lVal       -= (ULNG)pDestBuf[1] * 65536L;
    pDestBuf[2] = (UCHAR)(lVal/256L);
    lVal       -= (ULNG)pDestBuf[2] * 256L;
    pDestBuf[3] = (UCHAR)lVal;

    /* Finished, get out!!
    */
    return( nReturn );
}

/******************************************************************************
 * Function:    PutCharFromInt
 * Description: Place an int type variable into a character buffer in a known
 *              byte order. IE. An int is 16 bit, and is placed into the
 *              char buffer as MSB (1), LSB (0). Where MSB fits into the
 *              first byte of the buffer.
 * Returns:     R_OK   - Cannot fail ... see comment above.
 ******************************************************************************/
int    PutCharFromInt( UCHAR   *pDestBuf,    /* O: Destination buffer */
                       UINT    lVar )        /* I: Variable of type int */
{
    /* Local variables.
    */
    int         nReturn = R_OK;
    ULNG        lVal = (ULNG)lVar;

    /* Cant assume any byte ordering, so use arithmetic to break it down
     * into fundamental components. Assume 16 bit Int.
    */
    if(lVal > 65536L)
    {
        /* > 16 bit, so remove upper component.
        */
        lVal -= (lVal/16777216L)*65536L;
    }
    pDestBuf[0] = (UCHAR)(lVal/256L);
    lVal       -= (ULNG)pDestBuf[0] * 256L;
    pDestBuf[1] = (UCHAR)lVal;

    /* Finished, get out!!
    */
    return( nReturn );
}

/******************************************************************************
 * Function:    GetLongFromChar
 * Description: Get a long type variable from a character buffer. The byte 
 *              ordering in the buffer is assumed to be 32bit, MSB(3), 2, 1,
 *              LSB (0), where the MSB fits into the first byte of the
 *              buffer.
 * Returns:     R_OK   - Cannot fail ... will I be eating my words...?
 ******************************************************************************/
ULNG GetLongFromChar( UCHAR    *pDestBuf )    /* I: Source buffer to convert */
{
    /* Local variables.
    */
    ULNG        lVal = 0;

    /* Cant assume any byte ordering, so use arithmetic to break it down
     * into fundamental components. Assume a long is 32bit.
    */
    lVal  = (UCHAR)pDestBuf[0] * 16777216L;
    lVal += (UCHAR)pDestBuf[1] * 65536L;
    lVal += (UCHAR)pDestBuf[2] * 256L;
    lVal += (UCHAR)pDestBuf[3];

    /* Finished, get out!!
    */
    return( lVal );
}

/******************************************************************************
 * Function:    GetIntFromChar
 * Description: Get a long type variable from a character buffer. The byte 
 *              ordering in the buffer is assumed to be 16bit, MSB(1), LSB(0)
 *              where the MSB fits into the first byte of the buffer.
 * Returns:     R_OK   - Cannot fail ... will I be eating my words...?
 ******************************************************************************/
UINT GetIntFromChar( UCHAR    *pDestBuf )    /* I: Source buffer to convert */
{
    /* Local variables.
    */
    UINT        lVal = 0;

    /* Cant assume any byte ordering, so use arithmetic to break it down
     * into fundamental components. Assume a long is 32bit.
    */
    lVal += (UCHAR)pDestBuf[0] * 256;
    lVal += (UCHAR)pDestBuf[1];

    /* Finished, get out!!
    */
    return( lVal );
}

/******************************************************************************
 * Function:    StrPut
 * Description: Put a string INTO another string. Same as strcpy BUT it doesnt
 *              terminate the destination string.
 * Returns:     R_OK   - Cannot fail ... will I be eating my words...?
 ******************************************************************************/
UINT StrPut( UCHAR    *spDestBuf, /* I: Destination buffer to copy into */
             UCHAR    *spSrcBuf,  /* I: Source buffer to copy from */
             UINT     nBytes )    /* I: Number of bytes to copy */
{
    /* Local variables.
    */
    UINT        nNdx;

    /* Simple copy.
    */
    for(nNdx=0; nNdx < nBytes && spSrcBuf[nNdx] != '\0'; nNdx++)
    {
        spDestBuf[nNdx] = spSrcBuf[nNdx];
    }

    /* Finished, get out!!
    */
    return( R_OK );
}

/******************************************************************************
 * Function:    FFwdOverWhiteSpace
 * Description: Forward a pointer past whitespace in the input buffer.
 * Returns:     Non.
 ******************************************************************************/
void FFwdOverWhiteSpace( UCHAR   *szInBuf,    /* I: Input data buffer */
                         UINT    *nPos )      /* IO: Start/End position in buf */
{
    /* Local variables.
    */

    /* Move the pointer past whitespace to the fisrt non-whitespace character
     * or the end of file.
    */
    while(szInBuf[*nPos] != '\0' && isspace(szInBuf[*nPos]))
    { (*nPos)++; }

    return;
}

/******************************************************************************
 * Function:    ParseForToken
 * Description: Parse the input buffer for the next token. A token can be a
 *              Alpha/Alphanum word, a numeric, a single character or a 
 *              string.
 * Returns:     Type of Token located.
 ******************************************************************************/
UINT ParseForToken( UCHAR    *szInBuf,      /* I: Input buffer */
                    UINT     *nPos,         /* IO: Current pos in buffer */
                    UCHAR    *szTokBuf )    /* O: Token output buffer */
{
    /* Local variables.
    */
    UINT        nExit;
    UINT        nQuoteType;
    UINT        nToken;
    UINT        nTokNdx;

    /* Initially, forward to the first non-whitespace.
    */
    while(szInBuf[*nPos] != '\0' && isspace(szInBuf[*nPos]))
    {
        (*nPos)++;
    }

    /* End of buffer..?
    */
    if(szInBuf[*nPos] == '\0')
    {
        /* Instruct caller that we are at the end.
        */
        nToken = TOK_EOB;
    } else
    /* Do we have an alpha/alphanum in the buffer?
    */
    if(isalpha(szInBuf[*nPos]))
    {
        /* Instruct caller, initially, that we located an ALPHA word.
        */
        nToken = TOK_ALPHA;

        /* Go through the buffer, copying out the word.
        */
        for(nTokNdx=0; isalnum(szInBuf[*nPos]) || szInBuf[*nPos] == '-' ||
            szInBuf[*nPos] == '_' || szInBuf[*nPos] == '/';
            nTokNdx++, (*nPos)++)
        {
            /* Data copy.
            */
            szTokBuf[nTokNdx] = szInBuf[*nPos];
        }

        /* Terminate Token Buffer.
        */
        szTokBuf[nTokNdx] = '\0';
    } else
    /* Do we have a numeric in the buffer?
    */
    if(isdigit(szInBuf[*nPos]) ||
       (szInBuf[*nPos] == '-' && isdigit(szInBuf[(*nPos)+1])) ||
       (szInBuf[*nPos] == '+' && isdigit(szInBuf[(*nPos)+1])))
    {
        /* Instruct caller that we located a numeric.
        */
        nToken = TOK_NUMERIC;

        /* Straight data copy.
        */
        for(nTokNdx=0;
            isdigit(szInBuf[*nPos]) || szInBuf[*nPos] == '.' ||
            szInBuf[*nPos] == '-' || szInBuf[*nPos] == '+';
            nTokNdx++, (*nPos)++)
        {
            szTokBuf[nTokNdx] = szInBuf[*nPos];
        }

        /* Terminate Token Buffer.
        */
        szTokBuf[nTokNdx] = '\0';
    } else
    /* Do we have a string in the buffer?
    */
    if(szInBuf[*nPos] == 0x27 || szInBuf[*nPos] == 0x22)
    {
        /* Instruct caller that we located a string.
        */
        nToken = TOK_STRING;

        /* What type of quote are we looking for?
        */
        if(szInBuf[(*nPos)] == 0x27)
            nQuoteType = 0;
        else
            nQuoteType = 1;

        /* Go through the input buffer until we locate the matching quote
         * or we hit the end of buffer.
        */
        for(nTokNdx=0, nExit=FALSE; nExit == FALSE; nTokNdx++, (*nPos)++)
        {
            /* Copy over to the token buffer, all chars, including the final
             * quote/eob.
            */
            szTokBuf[nTokNdx] = szInBuf[*nPos];

            /* If were looking for single quotes and we locate one which is
             * not escaped, then we've reached the end of the string.
            */
            if(szInBuf[*nPos] == 0x27 && nQuoteType == 0)
            {
                /* If where at the beginning of a string and we dont meet
                 * the criterion for a null string, the loop.
                */
                if(nTokNdx == 0 &&
                   szInBuf[(*nPos)+1] != 0x27)
                {
                    continue;
                } else

                /* A null string?
                */
                if(nTokNdx == 0 &&
                   szInBuf[(*nPos)+1] == 0x27 &&
                   szInBuf[(*nPos)+2] != 0x27)
                {
                    /* Copy final byte into token buffer.
                    */
                    szTokBuf[++nTokNdx] = szInBuf[++(*nPos)];
                    nExit = TRUE;
                } else

                /* A lone quote?
                */
                if(szInBuf[(*nPos)-1] != 0x5c &&
                   szInBuf[(*nPos)+1] != 0x27 &&
                   szInBuf[(*nPos)-1] != 0x27)
                {
                    nExit = TRUE;
                }
            } else

            /* If were looking for double quotes and we locate on which is
             * not escaped, then we've reached the end of the string.
            */
            if(szInBuf[*nPos] == 0x22 && nQuoteType == 1)
            {
                /* If where at the beginning of a string and we dont meet
                 * the criterion for a null string, then loop.
                */
                if(nTokNdx == 0 &&
                   szInBuf[(*nPos)+1] != 0x22)
                {
                    continue;
                } else

                /* A null string?
                */
                if(szInBuf[(*nPos)+1] == 0x22 &&
                   szInBuf[(*nPos)+2] != 0x22 &&
                   nTokNdx == 0)
                {
                    /* Copy final byte into token buffer.
                    */
                    szTokBuf[++nTokNdx] = szInBuf[++(*nPos)];
                    nExit = TRUE;
                } else

                /* A lone quote?
                */
                if(szInBuf[(*nPos)-1] != 0x5c &&
                   szInBuf[(*nPos)+1] != 0x22 &&
                   szInBuf[(*nPos)-1] != 0x22)
                {
                    nExit = TRUE;
                }
            } else

            /* Found the end of buffer... some idiot hasnt terminated the
             * string correctly.
            */
            if(szInBuf[*nPos] == '\0')
                nExit = TRUE;
        }

        /* Terminate Token Buffer.
        */
        szTokBuf[nTokNdx] = '\0';
    } else
    /* Is this a comment...?
    */
    if(szInBuf[*nPos] == '/' && szInBuf[(*nPos)+1] == '*')
    {
        /* Instruct caller that we located a comment.
        */
        nToken = TOK_COMMENT;

        /* Loop through copying upto the end of the comment.
        */
        for(nTokNdx=0, nExit=FALSE; nExit == FALSE; nTokNdx++, (*nPos)++)
        {
            /* Copy data into token buffer.
            */
            szTokBuf[nTokNdx] = szInBuf[*nPos];
            if(nTokNdx < 2) continue;

            /* End of comment?
            */
            if(szInBuf[*nPos] == '*' && szInBuf[(*nPos)+1] == '/')
            {
                nExit = TRUE;
            }

            /* Found the end of buffer... some idiot hasnt terminated the
             * comment correctly.
            */
            if(szInBuf[*nPos] == '\0')
                nExit = TRUE;
        }

        /* Terminate Token Buffer.
        */
        szTokBuf[nTokNdx] = '\0';
    } else
     {
        /* Instruct caller that we located a character.
        */
        nToken = TOK_CHAR;

        /* Straight data copy and termination.
        */
        szTokBuf[0] = szInBuf[(*nPos)++];
        szTokBuf[1] = '\0';
    }

    /* Return type to caller.
    */
    return(nToken);
}

/******************************************************************************
 * Function:    ParseForString
 * Description: Get next valid string from input buffer.
 * Returns:     Non.
 ******************************************************************************/
int ParseForString( UCHAR    *szInBuf,      /* I: Input buffer */
                    UINT     *nPos,         /* I: Position in input buffer */
                    UCHAR    *szOutBuf )    /* O: Target buffer for string */
{
    /* Local variables.
    */
    UINT        nReturn = R_OK;
    UINT        nTokType;

    /* Zero callers buffer.
    */
    szOutBuf[0] = '\0';

    /* Get next parameter.
    */
    nTokType = ParseForToken(szInBuf, nPos, szOutBuf);

    /* If its not a valid string, modify return code.
    */
    if(nTokType == TOK_COMMENT || nTokType == TOK_CHAR ||
       nTokType == TOK_EOB || strlen(szOutBuf) == 0)
    {
        nReturn = R_FAIL;
    }

    /* Return result to caller.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    ParseForInteger
 * Description: Get next valid integer from input buffer.
 * Returns:     Non.
 ******************************************************************************/
int ParseForInteger( UCHAR   *szInBuf,     /* I: Input buffer */
                     UINT    *nPos,        /* I: Position in input buffer */
                     UINT    *nMin,        /* I: Minimum allowable value */
                     UINT    *nMax,        /* I: Maximum allowable value */
                     int     *pOutInt )    /* O: Target buffer for integer */
{
    /* Local variables.
    */
    UINT        nInt;
    UINT        nReturn = R_OK;
    UINT        nTokType;
    UCHAR       szTmpBuf[MAX_TMPBUFLEN];

    /* Get next parameter.
    */
    nTokType = ParseForToken(szInBuf, nPos, szTmpBuf);

    /* Convert into an integer.
    */
    if(nTokType != TOK_NUMERIC || sscanf(szTmpBuf, "%d", &nInt) != 1 ||
       (nMin != NULL && nInt < *nMin) || (nMax != NULL && nInt > *nMax))
    {
        nReturn = R_FAIL;
    } else
     {
        *pOutInt = nInt;
    }

    /* Return result to caller.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    ParseForLong
 * Description: Get next valid Long from input buffer.
 * Returns:     Non.
 ******************************************************************************/
int ParseForLong( UCHAR   *szInBuf,     /* I: Input buffer */
                  UINT    *nPos,        /* I: Position in input buffer */
                  long    *lMin,        /* I: Minimum allowable value */
                  long    *lMax,        /* I: Maximum allowable value */
                  long    *pOutLong )   /* O: Target buffer for Long */
{
    /* Local variables.
    */
    UINT        nReturn = R_OK;
    UINT        nTokType;
    long        lLng;
    UCHAR       szTmpBuf[MAX_TMPBUFLEN];

    /* Get next parameter.
    */
    nTokType = ParseForToken(szInBuf, nPos, szTmpBuf);

    /* Convert into an integer.
    */
    if(nTokType != TOK_NUMERIC || sscanf(szTmpBuf, "%ld", &lLng) != 1 ||
       (lMin != NULL && lLng < *lMin) || (lMax != NULL && lLng > *lMax))
    {
        nReturn = R_FAIL;
    } else
     {
        *pOutLong = lLng;
    }

    /* Return result to caller.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    StrRTrim
 * Description: A function to trim off all trailing spaces for a given string.
 *              The function works by starting at the end of a null terminated
 *              string and looking for the first non-space character. It then
 *              places a null terminator at the new location.
 * Returns:     Pointer to new string.
 ******************************************************************************/
char *StrRTrim(    char    *szSrc )    /* IO: Base string to trim */
{
    /* Local variables.
    */
    char        *pLocation = &szSrc[strlen(szSrc)-1];

    /* Scan backwards until we find a non space or we meet the beginning
     * of the string.
    */
    while( pLocation != szSrc && isspace(*pLocation) )
    {
        pLocation--;
    }

    /* If we are not at the beginning of the string, then place a null
     * terminator at t+1;
    */
    if(pLocation != szSrc)
    {
        pLocation++;
        *pLocation = '\0';
    }

    /* All done, exit.
    */
    return(szSrc);
}

/******************************************************************************
 * Function:    StrCaseCmp
 * Description: A function to perform string compares regardless of 
 *              character case. Provided mainly for operating systems that
 *              dont possess such functionality.
 * Returns:     0 - Strings compare.
 *              > 0 
 *              < 0
 ******************************************************************************/
int    StrCaseCmp( const char    *szSrc,       /* I: Base string to compare against */
                   const char    *szCmp )      /* I: Comparator string */
{
    /* Local variables.
    */
    register signed char    cResult;

    /* Loop, progressing through the strings until we have a difference or
     * we get to the end of the string and hence have a comparison match.
    */
    while(1)
    {
        cResult = toupper(*szSrc) - toupper(*szCmp);
        szCmp++;
        szSrc++;
        if(cResult != 0 || *szCmp == '\0' || *szSrc == '\0')
            break;
    }

    /* Return result to caller.
    */
    return(cResult);
}

/******************************************************************************
 * Function:    StrnCaseCmp
 * Description: A function to perform string compares regardless of 
 *              character case for a specified number of characters within
 *              both strings. Provided mainly for operating systems that
 *              dont possess such functionality.
 * Returns:     0 - Strings compare.
 *              > 0 
 *              < 0
 ******************************************************************************/
int    StrnCaseCmp( const char    *szSrc,       /* I: Base string to compare against */
                    const char    *szCmp,       /* I: Comparator string */
                    size_t        nCount )      /* I: Number of bytes to compare */
{
    /* Local variables.
    */
    register signed char    cResult;

    /* Loop, progressing through the strings until we have a difference or
     * the given number of characters match in both strings.
    */
    while(nCount)
    {
        cResult = toupper(*szSrc) - toupper(*szCmp);
        szCmp++;
        szSrc++;
        if(cResult != 0 || *szCmp == '\0' || *szSrc == '\0')
            break;
        nCount--;
    }

    /* Return result to caller.
    */
    return(cResult);
}

/******************************************************************************
 * Function:    SplitFQFN
 * Description: A function to split a fully qualified filename into a
 *              directory and filename components.
 * Returns:     R_OK   - Filename split.
 *              R_FAIL - Couldnt split due to errors, ie. memory.
 ******************************************************************************/
int    SplitFQFN( char        *szFQFN,    /* I: Fully Qualified File Name */
                  char        **szDir,    /* O: Directory component */
                  char        **szFN )    /* O: Filename component */
{
    /* Local variables.
    */
    UINT        nNdx;
    char        *spTmpDir;
    char        *spTmpFN;
    char        *szFunc = "SplitFQFN";

    /* Allocate a buffer to hold the directory component.
    */
    if((spTmpDir=(UCHAR *)malloc(strlen(szFQFN)+1)) == NULL)
    {
        Lgr(LOG_DEBUG, szFunc, "Couldnt malloc (%d) bytes", strlen(szFQFN)+1);
        Errno = E_NOMEM;
        return(R_FAIL);
    }

    /* Allocate a buffer to hold the filename component.
    */
    if((spTmpFN=(UCHAR *)malloc(strlen(szFQFN)+1)) == NULL)
    {
        Lgr(LOG_DEBUG, szFunc, "Couldnt malloc (%d) bytes", strlen(szFQFN)+1);
        Errno = E_NOMEM;
        return(R_FAIL);
    }

    /* Starting at the end of the string, work backwards until we find
     * a directory element character or start of string.
    */
    for(nNdx=strlen(szFQFN); nNdx >= 0 && szFQFN[nNdx] != '/'; nNdx--);

    /* If nNdx == 0 && szFQFN[nNdx] not equal to the directory character
     * then we have a filename with no directory. Just copy.
    */
    if(nNdx == 0 && szFQFN[nNdx] != '/')
    {
        strcpy(spTmpDir, "");
        strcpy(spTmpFN, szFQFN);
    } else
    if(nNdx == 0 && szFQFN[nNdx] == '/')
    {
        strcpy(spTmpDir, "/");
        strcpy(spTmpFN, &szFQFN[1]);
    } else
    {
        szFQFN[nNdx] = '\0';
        strcpy(spTmpDir, szFQFN);
        strcpy(spTmpFN, &szFQFN[nNdx+1]);
        szFQFN[nNdx] = '/';
    }

    /* Setup callers pointers to point to new memory.
    */
    *szDir = spTmpDir;
    *szFN = spTmpFN;

    /* Return result to caller.
    */
    return(R_OK);
}
