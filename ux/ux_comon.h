/******************************************************************************
 * Product:       #     # #     #         #         ###   ######
 *                #     #  #   #          #          #    #     #
 *                #     #   # #           #          #    #     #
 *                #     #    #            #          #    ######
 *                #     #   # #           #          #    #     #
 *                #     #  #   #          #          #    #     #
 *                 #####  #     # ####### #######   ###   ######
 *
 * File:          ux_comon.h
 * Description:   General purpose library routines.
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

/* Ensure file is only included once - avoid compile loops.
*/
#ifndef    UX_COMMON_H
#define    UX_COMMON_H

/* Definitions for Linked List sorting. 
*/
#define    SORT_NONE        0        /* No linklist sorting */
#define    SORT_INT_UP      1        /* Linklist sorted incrementally on int */
#define    SORT_INT_DOWN    2        /* Linklist sorted decrementally on int */
#define    SORT_LONG_UP     3        /* Linklist sorted incrementally on long */
#define    SORT_LONG_DOWN   4        /* Linklist sorted decrementally on long */
#define    SORT_CHAR_UP     5        /* Linklist sorted on alpha string */
#define    SORT_CHAR_DOWN   6        /* Linklist sorted in reverseo on alpha string*/

/* Logger definitions. Define's mode and level logger operates at.
*/
#define    LOG_OFF          0        /* LEVEL: Logging off */
#define    LOG_CONFIG       1        /* LEVEL: Logger being configured */
#define    LOG_DEBUG        2        /* LEVEL: All debug messages and above */
#define    LOG_WARNING      3        /* LEVEL: All warning messages and above */
#define    LOG_MESSAGE      4        /* LEVEL: All information messages and above */
#define    LOG_ALERT        5        /* LEVEL: All alert messages and above */
#define    LOG_FATAL        6        /* LEVEL: All fatal messages */
#define    LOG_DIRECT       7        /* LEVEL: Always log to stdout if logging on */
#define    LGM_OFF          0        /* MODE: Logger switched off */
#define    LGM_STDOUT       1        /* MODE: Logger logging to stdout */
#define    LGM_FLATFILE     2        /* MODE: Logger logging to flat file */
#define    LGM_DB           3        /* MODE: Logger logging to database */
#define    LGM_ALL          4        /* MODE: Logger logging to all destinations */

/* Parser token flags. Identifies the type of token encountered in a parsing
 * input stream.
*/
#define    TOK_EOB          0        /* End of Buffer */
#define    TOK_ALPHA        1        /* Token is an alphabetic word */
#define    TOK_ALPHANUM     2        /* Token is an alphanumeric word */
#define    TOK_STRING       3        /* Token is a complete string */
#define    TOK_NUMERIC      4        /* Token is a numeric */
#define    TOK_COMMENT      5        /* Token is a comment */
#define    TOK_CHAR         6        /* Token is a character */

/* Define prototypes for functions globally available.
*/
int        AddItem(LINKLIST **, LINKLIST **, int, UINT *, ULNG *, UCHAR *, void *);
int        DelItem( LINKLIST **, LINKLIST **, void *, UINT *, ULNG *, UCHAR * );
void      *FindItem( LINKLIST *, UINT *, ULNG *, UCHAR * );
void      *StartItem( LINKLIST *, LINKLIST ** );
void      *NextItem( LINKLIST ** );
int        MergeLists( LINKLIST **, LINKLIST **, LINKLIST *, LINKLIST *, int );
int        DelList( LINKLIST **, LINKLIST ** );
int        SizeList( LINKLIST *, UINT * );
int        PutCharFromLong( UCHAR *, ULNG );
int        PutCharFromInt( UCHAR *, UINT );
ULNG       GetLongFromChar( UCHAR * );
UINT       GetIntFromChar( UCHAR * );
UINT       StrPut( UCHAR *, UCHAR *, UINT );
void       FFwdOverWhiteSpace( UCHAR *, UINT * );
UINT       ParseForToken( UCHAR *, UINT *, UCHAR * );
int        ParseForString( UCHAR *, UINT *, UCHAR * );
int        ParseForInteger( UCHAR *, UINT *, UINT *, UINT *, int * );
int        ParseForLong( UCHAR *, UINT *, long *, long *, long * );
UCHAR     *Compress( UCHAR *, UINT * );
UCHAR     *Decompress( UCHAR *, UINT * );
void       Lgr( int, ... );
int        GetCLIParam( int, UCHAR **, UCHAR *, UINT, UCHAR    *, UINT, UINT );
char      *StrRTrim( char * );
int        StrCaseCmp(    const char *, const char * );
int        StrnCaseCmp(const char *, const char *, size_t );
int        SplitFQFN(    char *, char **, char ** );

/* For windows, we must logically associate the case insensitive comparator
 * functions with those in this library rather than those in the operating
 * system as at the current moment (6/96) Windows does not provide them.
*/
#if defined(_WIN32)
#define    strcasecmp    StrCaseCmp
#define    strncasecmp   StrnCaseCmp
#endif

#endif    /* UX_COMMON_H */
