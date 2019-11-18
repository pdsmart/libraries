/******************************************************************************
 * Product:       #     # ######   #####          #         ###   ######
 *                ##   ## #     # #     #         #          #    #     #
 *                # # # # #     # #               #          #    #     #
 *                #  #  # #     # #               #          #    ######
 *                #     # #     # #               #          #    #     #
 *                #     # #     # #     #         #          #    #     #
 *                #     # ######   #####  ####### #######   ###   ######
 *
 * File:          mdc.h
 * Description:   Header file for declaration of structures, datatypes etc.
 * Version:       %I%
 * Dated:         %D%
 * Copyright:     P.D. Smart, 1996-2019.
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
#ifndef    MDC_H
#define    MDC_H

/* Bring in declaration of all UX functions.
*/
#include    <ux.h>

/* Definitions for maxims etc.
*/
#define    MAX_DBNAMELEN        20        /* Database Name length */
#define    MAX_ERRMSGLEN        1024      /* Length of an error msg */
#define    MAX_FTPOPTIONLEN     20        /* Length of an option string for FTP */
#define    MAX_PASSWORDLEN      20        /* Password length */
#define    MAX_SERVERNAMELEN    20        /* Server Name length */
#define    MAX_USERNAMELEN      20        /* User Name length */

/* Definitions for Function Returns
*/
#define    MDC_OK               0         /* Return OK */
#define    MDC_FAIL             -1        /* Return FAIL */
#define    MDC_NODAEMON         -2        /* No daemon at destination */
#define    MDC_SERVICENAK       -3        /* Daemon cannot provide the req service */
                                          /* or bad parameters provided    */
#define    MDC_BADCONTEXT       -4        /* Attempting to perform action in the */
                                          /* wrong context */
#define    MDC_SNDREQNAK        -5        /* NAK received for a Send Request */

/* Define MDC packet type definitions.
*/
#define    MDC_ACK              'A'       /* Positive comms reply */
#define    MDC_ABORT            'B'       /* Abort command packet */
#define    MDC_CHANGE           'C'       /* Change service command */
#define    MDC_DATA             'D'       /* Data packet */
#define    MDC_EXIT             'E'       /* Exit command packet */
#define    MDC_INIT             'I'       /* Service Initialisation request */
#define    MDC_NAK              'N'       /* Negative comms reply */
#define    MDC_PREQ             'P'       /* Process request command */

/* Service types
*/
#define    SRV_FTPX             'F'       /* FTP driver */
#define    SRV_JAVA             'J'       /* JAVA code execution */
#define    SRV_ODBC             'O'       /* ODBC access */
#define    SRV_SCMD             'C'       /* System command execution driver */
#define    SRV_SYBASE           'S'       /* Sybase database access */
#define    SRV_AUPL             'A'       /* Audio Player driver */

/* Define error return codes which are embedded into returned error messages
 * for the user to decipher.
*/
#define    MDC_EMSG_MEMORY      "M0000"

/* Service details structures for Sybase & ODBC.
*/
typedef struct SybaseServInfo {
    UCHAR    szUser[MAX_USERNAMELEN];     /* User Name */
    UCHAR    szPassword[MAX_PASSWORDLEN]; /* Password */
    UCHAR    szServer[MAX_SERVERNAMELEN]; /* Server name */
    UCHAR    szDatabase[MAX_DBNAMELEN];   /* Database name */
} SERV_INFO;

/* Service detail structures for FTP.
*/
typedef struct FTPServInfo {
    UCHAR    szServer[MAX_SERVERNAMELEN]; /* Name of FTP server */
    UCHAR    szUser[MAX_USERNAMELEN];     /* User on FTP server */
    UCHAR    szPassword[MAX_PASSWORDLEN]; /* Pwd on FTP server */
} FTPX_INFO;

typedef struct ServiceDetails {
    UCHAR    cServiceType;            /* Type of service identifier */

    /* Union of the different types of service structures for the different
     * types of drivers.
    */
    union {
        /* structure for each service type
        */
          SERV_INFO    sSybaseInfo;
          SERV_INFO    sODBCInfo;
        FTPX_INFO    sFTPInfo;
    } uServiceInfo;
} SERVICEDETAILS;

/* Prototypes externally visible for MDC Client API.
*/
int    MDC_Start(void);
int    MDC_End(void);
int    MDC_SetTimeout(UCHAR *, UINT, UCHAR *);
int    MDC_CreateService(UCHAR *, UINT *, SERVICEDETAILS *);
int    MDC_ChangeService(UINT, SERVICEDETAILS *);
int    MDC_CloseService(UINT);
int    MDC_SendRequest(UINT, UCHAR *, UINT, void (*) (UINT, UCHAR *, UINT));
int    MDC_GetResult(UINT, UCHAR **);
int    MDC_GetStatus(UINT, UINT *);

/* Prototypes externally visible for MDC Server API.
*/
int    MDC_Server( UINT *, UCHAR *, int (*)(UCHAR *, int, UCHAR *), void (*)( UCHAR ) );
int    MDC_ReturnData( UCHAR *, int );
int    MDC_TimerCB( ULNG, UINT, UINT, void (*)(void) );

#endif    /* MDC_H */
