/******************************************************************************
 * Product:       #     # #     #         #         ###   ######
 *                #     #  #   #          #          #    #     #
 *                #     #   # #           #          #    #     #
 *                #     #    #            #          #    ######
 *                #     #   # #           #          #    #     #
 *                #     #  #   #          #          #    #     #
 *                 #####  #     # ####### #######   ###   ######
 *
 * File:          ux_mon.h
 * Description:   Application internal monitor processing system. Allows
 *                possibility of real-time access to a running application.
 *
 * Version:       %I%    
 * Dated:         %D%
 * Copyright:     P.D. Smart, 1994-2019
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
#ifndef    UX_MONITOR_H
#define    UX_MONITOR_H

/* Interactive monitor service types.
*/
#define    MON_SERVICE_HTML    0x00    /* All comms are based on HTML */
#define    MON_SERVICE_NL      0x01    /* All comms are based on Natural Language. */
#define    MON_SERVICE_EXT     0xff    /* External service handled by an application interpreter override */

/* Interactive monitor commands.
*/
#define    MON_MSG_NEW         0x1F    /* New IM process connecting */
#define    MON_MSG_TERMINATE   0x1E    /* Terminating IM process */
#define    MON_MSG_REDISPLAY   0x1D    /* Redisplay prompt */
#define    MON_MSG_TEST        0x1C    /* Test connectivity */
#define    MON_MSG_ERRMSG      0x1B    /* Inbound error message */
#define    MON_MSG_ALERT       0x1A    /* Red-Alert condition */
#define    MON_MSG_CANALERT    0x19    /* Cancel Red-Alert condition */

/* Basic Interactive monitoring command set.
*/
#define    MC_TERMINATE        "TERMINATE"
#define    MC_HTMLGET          "GET"
#define    MC_HTMLHEAD         "HEAD"
#define    MC_HTMLPOST         "POST"
#define    MC_HTMLPUT          "PUT"
#define    MC_HTMLDELETE       "DELETE"
#define    MC_HTMLCONNECT      "CONNECT"
#define    MC_HTMLOPTIONS      "OPTIONS"
#define    MC_HTMLTRACE        "TRACE"

/* A structure containing data relevant to each unique connection made
 * by a client with the monitor system.
*/
typedef struct {
    UINT        nChanId;               /* Channel ID descr for Socket Lib */
    UINT        nClientPortNo;         /* TCP port client connected on */
    ULNG        lClientIPaddr;         /* IP address of connecting client */
} ML_CONLIST;

/* A structure defining the list of commands that the interactive monitor
 * port recognises and the associated callbacks for each command.
*/
typedef struct {
    UCHAR       *szCommand;            /* Command in ascii text format */
    int         (*nCallback)();        /* Function to call when text recognised */
} ML_COMMANDLIST;

/* A structure defining an individual service offered by this application.
*/
typedef struct {
    UINT        nMonPort;              /* TCP port service provided on */
    UINT        nServiceType;          /* Type of service provided */
    int         (*nConnectCB)();       /* CB on Client Connection */
    int         (*nDisconCB)();        /* CB on Client Disconnection */
    int         (*nInterpretOvr)();    /* Override for Interpretation function */
    UCHAR       *szServerName;         /* Name of server for HTTP responses */
    LINKLIST    *spConHead;            /* Head of Connection List */
    LINKLIST    *spConTail;            /* Tail of Connection List */
    LINKLIST    *spMCHead;             /* Head of LList containing I/Mon commands */
    LINKLIST    *spMCTail;             /* Tail of LList containing I/Mon commands */
} ML_MONLIST;

/* Global variables for the Comms module.
*/
typedef struct {
    LINKLIST    *spMonHead;            /* Head of LList containing I/Mon commands */
    LINKLIST    *spMonTail;            /* Tail of LList containing I/Mon commands */
} ML_GLOBALS;

/* Prototypes for functions internal to MonitorLib module.
*/
int        _ML_MonTerminate( UINT, UCHAR *, UINT );
void       _ML_MonitorCB( UINT, UCHAR *, UINT );
void       _ML_MonitorCntrl( int, ... );

/* Prototypes to externally visible and usable functions.
*/
int        ML_Init( UINT, UINT, UCHAR *, int (*)(), int    (*)(), int (*)());
int        ML_Exit( UCHAR    * );
int        ML_Send( UINT, UCHAR *, UINT );
int        ML_AddMonCommand( UINT, UCHAR *, int (*)() );
int        ML_DelMonCommand( UINT, UCHAR * );
int        ML_Broadcast( UCHAR    *, UINT );

#endif    /* UX_MONITOR_H */
