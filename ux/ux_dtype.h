/******************************************************************************
 * Product:       #     # #     #         #         ###   ######
 *                #     #  #   #          #          #    #     #
 *                #     #   # #           #          #    #     #
 *                #     #    #            #          #    ######
 *                #     #   # #           #          #    #     #
 *                #     #  #   #          #          #    #     #
 *                 #####  #     # ####### #######   ###   ######
 *
 * File:          ux_dtype.h
 * Description:   Header file for declaration of structures, datatypes etc.
 * Version:       %I%
 * Dated:         %D%
 * Copyright:     P.D. Smart 1994-2019.
 *
 * History:       1.0 - Initial Release.
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
#ifndef    UX_DATATYPE_H
#define    UX_DATATYPE_H

/* Datatype declarations. Basically need to use unsigned variables in most
 * instances, so rather than typing out the full word, we define some
 * short names.
*/
typedef    unsigned int     UINT;
typedef    unsigned long    ULNG;
typedef    unsigned char    UCHAR;
typedef    unsigned short   USHRT;

/* Standard declarations. Sometimes they may not be defined.
*/
#ifndef    TRUE
#define    TRUE                1
#endif
#ifndef    FALSE
#define    FALSE               0
#endif

/* Define Maxim's, minimums etc.
*/
#define    MAX_DATETIME        27         /* Max len of a date/time buffer */
#define    MAX_DBNAME          32         /* Max len of database name */
#define    MAX_DBROWSIZE       255        /* Max size of a database row */
#define    MAX_DESTMAPENTRIES  255        /* Max number of dest map lookup entries */
#define    MAX_ERRMSG          2048       /* Max size of an error message */
#define    MAX_FILEDESCR       256        /* Max number of Socket/File descriptors */
#define    MAX_FILENAME        80         /* Max size of a filename */
#define    MAX_HOSTNAME        11         /* Max size of a machine host name */
#define    MAX_INGRESSTMTBUF   131072     /* Max size for Ingres Statement Buffer */
#define    MAX_IPADDR          16         /* Max size of an ascii IP address */
#define    MAX_MACHINENAME     40         /* Max size of a machines name */
#define    MAX_PACKETS         256        /* Max number of packets in the inqueue */
#define    MAX_PATHLEN         128        /* Max len of a pathname */
#define    MAX_PROGNAME        80         /* Max size of a program name */
#define    MAX_QDATA           1530       /* Max size of Q data */
#define    MAX_RECVBUFSIZE     1048576    /* Max size a rcv buf can grow to */
#define    MAX_RECVLEN         2048       /* Max size of a single receive */
#define    MAX_SENDBUFFER      2048       /* Max size of a transmit buffer */
#define    MAX_SERVERNAME      32         /* Max size of a server name */
#define    MAX_SERVICENAME     50         /* Max size of a service name */
#define    MAX_SOCKETBACKLOG   5          /* Max number of incoming socket backlog */
#define    MAX_SYB_INQUPD      15         /* Max simultaneous number of InQ updates */
#define    MAX_SYB_OUTQUPD     15         /* Max simultaneous number of OutQ Upd */
#define    MAX_TABLENAME       32         /* Max size of a table name */
#define    MAX_TMPBUFLEN       2048       /* Max size of a temporary buffer */
#define    MAX_USERNAME        32         /* Max size of a user name */
#define    MAX_USERPWD         32         /* Max size of a user password */
#define    MAX_VARARGBUF       8192       /* Max size of a vararg sprintf buffer */
#define    MAX_XLATEBUF        10240      /* Max size of an sql xlate buffer */
#define    MIN_COMPRESSLEN     256        /* Min size of data prior to compression */

/* Definitions for function return parameters.
*/
#undef     R_OK                           /* Linux declares R_OK for Read permissions, so undefine as name clash and not needed. */
#define    R_OK                0
#define    R_FAIL              1
#define    R_EXIT              2

/* Definitions for describing a target variable to a called function. Used
 * primarily in switches.
*/
#define    T_INT               3000       /* Type is for integer */
#define    T_STR               3001       /* Type is for string */
#define    T_CHAR              3002       /* Type is for character */
#define    T_LONG              3003       /* Type is for long */

/* Error return codes from functions within the Replication System. Generally
 * returned by a function in the global external 'errno'.
*/
#define    E_BADHEAD           1          /* Bad Head pointer in Link List */
#define    E_BADTAIL           2          /* Bad Tail pointer in Link List */
#define    E_BADPARM           3          /* Bad parameters passed to a function */
#define    E_NOKEY             4          /* No Link List search key */
#define    E_MEMFREE           5          /* Couldnt free allocated memory */
#define    E_NOMEM             6          /* Couldnt allocate memory */
#define    E_EXISTS            7          /* A Link List entry already exists */
#define    E_NOBIND            8          /* Couldnt perform a socket bind */
#define    E_NOLISTEN          9          /* Couldnt perform a socket listen */
#define    E_NOSOCKET          10         /* Couldnt allocate a socket */
#define    E_BADACCEPT         11         /* Couldnt perform an accept on a socket */
#define    E_INVCHANID         12         /* Invalid channel Id passed to comms lib */
#define    E_NOSERVICE         13         /* No service provider for a client */
#define    E_BADCONNECT        14         /* Couldnt issue a successful connect */
#define    E_NOCONNECT         15         /* Couldnt perform a socket connect */
#define    E_BUSY              16         /* Comms channel is busy, retry later */
#define    E_BADSOCKET         17         /* Given socket is bad or not connected */
#define    E_BADSELECT         18         /* Bad parameters causing select to fail */
#define    E_NODBSERVER        19         /* No database server available */
#define    E_NODATA            20         /* No data available */
#define    E_DBNOTINIT         21         /* Database not initialised */

/* Own internal link list handling. Simple progressive link list, with the
 * header containing the key elements. In this case, one of each type is
 * given, thereby allowing searches without knowing the structure of
 * the underlying code.
*/
typedef struct linklist {
    UINT            nKey;
    ULNG            lKey;
    UCHAR           *szKey;
    void            *spData;
    struct linklist *spNext;
} LINKLIST;

/* Need a reference to the external errorno which the UX library procedures make
 * use of.
extern int        errno;
*/
#if defined(UX_COMMS_C)
    int            Errno;
#else
    extern int     Errno;
#endif

#endif    /* UX_DATATYPE_H */
